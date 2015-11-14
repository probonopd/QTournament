/*
 * File:   CatTabWidget.cpp
 * Author: volker
 *
 * Created on March 24, 2014, 7:13 PM
 */

#include <iostream>
#include <cassert>
#include <QMessageBox>

#include "CatTabWidget.h"
#include "TournamentDataDefs.h"
#include "dlgGroupAssignment.h"
#include "DlgSeedingEditor.h"

CatTabWidget::CatTabWidget()
{
  ui.setupUi(this);
  
  // connect to the view's signal that tells us that the model has changed
  connect(ui.catTableView, &CategoryTableView::catModelChanged, this, &CatTabWidget::onCatModelChanged);
  
  // connect the selection change signal of the two list widgets
  connect(ui.lwUnpaired, &QListWidget::itemSelectionChanged, this, &CatTabWidget::onUnpairedPlayersSelectionChanged);
  connect(ui.lwPaired, &QListWidget::itemSelectionChanged, this, &CatTabWidget::onPairedPlayersSelectionChanged);
  
  // connect to the change signal of the group config widget
  connect(ui.grpCfgWidget, &GroupConfigWidget::groupConfigChanged, this, &CatTabWidget::onGroupConfigChanged);
  
  // hide unused settings groups
  ui.gbGroups->hide();
  
  // initialize the entries in the drop box
  ui.cbMatchSystem->addItem(tr("Swiss ladder"), static_cast<int>(SWISS_LADDER));
  ui.cbMatchSystem->addItem(tr("Group matches with KO rounds"), static_cast<int>(GROUPS_WITH_KO));
  //ui.cbMatchSystem->addItem(tr("Random matches (for fun tournaments)"), static_cast<int>(RANDOMIZE));
  ui.cbMatchSystem->addItem(tr("Tree-like ranking system"), static_cast<int>(RANKING));
  ui.cbMatchSystem->addItem(tr("Single Elimination"), static_cast<int>(SINGLE_ELIM));
  ui.cbMatchSystem->addItem(tr("Round robin matches"), static_cast<int>(ROUND_ROBIN));
}

//----------------------------------------------------------------------------
    
CatTabWidget::~CatTabWidget()
{
}

//----------------------------------------------------------------------------

void CatTabWidget::onCatModelChanged()
{
  // react on selection changes in the category table
  connect(ui.catTableView->selectionModel(),
	  SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
	  SLOT(onCatSelectionChanged(const QItemSelection&, const QItemSelection&)));
  
  // get information about changed player names or category assignments
  connect(Tournament::getCatMngr(), &CatMngr::playerAddedToCategory, this, &CatTabWidget::onPlayerAddedToCategory);
  connect(Tournament::getCatMngr(), &CatMngr::playerRemovedFromCategory, this, &CatTabWidget::onPlayerRemovedFromCategory);
  connect(Tournament::getCatMngr(), &CatMngr::categoryStatusChanged, this, &CatTabWidget::onCatStateChanged);
  connect(Tournament::getPlayerMngr(), &PlayerMngr::playerRenamed, this, &CatTabWidget::onPlayerRenamed);

  updateControls();
  updatePairs();
}

//----------------------------------------------------------------------------

void CatTabWidget::onCatSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
  updatePairs();
  updateControls();
}

//----------------------------------------------------------------------------

