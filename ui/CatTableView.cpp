/* 
 * File:   PlayerTableView.cpp
 * Author: volker
 * 
 * Created on March 17, 2014, 8:19 PM
 */

#include "CatTableView.h"
#include "MainFrame.h"
#include "Category.h"
#include <stdexcept>
#include <QInputDialog>
#include <QString>
#include <QtWidgets/qmessagebox.h>
#include "ui/delegates/CatItemDelegate.h"
#include "ui/DlgSeedingEditor.h"
#include "ui/dlgGroupAssignment.h"
#include "ui/commonCommands/cmdBulkAddPlayerToCat.h"

#include "CatMngr.h"

CategoryTableView::CategoryTableView(QWidget* parent)
:QTableView(parent)
{
  // an empty model for clearing the table when
  // no tournament is open
  emptyModel = new QStringListModel();
  
  connect(MainFrame::getMainFramePointer(), &MainFrame::tournamentOpened, this, &CategoryTableView::onTournamentOpened);

  // handle context menu requests
  setContextMenuPolicy(Qt::CustomContextMenu);
  connect(this, SIGNAL(customContextMenuRequested(const QPoint&)),
          this, SLOT(onContextMenuRequested(const QPoint&)));

  // define a delegate for drawing the category items
  auto itemDelegate = new CatItemDelegate(this);
  setItemDelegate(itemDelegate);

  // prep the context menu
  initContextMenu();
}

//----------------------------------------------------------------------------
    
CategoryTableView::~CategoryTableView()
{
  delete emptyModel;
}

//----------------------------------------------------------------------------
    
void CategoryTableView::onTournamentOpened(Tournament* _tnmt)
{
  tnmt = _tnmt;
  setModel(Tournament::getCategoryTableModel());
  setEnabled(true);
  
  // connect signals from the Tournament and TeamMngr with my slots
  connect(tnmt, &Tournament::tournamentClosed, this, &CategoryTableView::onTournamentClosed);
  
  // tell the rest of the category widget, that the model has changed
  // and that it should update
  emit catModelChanged();
}

//----------------------------------------------------------------------------
    
void CategoryTableView::onTournamentClosed()
{
  // disconnect from all signals, because
  // the sending objects don't exist anymore
  disconnect(tnmt, &Tournament::tournamentClosed, this, &CategoryTableView::onTournamentClosed);
  
  // invalidate the tournament handle and deactivate the view
  tnmt = 0;
  setModel(emptyModel);
  setEnabled(false);
  
  
  // tell the rest of the category widget, that the model has changed
  // and that it should update
  emit catModelChanged();
}

//----------------------------------------------------------------------------

bool CategoryTableView::isEmptyModel()
{
  if (model())
  {
    return (model()->rowCount() == 0);
  }
  return true;
}

//----------------------------------------------------------------------------

bool CategoryTableView::hasCategorySelected()
{
  if (isEmptyModel())
  {
    //throw std::invalid_argument("No model/tournament loaded");
    return false;
  }
  
  QModelIndexList indexes = selectionModel()->selection().indexes();
  if (indexes.count() == 0)
  {
    //throw std::invalid_argument("No category selected");
    return false;
  }

  return true;  
}

//----------------------------------------------------------------------------

Category CategoryTableView::getSelectedCategory()
{
  if (!(hasCategorySelected()))
  {
    throw std::invalid_argument("No category selected");
  }
  
  QModelIndexList indexes = selectionModel()->selection().indexes();
  int selectedSeqNum = indexes.at(0).row();
  
  return Tournament::getCatMngr()->getCategoryBySeqNum(selectedSeqNum);
}

//----------------------------------------------------------------------------

