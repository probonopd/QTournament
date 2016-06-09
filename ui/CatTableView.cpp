/*
 *    This is QTournament, a badminton tournament management program.
 *    Copyright (C) 2014 - 2016  Volker Knollmann
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdexcept>

#include <QInputDialog>
#include <QString>
#include <QMessageBox>

#include "CatTableView.h"
#include "MainFrame.h"
#include "Category.h"
#include "CatMngr.h"
#include "PlayerMngr.h"
#include "ui/delegates/CatItemDelegate.h"
#include "ui/DlgSeedingEditor.h"
#include "ui/dlgGroupAssignment.h"
#include "ui/commonCommands/cmdBulkAddPlayerToCat.h"
#include "ui/commonCommands/cmdBulkRemovePlayersFromCat.h"
#include "ui/commonCommands/cmdCreateNewPlayerInCat.h"
#include "ui/commonCommands/cmdImportSinglePlayerFromExternalDatabase.h"

#include "CatMngr.h"

CategoryTableView::CategoryTableView(QWidget* parent)
:QTableView(parent), db(nullptr), curCatTableModel(nullptr), catItemDelegate(nullptr)
{
  // an empty model for clearing the table when
  // no tournament is open
  emptyModel = new QStringListModel();
  defaultDelegate = itemDelegate();

  // prepare a proxy model to support sorting by columns
  sortedModel = new QSortFilterProxyModel();
  sortedModel->setSourceModel(emptyModel);
  setModel(sortedModel);

  // set an initial default sorting column
  sortByColumn(CategoryTableModel::COL_NAME, Qt::AscendingOrder);

  // initiate the model(s) as empty
  setDatabase(nullptr);
  
  // handle context menu requests
  setContextMenuPolicy(Qt::CustomContextMenu);
  connect(this, SIGNAL(customContextMenuRequested(const QPoint&)),
          this, SLOT(onContextMenuRequested(const QPoint&)));

  // prep the context menu
  initContextMenu();
}

//----------------------------------------------------------------------------
    
CategoryTableView::~CategoryTableView()
{
  delete emptyModel;
  delete sortedModel;
  if (curCatTableModel != nullptr) delete curCatTableModel;
  if (defaultDelegate != nullptr) delete defaultDelegate;
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

void CategoryTableView::setDatabase(TournamentDB* _db)
{
  // According to the Qt documentation, the selection model
  // has to be explicitly deleted by the user
  //
  // Thus we store the model pointer for later deletion
  // QItemSelectionModel *oldSelectionModel = selectionModel();

  // set the new data model
  CategoryTableModel* newCatTabModel = nullptr;
  if (_db != nullptr)
  {
    newCatTabModel = new CategoryTableModel(_db);
    sortedModel->setSourceModel(newCatTabModel);

    // define a delegate for drawing the category items
    catItemDelegate = make_unique<CatItemDelegate>(_db, this);
    catItemDelegate->setProxy(sortedModel);
    setItemDelegate(catItemDelegate.get());
  } else {
    sortedModel->setSourceModel(emptyModel);
    setItemDelegate(defaultDelegate);
  }
  emit catModelChanged();

  // delete the old data model, if it was a
  // CategoryTableModel instance
  if (curCatTableModel != nullptr)
  {
    delete curCatTableModel;
  }

  // store the new CategoryTableModel instance, if any
  curCatTableModel = newCatTabModel;

  // delete the old selection model
  //delete oldSelectionModel;

  // update the database pointer and set the widget's enabled state
  db = _db;
  setEnabled(db != nullptr);
}

//----------------------------------------------------------------------------

void CategoryTableView::resizeEvent(QResizeEvent *event)
{
  // call parent function
  QTableView::resizeEvent(event);

  // distribute the available space evenly over the columns
  int nCol = model()->columnCount();
  int colWidth = (nCol > 0) ? width() / nCol : 0;
  int totalWidth = 0;
  for (int i=0; i < nCol; ++i)
  {
    if (i != (nCol - 1))
    {
      setColumnWidth(i, colWidth);
      totalWidth += colWidth;
    } else {
      // compensate for rounding errors by setting
      // the width of the last column to the not already
      // "consumed" width
      setColumnWidth(i, width() - totalWidth);
    }
  }

  // finish event processing
  event->accept();
}

//----------------------------------------------------------------------------

Category CategoryTableView::getSelectedCategory()
{
  if (!(hasCategorySelected()))
  {
    throw std::invalid_argument("No category selected");
  }
  
  QModelIndexList indexes = selectionModel()->selection().indexes();
  
  CatMngr cm{db};
  int selectedSourceRow = sortedModel->mapToSource(indexes.at(0)).row();
  return cm.getCategoryBySeqNum(selectedSourceRow);
}

//----------------------------------------------------------------------------

void CategoryTableView::onCategoryDoubleClicked(const QModelIndex& index)
{
  if (!(hasCategorySelected())) return;

  CatMngr cm{db};
  Category selectedCat = getSelectedCategory();
  
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
    ERR e = cm.renameCategory(selectedCat, newName);

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

QModelIndex CategoryTableView::mapToSource(const QModelIndex& proxyIndex)
{
  return sortedModel->mapToSource(proxyIndex);
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

    CatMngr cm{db};
    e = cm.createNewCategory(newCatName);
    ++cnt;
  }
}

//----------------------------------------------------------------------------

void CategoryTableView::onRemoveCategory()
{
  if (!(hasCategorySelected())) return;

  Category cat = getSelectedCategory();
  CatMngr cm{db};

  // can the category be deleted at all?
  ERR err = cm.canDeleteCategory(cat);

  // category is already beyond config state
  // or not all players are removable
  if (err != OK)
  {
    QString msg = tr("The category has already been started.\n\n");
    msg += tr("You can choose to force-delete the category anyway,\n");
    msg += tr("but the following data will be IRREVOCABLY DELETED:\n\n");
    msg += tr("\tPlayer and player pairs assigned to this category\n");
    msg += tr("\tAll matches (scheduled, staged, running or finished)\n");
    msg += tr("\tAll match results and rankings\n\n");
    msg += tr("Do you want to proceed?");
    int result = QMessageBox::warning(this, tr("Delete category"), msg, QMessageBox::Yes | QMessageBox::No);
    if (result != QMessageBox::Yes) return;

    // ask for a second confirmation
    msg = tr("Please confirm again that you want to proceed\n");
    msg += tr("and delete the category.\n\n");
    msg += tr("AGAIN: THIS STEP CANNOT BE UNDONE!");
    result = QMessageBox::warning(this, tr("Delete category"), msg, QMessageBox::Yes | QMessageBox::No);
    if (result != QMessageBox::Yes) return;

    // okay, force-delete the category
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    cm.deleteRunningCategory(cat);
    QApplication::restoreOverrideCursor();
    return;
  }

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
  err = cm.deleteCategory(cat);
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

  // make sure that the category contains at least three players
  int playerCount = selectedCat->getAllPlayersInCategory().size();
  if (playerCount < 3)
  {
    QMessageBox::critical(this, tr("Run Category"),
      tr("You need to have at least three players in a category!"));
    return;
  }

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
  else if (e == INVALID_PLAYER_COUNT)
  {
    QMessageBox::critical(this, tr("Run Category"),
      tr("The number of players / player pairs in this category is not sufficient\nto start the category."));
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

  CatMngr cm{db};
  e = cm.freezeConfig(*selectedCat);
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
  vector<PlayerPairList> ppListList;
  if (selectedCat->needsGroupInitialization())
  {
    dlgGroupAssignment dlg(db, this, *selectedCat);
    dlg.setModal(true);
    int result = dlg.exec();

    if (result != QDialog::Accepted)
    {
      unfreezeAndCleanup(std::move(selectedCat));
      return;
    }

    ppListList = dlg.getGroupAssignments();
    if (ppListList.empty())
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
    DlgSeedingEditor dlg{db, this};
    dlg.initSeedingList(selectedCat->getPlayerPairs());
    dlg.setModal(true);
    int result = dlg.exec();

    if (result != QDialog::Accepted)
    {
      unfreezeAndCleanup(std::move(selectedCat));
      return;
    }

    initialRanking = dlg.getSeeding();
    if (initialRanking.empty())
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
  if (ppListList.size() != 0)
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

  if (initialRanking.size() != 0)
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
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  e = cm.startCategory(*selectedCat, ppListList, initialRanking);
  QApplication::restoreOverrideCursor();
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
  CatMngr cm{db};

  ERR err = cm.cloneCategory(cat, tr("Clone"));
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
  if (!(hasCategorySelected()))
  {
    return;
  }

  cmdBulkRemovePlayersFromCategory cmd{this, getSelectedCategory()};
  cmd.exec();
}

//----------------------------------------------------------------------------

void CategoryTableView::onCreatePlayer()
{
  if (!(hasCategorySelected()))
  {
    return;
  }

  cmdCreateNewPlayerInCat cmd{this, getSelectedCategory()};
  cmd.exec();
}

//----------------------------------------------------------------------------

void CategoryTableView::onImportPlayer()
{
  if (!(hasCategorySelected()))
  {
    return;
  }

  cmdImportSinglePlayerFromExternalDatabase cmd{db, this, getSelectedCategory().getId()};
  cmd.exec();
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
  if (seedCandidates.empty()) return;

  DlgSeedingEditor dlg{db, this};
  dlg.initSeedingList(seedCandidates);
  dlg.setModal(true);
  int result = dlg.exec();

  if (result != QDialog::Accepted)
  {
    return;
  }

  PlayerPairList seeding = dlg.getSeeding();
  if (seeding.empty())
  {
    QMessageBox::warning(this, tr("Intermediate Seeding"), tr("Can't read seeding.\nOperation cancelled."));
    return;
  }

  /*
   * If we made it to this point, we can generate matches for the next round(s)
   */
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  CatMngr cm{db};
  ERR e = cm.continueWithIntermediateSeeding(*selectedCat, seeding);
  QApplication::restoreOverrideCursor();
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
  CatMngr cm{db};
  ERR e = cm.unfreezeConfig(*selectedCat);
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
  bool canAddPlayers = false;
  if (hasCategorySelected())
  {
    Category cat = getSelectedCategory();
    catState = cat.getState();
    canAddPlayers = cat.canAddPlayers();
  }

  // set the label of the "run" action depending
  // on the category state
  if (catState == STAT_CAT_WAIT_FOR_INTERMEDIATE_SEEDING)
  {
    actRunCategory->setText(tr("Continue..."));
  } else {
    actRunCategory->setText(tr("Run..."));
  }

  PlayerMngr pm{db};

  // enable / disable selection-specific actions
  actAddCategory->setEnabled(true);   // always possible
  actRunCategory->setEnabled(isCellClicked &&
                             ((catState == STAT_CAT_CONFIG) || (catState == STAT_CAT_WAIT_FOR_INTERMEDIATE_SEEDING)));
  actRemoveCategory->setEnabled(isCellClicked);
  actCloneCategory->setEnabled(isCellClicked);
  actAddPlayer->setEnabled(canAddPlayers);
  actImportPlayerToCat->setEnabled(canAddPlayers && pm.hasExternalPlayerDatabaseAvailable());
  actRemovePlayer->setEnabled(isCellClicked);
  actCreateNewPlayerInCat->setEnabled(canAddPlayers);   // TODO: this could be too restrictive for future purposes (e.g., random matches)

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
  actAddPlayer = new QAction(tr("Add existing player(s)..."), this);
  actRemovePlayer = new QAction(tr("Remove player(s) from category..."), this);
  actCreateNewPlayerInCat = new QAction(tr("Create new player in this category..."), this);
  actImportPlayerToCat = new QAction(tr("Import player to this category..."), this);

  // create the context menu and connect it to the actions
  contextMenu = make_unique<QMenu>();
  contextMenu->addAction(actAddCategory);
  contextMenu->addAction(actCloneCategory);
  contextMenu->addSeparator();
  contextMenu->addAction(actRunCategory);
  contextMenu->addSeparator();
  contextMenu->addAction(actRemoveCategory);
  contextMenu->addSeparator();
  contextMenu->addAction(actAddPlayer);
  contextMenu->addAction(actCreateNewPlayerInCat);
  contextMenu->addAction(actImportPlayerToCat);
  contextMenu->addAction(actRemovePlayer);

  // connect signals and slots
  connect(actAddCategory, SIGNAL(triggered(bool)), this, SLOT(onAddCategory()));
  connect(actCloneCategory, SIGNAL(triggered(bool)), this, SLOT(onCloneCategory()));
  connect(actRunCategory, SIGNAL(triggered(bool)), this, SLOT(onRunCategory()));
  connect(actRemoveCategory, SIGNAL(triggered(bool)), this, SLOT(onRemoveCategory()));
  connect(actAddPlayer, SIGNAL(triggered(bool)), this, SLOT(onAddPlayers()));
  connect(actRemovePlayer, SIGNAL(triggered(bool)), this, SLOT(onRemovePlayers()));
  connect(actCreateNewPlayerInCat, SIGNAL(triggered(bool)), this, SLOT(onCreatePlayer()));
  connect(actImportPlayerToCat, SIGNAL(triggered(bool)), this, SLOT(onImportPlayer()));
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
    