void CatTabWidget::updateControls()
{
  if (!(ui.catTableView->hasCategorySelected()))
  {
    ui.gbGeneric->setEnabled(false);
    ui.gbGroups->hide();
    ui.gbRandom->hide();
    return;
  }
  
  Category selectedCat = ui.catTableView->getSelectedCategory();
  
  //
  // if made it to this point, we can be sure to have a valid category selected
  //
  SEX sex = selectedCat.getSex();
  MATCH_TYPE mt = selectedCat.getMatchType();
  bool isEditEnabled = (selectedCat.getState() == STAT_CAT_CONFIG);

  ui.gbGeneric->setEnabled(isEditEnabled);
  
  // update the list box showing the match system
  int matchSysId = static_cast<int>(selectedCat.getMatchSystem());
  ui.cbMatchSystem->setCurrentIndex(ui.cbMatchSystem->findData(matchSysId, Qt::UserRole));
  
  // activate the applicable group with the special settings
  MATCH_SYSTEM ms = selectedCat.getMatchSystem();
  if (ms == GROUPS_WITH_KO)
  {
    ui.gbGroups->show();
    ui.gbRandom->hide();
    
    // read the current group settings from the database and
    // copy them to the widget
    KO_Config cfg = KO_Config(selectedCat.getParameter_string(GROUP_CONFIG));
    ui.grpCfgWidget->applyConfig(cfg);
  }
  else if (ms == RANDOMIZE)
  {
    ui.gbGroups->hide();
    ui.gbRandom->show();
  }
  else
  {
    ui.gbGroups->hide();
    ui.gbRandom->hide();
  }
  
  // update the match type
  ui.rbSingles->setChecked(mt == SINGLES);
  ui.rbDoubles->setChecked(mt == DOUBLES);
  ui.rbMixed->setChecked(mt == MIXED);
  
  // disable radio buttons for male / female for mixed categories
  ui.rbMen->setEnabled(mt != MIXED);
  ui.rbLadies->setEnabled(mt != MIXED);
  if (mt == MIXED)
  {
    ui.rbMen->hide();
    ui.rbLadies->hide();
  } else {
    ui.rbMen->show();
    ui.rbLadies->show();
  }
  
  // update the applicable sex; this requires an extra hack if the sex is
  // set to "don't care", because in this case, both radio buttons in a
  // radio button group have to be deactivated
  if (sex == DONT_CARE)
  {
    ui.rbgSex->setExclusive(false);
    ui.rbMen->setChecked(false);
    ui.rbLadies->setChecked(false);
    ui.rbgSex->setExclusive(true);
  } else {
    ui.rbMen->setChecked((sex == M) && (mt != MIXED));
    ui.rbLadies->setChecked((sex == F) && (mt != MIXED));
  }
  ui.cbDontCare->setChecked(sex == DONT_CARE);
  
  // the "accept draw" checkbox
  bool allowDraw = selectedCat.getParameter(ALLOW_DRAW).toBool();
  ui.cbDraw->setChecked(allowDraw);

  // the checkbox must be disabled under two conditions:
  // 1) we are in a match system with elimination rounds
  // 2) we have round robins resulting directly in finals
  //
  // In the latter case, we need the second player for the
  // match for 3rd place
  bool enableDrawCheckbox = true;
  if ((ms == SINGLE_ELIM) || (ms == RANKING))
  {
    enableDrawCheckbox = false;
  }
  if (ms == GROUPS_WITH_KO)
  {
    KO_Config cfg = KO_Config(selectedCat.getParameter_string(GROUP_CONFIG));
    if (cfg.getStartLevel() == FINAL) enableDrawCheckbox = false;
  }
  ui.cbDraw->setEnabled(enableDrawCheckbox);

  // FIX ME / TO DO: playing "draw" is not yet implemented, so for now
  // we hardwire the checkbox to disabled.
  ui.cbDraw->setEnabled(false);
  
  // the score spinboxes
  int drawScore = selectedCat.getParameter(DRAW_SCORE).toInt();
  int winScore = selectedCat.getParameter(WIN_SCORE).toInt();
  ui.sbDrawScore->setValue(drawScore);
  ui.sbWinScore->setValue(winScore);
  if (allowDraw)
  {
    ui.sbWinScore->setMinimum(drawScore + 1);
    ui.sbWinScore->setMaximum(99);
    ui.sbDrawScore->setMaximum(winScore - 1);
    ui.sbDrawScore->setMinimum(1);
    ui.sbDrawScore->show();
  } else {
    ui.sbWinScore->setMinimum(1);
    ui.sbWinScore->setMaximum(99);
    ui.sbDrawScore->hide();
  }
  
  // FIX ME / TO DO: playing "draw" is not yet implemented, so for now
  // we hardwire the checkbox to "hidden".
  ui.sbDrawScore->hide();
  ui.sbWinScore->hide();

  // group box for configuring player pairs
  if (mt == SINGLES)
  {
    ui.gbPairButtons->setEnabled(false);
    ui.lwPaired->setEnabled(false);
  } else {
    ui.gbPairButtons->setEnabled(true && isEditEnabled);
    ui.lwPaired->setEnabled(true && isEditEnabled);
  }

  // disable controls if editing is no longer permitted
  // because the category is no longer in state CONFIG
  ui.gbGroups->setEnabled(isEditEnabled);
  ui.gbRandom->setEnabled(isEditEnabled);

  // change the label of the "run" button and enable or
  // disable it
  OBJ_STATE catState = selectedCat.getState();
  if (catState == STAT_CAT_WAIT_FOR_INTERMEDIATE_SEEDING)
  {
    ui.btnRunCategory->setText("Continue");
  } else {
    ui.btnRunCategory->setText("Run");
  }
  bool enableRunButton = ((catState == STAT_CAT_CONFIG) || (catState == STAT_CAT_WAIT_FOR_INTERMEDIATE_SEEDING));
  ui.btnRunCategory->setEnabled(enableRunButton);
}