void CategoryTableView::onCategoryDoubleClicked(const QModelIndex& index)
{
  if (!(index.isValid()))
  {
    return;
  }
  
  Category selectedCat = Tournament::getCatMngr()->getCategoryBySeqNum(index.row());
  
  QString oldName = selectedCat.getName();
  
  bool isOk = false;
  while (!isOk)
  {
    QString newName = QInputDialog::getText(this, tr("Rename category"), tr("Enter new category name:"),
	    QLineEdit::Normal, oldName, &isOk);

    if (!isOk)
    {
      return;  // the user hit cancel
    }

    if (newName.isEmpty())
    {
      QMessageBox::critical(this, tr("Rename category"), tr("The new name may not be empty!"));
      isOk = false;
      continue;
    }

    if (oldName == newName)
    {
      return;
    }

    // okay, we have a valid name. try to rename the category
    newName = newName.trimmed();
    ERR e = Tournament::getCatMngr()->renameCategory(selectedCat, newName);

    if (e == INVALID_NAME)
    {
      QMessageBox::critical(this, tr("Rename category"), tr("The name you entered is invalid (e.g., too long)"));
      isOk = false;
      continue;
    }

    if (e == NAME_EXISTS)
    {
      QMessageBox::critical(this, tr("Rename category"), tr("A category of this name already exists"));
      isOk = false;
      continue;
    }
  }
}

//----------------------------------------------------------------------------

void CategoryTableView::onAddCategory()
{
  // try to create new categories using a
  // canonical name until it finally succeeds
  ERR e = NAME_EXISTS;
  int cnt = 0;
  while (e != OK)
  {
    QString newCatName = tr("New Category ") + QString::number(cnt);

    e = Tournament::getCatMngr()->createNewCategory(newCatName);
    ++cnt;
  }
}

//----------------------------------------------------------------------------

void CategoryTableView::onRemoveCategory()
{
  if (!(hasCategorySelected())) return;

  Category cat = getSelectedCategory();
  CatMngr* cm = Tournament::getCatMngr();

  // can the category be deleted at all?
  ERR err = cm->canDeleteCategory(cat);

  // category is already beyond config state
  if (err == CATEGORY_NOT_CONFIGURALE_ANYMORE)
  {
    QString msg = tr("The has already been started.\n");
    msg += tr("Running categories cannot be deleted anymore.");
    QMessageBox::critical(this, tr("Delete category"), msg);
    return;
  }

  // not all players removable
  if (err == PLAYER_NOT_REMOVABLE_FROM_CATEGORY)
  {
    QString msg = tr("Cannot remove all players from the category.\n");
    msg += tr("The category cannot be deleted.");
    QMessageBox::critical(this, tr("Delete category"), msg);
    return;
  }

  // okay, the category can be deleted. Get a confirmation
  QString msg = tr("Note: this will remove all player assignments from this category\n");
  msg += tr("and the category from the whole tournament.\n\n");
  msg += tr("This step is irrevocable!\n\n");
  msg += tr("Proceed?");
  int result = QMessageBox::question(this, tr("Delete category"), msg);
  if (result != QMessageBox::Yes) return;

  // we can actually delete the category. Let's go!
  err = cm->deleteCategory(cat);
  if (err != OK) {
    QString msg = tr("Something went wrong when deleting the category. This shouldn't happen.\n\n");
    msg += tr("For the records: error code = ") + QString::number(static_cast<int> (err));
    QMessageBox::warning(this, tr("WTF??"), msg);
  }
}

//----------------------------------------------------------------------------