//----------------------------------------------------------------------------

void CatTabWidget::onCbDrawChanged(bool newState)
{
  Category c = ui.catTableView->getSelectedCategory();
  c.setParameter(ALLOW_DRAW, newState);
  updateControls();
}

//----------------------------------------------------------------------------

void CatTabWidget::onWinScoreChanged(int newVal)
{
  Category c = ui.catTableView->getSelectedCategory();
  c.setParameter(WIN_SCORE, newVal);
  updateControls();
}

//----------------------------------------------------------------------------
    
void CatTabWidget::onDrawScoreChanged(int newVal)
{
  Category c = ui.catTableView->getSelectedCategory();
  c.setParameter(DRAW_SCORE, newVal);
  updateControls();
}

//----------------------------------------------------------------------------

/**
 * Updates the list widgets with the player pairs. Should only be called
 * when the selected category changes.
 * 
 * This method disables the pairing buttons.
 */
void CatTabWidget::updatePairs()
{
  // remove all entries
  ui.lwUnpaired->clear();
  ui.lwPaired->clear();
  unpairedPlayerId1 = -1;
  unpairedPlayerId2 = -1;
  ui.btnPair->setEnabled(false);
  ui.btnSplit->setEnabled(false);
  
  // if no model is loaded or no category is selected, simply returns
  if (!(ui.catTableView->hasCategorySelected())) return;
  
  // get the pair data from the selected category
  Category selCat = ui.catTableView->getSelectedCategory();
  PlayerPairList pairList = selCat.getPlayerPairs();
  
  int nextPairedItemCount = 1;
  int nextUnPairedItemCount = 1;
  for (int i=0; i < pairList.count(); i++)
  {
    PlayerPair pp = pairList.at(i);
    
    if (pp.hasPlayer2())
    {
      QString itemLabel = QString("%1. %2").arg(nextPairedItemCount).arg(pp.getDisplayName());
      QListWidgetItem* item = new QListWidgetItem(itemLabel);
      item->setData(Qt::UserRole, pp.getPairId());
      ui.lwPaired->addItem(item);
      ++nextPairedItemCount;
    } else {
      QString itemLabel = QString("%1. %2").arg(nextUnPairedItemCount).arg(pp.getDisplayName());
      QListWidgetItem* item = new QListWidgetItem(itemLabel);
      item->setData(Qt::UserRole, pp.getPlayer1().getId());
      ui.lwUnpaired->addItem(item);
      ++nextUnPairedItemCount;
    }
  }
  
  // update the "required players" label in the GroupConfigWidget
  if (selCat.getMatchType() == SINGLES)
  {
    ui.grpCfgWidget->setRequiredPlayersCount(ui.lwUnpaired->count());
  } else {
    ui.grpCfgWidget->setRequiredPlayersCount(ui.lwPaired->count());
  }
}

//----------------------------------------------------------------------------

void CatTabWidget::onUnpairedPlayersSelectionChanged()
{
  QList<QListWidgetItem *> selPlayers = ui.lwUnpaired->selectedItems();
  
  if (selPlayers.count() == 0)
  {
    unpairedPlayerId1 = -1;
    unpairedPlayerId2 = -1;
  }
  
  if (selPlayers.count() == 1)
  {
    unpairedPlayerId1 = selPlayers.at(0)->data(Qt::UserRole).toInt();
    unpairedPlayerId2 = -1;
  }
  
  if (selPlayers.count() == 2)
  {
    int id1 = selPlayers.at(0)->data(Qt::UserRole).toInt();
    int id2 = selPlayers.at(1)->data(Qt::UserRole).toInt();
    
    unpairedPlayerId2 = (id1 == unpairedPlayerId1) ? id2 : id1;
  }
  
  if (selPlayers.count() == 3)
  {
    int id1 = selPlayers.at(0)->data(Qt::UserRole).toInt();
    int id2 = selPlayers.at(1)->data(Qt::UserRole).toInt();
    int id3 = selPlayers.at(2)->data(Qt::UserRole).toInt();
    
    // remove the oldest selection, which is the entry in in
    // unpairedPlayerId1
    int idToBeDeselected = unpairedPlayerId1;
    
    // shift the stored selection by one
    unpairedPlayerId1 = unpairedPlayerId2;
    
    // find out which selected item is the newest one
    if ((id1 != idToBeDeselected) && (id1 != unpairedPlayerId1))
    {
      unpairedPlayerId2 = id1;
    }
    else if ((id2 != idToBeDeselected) && (id2 != unpairedPlayerId1))
    {
      unpairedPlayerId2 = id2;
    }
    else {
      unpairedPlayerId2 = id3;
    }
    
    // find which item is to be deselected
    if (idToBeDeselected == id1)
    {
      selPlayers.at(0)->setSelected(false);
    }
    else if (idToBeDeselected == id2)
    {
      selPlayers.at(1)->setSelected(false);
    }
    else
    {
      selPlayers.at(2)->setSelected(false);
    }
  }
  
  // fall-back: deselect all
  if (selPlayers.count() > 3)
  {
    for (int i=0; i < selPlayers.count(); i++ )
    {
      selPlayers.at(i)->setSelected(false);
    }
    unpairedPlayerId1 = -1;
    unpairedPlayerId2 = -1;
  }
  
  // update the "pair" button, if necessary
  bool canPair = false;
  if ((unpairedPlayerId1 > 0) && (unpairedPlayerId2 > 0))
  {
    Player p1 = Tournament::getPlayerMngr()->getPlayer(unpairedPlayerId1);
    Player p2 = Tournament::getPlayerMngr()->getPlayer(unpairedPlayerId2);
    
    Category c = ui.catTableView->getSelectedCategory();
    canPair = (c.canPairPlayers(p1, p2) == OK);
  }
  ui.btnPair->setEnabled(canPair);
}

//----------------------------------------------------------------------------

void CatTabWidget::onBtnPairClicked()
{
  QList<QListWidgetItem *> selPlayers = ui.lwUnpaired->selectedItems();
  
  //
  // I'll repeat a few of the checks here, in case the "selection-changed" handler
  // has not been called and the current selection is invalid for pairing
  //
  if (selPlayers.count() != 2)
  {
    QMessageBox::warning(this, tr("Need exactly two selected players for pairing!"), tr("Pairing impossible"));
    updatePairs();
    return;
  }
  
  int id1 = selPlayers.at(0)->data(Qt::UserRole).toInt();
  int id2 = selPlayers.at(1)->data(Qt::UserRole).toInt();
  Category c = ui.catTableView->getSelectedCategory();
  Player p1 = Tournament::getPlayerMngr()->getPlayer(id1);
  Player p2 = Tournament::getPlayerMngr()->getPlayer(id2);
  if (c.canPairPlayers(p1, p2) != OK)
  {
    QMessageBox::warning(this, tr("These two players can't be paired for this category!"), tr("Pairing impossible"));
    updatePairs();
    return;
  }
  
  ERR e = Tournament::getCatMngr()->pairPlayers(c, p1, p2);
  if (e != OK)
  {
    QMessageBox::warning(this, tr("Something went wrong during pairing. This shouldn't happen. For the records: Error code = ") + e, tr("Pairing impossible"));
  }
  
  updatePairs();
}