void CategoryTableView::onRunCategory()
{
  if (!(hasCategorySelected())) return;

  unique_ptr<Category> selectedCat = getSelectedCategory().convertToSpecializedObject();

  // branch here to another function if the action was triggered to
  // continue a category that's waiting in state STAT_CAT_WAIT_FOR_INTERMEDIATE_SEEDING
  if (selectedCat->getState() == STAT_CAT_WAIT_FOR_INTERMEDIATE_SEEDING)
  {
    handleIntermediateSeedingForSelectedCat();
    return;
  }

  // pre-test category-specific conditions
  ERR e = selectedCat->canFreezeConfig();
  if (e == CONFIG_ALREADY_FROZEN)
  {
    QMessageBox::critical(this, tr("Run Category"),
      tr("This category has already been started (STAT != Config)"));
  }
  else if (e == UNPAIRED_PLAYERS)
  {
    QMessageBox::critical(this, tr("Run Category"),
      tr("This category has unpaired players!\nPlease pair all players before starting the matches."));
  }
  else if (e == INVALID_KO_CONFIG)
  {
    QMessageBox::critical(this, tr("Run Category"),
      tr("The setup for the round robin phase and the KO rounds are invalid!"));
  } else if (e != OK)
  {
    QMessageBox::critical(this, tr("Run Category"),
      tr("Uncaptured error. Category has no valid configuration and can't be started"));
  }

  if (e != OK) return;

  e = Tournament::getCatMngr()->freezeConfig(*selectedCat);
  // after we checked for category-specific errors above, we can only see general errors here
  if (e == NOT_ALL_PLAYERS_REGISTERED)
  {
    QMessageBox::critical(this, tr("Run Category"),
      tr("Some players in this category have not yet registered."));
  } else if (e != OK)
  {
    QMessageBox::critical(this, tr("Run Category"),
      tr("Uncaptured error. Category has no valid configuration and can't be started"));
  }

  if (e != OK) return;

  /**
   * Now the category is in status FROZEN.
   *
   * Execute all GUI activities necessary for actually running the category
   * but DO NOT MODIFY THE DATABASE until everything is confirmed by the user
   *
   */

  // show the dialog for the initial group assignments, if necessary
  QList<PlayerPairList> ppListList;
  if (selectedCat->needsGroupInitialization())
  {
    dlgGroupAssignment dlg(*selectedCat);
    dlg.setModal(true);
    int result = dlg.exec();

    if (result != QDialog::Accepted)
    {
      unfreezeAndCleanup(std::move(selectedCat));
      return;
    }

    ppListList = dlg.getGroupAssignments();
    if (ppListList.isEmpty())
    {
      QMessageBox::warning(this, tr("Run Category"), tr("Can't read group assignments.\nOperation cancelled."));
      unfreezeAndCleanup(std::move(selectedCat));
      return;
    }
  }

  // check if we need a seeded ranking, e.g. for single / double elimination rounds
  PlayerPairList initialRanking;
  if (selectedCat->needsInitialRanking())
  {
    DlgSeedingEditor dlg;
    dlg.initSeedingList(selectedCat->getPlayerPairs());
    dlg.setModal(true);
    int result = dlg.exec();

    if (result != QDialog::Accepted)
    {
      unfreezeAndCleanup(std::move(selectedCat));
      return;
    }

    initialRanking = dlg.getSeeding();
    if (initialRanking.isEmpty())
    {
      QMessageBox::warning(this, tr("Run Category"), tr("Can't read seeding.\nOperation cancelled."));
      unfreezeAndCleanup(std::move(selectedCat));
      return;
    }
  }

  /*
   * If we made it to this point, the user accepted all configuration details
   * for the selected category.
   *
   * Let's do a final check before we actually apply the settings and write to
   * the database.
   */
  if (ppListList.count() != 0)
  {
    ERR e = selectedCat->canApplyGroupAssignment(ppListList);
    if (e != OK)
    {
      QString msg = tr("Something is wrong with the group assignment. This shouldn't happen.\nFault:");
      if (e == CATEGORY_NOT_YET_FROZEN)
      {
        msg += tr("Category state not valid for group assignments (STAT != FROZEN)");
      }
      else if (e == CATEGORY_NEEDS_NO_GROUP_ASSIGNMENTS)
      {
        msg += tr("This category needs no group assignments");
      }
      else if (e == INVALID_KO_CONFIG)
      {
        msg += tr("The configuration of the groups and the KO rounds is invalid.");
      }
      else if (e == INVALID_GROUP_NUM)
      {
        msg += tr("The number of assigned groups doesn't match the number of required groups.");
      }
      else if (e == INVALID_PLAYER_COUNT)
      {
        msg += tr("The number of assigned players doesn't match the number of players in the category.");
      }
      else if (e == PLAYER_ALREADY_IN_CATEGORY)
      {
        msg += tr("There are invalid players in the group assignments.");
      }
      else
      {
        msg += tr("Don't now...");
      }
      QMessageBox::warning(this, tr("Run Category"), msg);

      unfreezeAndCleanup(std::move(selectedCat));
      return;
    }
  }

  if (initialRanking.count() != 0)
  {
    ERR e = selectedCat->canApplyInitialRanking(initialRanking);
    if (e != OK)
    {
      QString msg = tr("Something is wrong with the initial ranking. This shouldn't happen.\nFault:");
      if (e == CATEGORY_NOT_YET_FROZEN)
      {
        msg += tr("Category state not valid for setting the initial ranking (STAT != FROZEN)");
      }
      else if (e == CATEGORY_NEEDS_NO_SEEDING)
      {
        msg += tr("This category needs no initial ranking.");
      }
      else if (e == INVALID_PLAYER_COUNT)
      {
        msg += tr("The number of player in the initial ranking doesn't match the number of players in the category.");
      }
      else if (e == PLAYER_ALREADY_IN_CATEGORY)
      {
        msg += tr("There are invalid players in the initial ranking.");
      }
      else
      {
        msg += tr("Don't now...");
      }
      QMessageBox::warning(this, tr("Run Category"), msg);

      unfreezeAndCleanup(std::move(selectedCat));
      return;
    }
  }

  /*
   * If we made it to this point, it is safe to apply the settings and write to
   * the database.
   */
  e = Tournament::getCatMngr()->startCategory(*selectedCat, ppListList, initialRanking);
  if (e != OK)  // should never happen
  {
    throw runtime_error("Unexpected error when starting the category");
  }

  QMessageBox::information(this, tr("Start category"), tr("Category successfully started!"));
}