//----------------------------------------------------------------------------
    
void CatTabWidget::onPairedPlayersSelectionChanged()
{
  QList<QListWidgetItem *> selPairs = ui.lwPaired->selectedItems();
  
  ui.btnSplit->setEnabled(selPairs.count() != 0);
}

//----------------------------------------------------------------------------

void CatTabWidget::onBtnSplitClicked()
{
  QList<QListWidgetItem *> selPairs = ui.lwPaired->selectedItems();
  Category c = ui.catTableView->getSelectedCategory();
  CatMngr* cmngr = Tournament::getCatMngr();
  
  for (int i=0; i < selPairs.count(); i++)
  {
    int pairId = selPairs.at(i)->data(Qt::UserRole).toInt();
    ERR e = cmngr->splitPlayers(c, pairId);
    if (e != OK)
    {
      QMessageBox::warning(this, tr("Something went wrong during splitting. This shouldn't happen. For the records: Error code = ") + e, tr("Splitting impossible"));
    }
  }
  
  updatePairs();
}

//----------------------------------------------------------------------------

void CatTabWidget::onMatchTypeButtonClicked(int btn)
{
  Category selCat = ui.catTableView->getSelectedCategory();
  
  MATCH_TYPE oldType = selCat.getMatchType();
  
  MATCH_TYPE newType = SINGLES;
  if (ui.rbDoubles->isChecked()) newType = DOUBLES;
  if (ui.rbMixed->isChecked()) newType = MIXED;
  
  // do we actually have a change?
  if (oldType == newType)
  {
    return;  // nope, no action required
  }
  
  // change the type
  ERR e = selCat.setMatchType(newType);
  if (e != OK)
  {
    QMessageBox::warning(this, tr("Error"), tr("Could change match type. Error number = ") + e);
  }
  
  // in case the change wasn't successful, restore the old, correct type;
  // furthermore, the pairs could have changed. So basically we need to
  // reset the whole widget
  updatePairs();
  updateControls();
}

//----------------------------------------------------------------------------

void CatTabWidget::onSexClicked(int btn)
{
  // the new sex must be either M or F
  SEX newSex = M;
  if (ui.rbLadies->isChecked()) newSex = F;
  
  // in any case, we can de-select the "don't care" box
  ui.cbDontCare->setChecked(false);
  
  // actually change the sex
  Category selCat = ui.catTableView->getSelectedCategory();
  ERR e = selCat.setSex(newSex);
  if (e != OK)
  {
    QMessageBox::warning(this, tr("Error"), tr("Could change sex. Error number = ") + e);
  }
  
  // in case the change wasn't successful, restore the old, correct type;
  // furthermore, the pairs could have changed. So basically we need to
  // reset the whole widget
  updatePairs();
  updateControls();
}

//----------------------------------------------------------------------------

void CatTabWidget::onDontCareClicked()
{
  bool newState = ui.cbDontCare->isChecked();
  Category selCat = ui.catTableView->getSelectedCategory();
  
  // unless we are in a mixed category, "don't care" can only be deactivated
  // by selecting a specific sex
  if (!newState && (selCat.getMatchType() != MIXED))
  {
    QString msg = tr("Please deactivate 'Don't care' by selecting a specific sex!");
    QMessageBox::information(this, tr("Change category sex"), msg);
    
    // re-check the checkbox
    ui.cbDontCare->setChecked(true);
    
    return;
  }
  
  SEX newSex = DONT_CARE;
  
  // if we are in a mixed category, allow simple de-activation of "Don't care"
  if (!newState && (selCat.getMatchType() == MIXED))
  {
    // set a dummy default value that is not "Don't care"
    newSex = M;
  }
  
  // un-check "Men" and "Ladies" if "Don't care" was activated
  if (newState)
  {
    ui.rbgSex->setExclusive(false);
    ui.rbMen->setChecked(false);
    ui.rbLadies->setChecked(false);
    ui.rbgSex->setExclusive(true);
  }
  
  // set the new value
  ERR e = selCat.setSex(newSex);
  if (e != OK)
  {
    QMessageBox::warning(this, tr("Error"), tr("Could change sex. Error number = ") + e);
  }
  
  // in case the change wasn't successful, restore the old, correct type;
  // furthermore, the pairs could have changed. So basically we need to
  // reset the whole widget
  updatePairs();
  updateControls();
}