//----------------------------------------------------------------------------

void CategoryTableView::onCloneCategory()
{
  if (!(hasCategorySelected())) return;

  Category cat = getSelectedCategory();
  CatMngr* cm = Tournament::getCatMngr();

  ERR err = cm->cloneCategory(cat, tr("Clone"));
  if (err == OK) return;

  if ((err == INVALID_NAME) || (err == NAME_EXISTS))
  {
    QString msg = tr("Cloning the category failed due to\n");
    msg += tr("issues defining the clone's name.\n\n");
    msg += tr("No clone has been created.");
    QMessageBox::warning(this, tr("Clone category"), msg);

    return;
  }

  QString msg = tr("Cloning the category partially failed.\n");
  msg += tr("A new category has been created but not all\n");
  msg += tr("settings, players and/or pairs could be copied to the clone.");
  QMessageBox::warning(this, tr("Clone category"), msg);

}

//----------------------------------------------------------------------------

void CategoryTableView::onAddPlayers()
{
  if (!(hasCategorySelected()))
  {
    return;
  }

  cmdBulkAddPlayerToCategory cmd{this, getSelectedCategory()};
  cmd.exec();
}

//----------------------------------------------------------------------------

void CategoryTableView::onRemovePlayers()
{

}

//----------------------------------------------------------------------------

void CategoryTableView::handleIntermediateSeedingForSelectedCat()
{
  if (!(hasCategorySelected())) return;
  unique_ptr<Category> selectedCat = getSelectedCategory().convertToSpecializedObject();
  if (selectedCat == nullptr) return;

  if (selectedCat->getState() != STAT_CAT_WAIT_FOR_INTERMEDIATE_SEEDING)
  {
    return;
  }

  PlayerPairList seedCandidates = selectedCat->getPlayerPairsForIntermediateSeeding();
  if (seedCandidates.isEmpty()) return;

  DlgSeedingEditor dlg;
  dlg.initSeedingList(seedCandidates);
  dlg.setModal(true);
  int result = dlg.exec();

  if (result != QDialog::Accepted)
  {
    return;
  }

  PlayerPairList seeding = dlg.getSeeding();
  if (seeding.isEmpty())
  {
    QMessageBox::warning(this, tr("Intermediate Seeding"), tr("Can't read seeding.\nOperation cancelled."));
    return;
  }

  /*
   * If we made it to this point, we can generate matches for the next round(s)
   */
  ERR e = Tournament::getCatMngr()->continueWithIntermediateSeeding(*selectedCat, seeding);
  if (e != OK)  // should never happen
  {
    throw runtime_error("Unexpected error when applying intermediate seeding");
  }
  QMessageBox::information(this, tr("Continue category"), tr("Matches successfully generated!"));
}

//----------------------------------------------------------------------------

/*
 * NOTE: this method deletes the object that the parameter points to!
 * Do not use the provided pointer anymore after calling this function!
 */
bool CategoryTableView::unfreezeAndCleanup(unique_ptr<Category> selectedCat)
{
  if (selectedCat == 0) return false;
  if (selectedCat->getState() != STAT_CAT_FROZEN) return false;

  // undo all database changes that happened during freezing
  ERR e = Tournament::getCatMngr()->unfreezeConfig(*selectedCat);
  if (e != OK) // this should never be true
  {
    QMessageBox::critical(this, tr("Run Category"),
            tr("Uncaptured error. Category has no valid configuration and can't be started.\nExpect data corruption for this category."));
  }

  // clean-up and return
  return OK;
}

//----------------------------------------------------------------------------

void CategoryTableView::onContextMenuRequested(const QPoint& pos)
{
  // map from scroll area coordinates to global widget coordinates
  QPoint globalPos = viewport()->mapToGlobal(pos);

  // resolve the click coordinates to the table row
  int clickedRow = rowAt(pos.y());
  int clickedCol = columnAt(pos.x());
  bool isCellClicked = ((clickedRow >= 0) && (clickedCol >= 0));

  // check if we have a valid category selection and
  // determine the state of the selected category
  OBJ_STATE catState = STAT_CO_DISABLED;   // an arbitrary, dummy default... not related to categories
  if (hasCategorySelected())
  {
    Category cat = getSelectedCategory();
    catState = cat.getState();
  }

  // set the label of the "run" action depending
  // on the category state
  if (catState == STAT_CAT_WAIT_FOR_INTERMEDIATE_SEEDING)
  {
    actRunCategory->setText(tr("Continue..."));
  } else {
    actRunCategory->setText(tr("Run..."));
  }

  // enable / disable selection-specific actions
  actAddCategory->setEnabled(true);   // always possible
  actRunCategory->setEnabled(isCellClicked &&
                             ((catState == STAT_CAT_CONFIG) || (catState == STAT_CAT_WAIT_FOR_INTERMEDIATE_SEEDING)));
  actRemoveCategory->setEnabled(isCellClicked && (catState == STAT_CAT_CONFIG));
  actCloneCategory->setEnabled(isCellClicked);
  actAddPlayer->setEnabled(isCellClicked);
  actRemovePlayer->setEnabled(isCellClicked);

  // show the context menu
  QAction* selectedItem = contextMenu->exec(globalPos);
}

//----------------------------------------------------------------------------

void CategoryTableView::initContextMenu()
{
  // prepare all action
  actAddCategory = new QAction(tr("Add new"), this);
  actCloneCategory = new QAction(tr("Clone"), this);
  actRunCategory = new QAction(tr("Run..."), this);
  actRemoveCategory = new QAction(tr("Remove..."), this);
  actAddPlayer = new QAction(tr("Add player(s)..."), this);
  actRemovePlayer = new QAction(tr("Remove player(s) from category..."), this);

  // create the context menu and connect it to the actions
  contextMenu = unique_ptr<QMenu>(new QMenu());
  contextMenu->addAction(actAddCategory);
  contextMenu->addAction(actCloneCategory);
  contextMenu->addSeparator();
  contextMenu->addAction(actRunCategory);
  contextMenu->addSeparator();
  contextMenu->addAction(actRemoveCategory);
  contextMenu->addSeparator();
  contextMenu->addAction(actAddPlayer);
  contextMenu->addAction(actRemovePlayer);

  // connect signals and slots
  connect(actAddCategory, SIGNAL(triggered(bool)), this, SLOT(onAddCategory()));
  connect(actCloneCategory, SIGNAL(triggered(bool)), this, SLOT(onCloneCategory()));
  connect(actRunCategory, SIGNAL(triggered(bool)), this, SLOT(onRunCategory()));
  connect(actRemoveCategory, SIGNAL(triggered(bool)), this, SLOT(onRemoveCategory()));
  connect(actAddPlayer, SIGNAL(triggered(bool)), this, SLOT(onAddPlayers()));
  connect(actRemovePlayer, SIGNAL(triggered(bool)), this, SLOT(onRemovePlayers()));
}
//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