//----------------------------------------------------------------------------

void CatTabWidget::onBtnAddCatClicked()
{
  // try to create new categories using a
  // canonical name until it finally succeeds
  ERR e = NAME_EXISTS;
  int cnt = 0;
  while (e != OK)
  {
    QString newCatName = tr("New Category ") + QString::number(cnt);
    
    e = Tournament::getCatMngr()->createNewCategory(newCatName);
    cnt++;
  }

}

//----------------------------------------------------------------------------

void CatTabWidget::onMatchSystemChanged(int newIndex)
{
  // get the new match system
  int msId = ui.cbMatchSystem->itemData(newIndex, Qt::UserRole).toInt();
  MATCH_SYSTEM ms = static_cast<MATCH_SYSTEM>(msId);
  
  if (!(ui.catTableView->hasCategorySelected())) return;
  
  Category selectedCat = ui.catTableView->getSelectedCategory();
  Tournament::getCatMngr()->setMatchSystem(selectedCat, ms);
  updateControls();
}

//----------------------------------------------------------------------------

void CatTabWidget::onGroupConfigChanged(const KO_Config& newCfg)
{
  if (!(ui.catTableView->hasCategorySelected())) return;
  
  Category selectedCat = ui.catTableView->getSelectedCategory();
  selectedCat.setParameter(GROUP_CONFIG, newCfg.toString());
}

//----------------------------------------------------------------------------

void CatTabWidget::onPlayerAddedToCategory(const Player& p, const Category& c)
{
  if (!(ui.catTableView->hasCategorySelected())) return;
  
  Category selectedCat = ui.catTableView->getSelectedCategory();
  if (selectedCat != c) return;
  
  updateControls();
  updatePairs();
}

//----------------------------------------------------------------------------
    
void CatTabWidget::onPlayerRemovedFromCategory(const Player& p, const Category& c)
{
  if (!(ui.catTableView->hasCategorySelected())) return;
  
  Category selectedCat = ui.catTableView->getSelectedCategory();
  if (selectedCat != c) return;
  
  updateControls();
  updatePairs();
}

//----------------------------------------------------------------------------
    
void CatTabWidget::onPlayerRenamed(const Player& p)
{
  updatePairs();
}
//----------------------------------------------------------------------------

void CatTabWidget::onBtnRunCatClicked()
{
  if (!(ui.catTableView->hasCategorySelected())) return;
  
  unique_ptr<Category> selectedCat = ui.catTableView->getSelectedCategory().convertToSpecializedObject();

  // branch here to another function if the button was pressed to
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

/*
 * NOTE: this method deletes the object that the parameter points to!
 * Do not use the provided pointer anymore after calling this function!
 */
bool CatTabWidget::unfreezeAndCleanup(unique_ptr<Category> selectedCat)
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

void CatTabWidget::handleIntermediateSeedingForSelectedCat()
{
  if (!(ui.catTableView->hasCategorySelected())) return;
  unique_ptr<Category> selectedCat = ui.catTableView->getSelectedCategory().convertToSpecializedObject();
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

/**
  Is being called if the CategoryManager changes the state of a Category. Updates
  the UI accordingsly.
  */
void CatTabWidget::onCatStateChanged(const Category &c, const OBJ_STATE fromState, const OBJ_STATE toState)
{
  updateControls();
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


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------

