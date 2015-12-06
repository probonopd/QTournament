/*
 *    This is QTournament, a badminton tournament management program.
 *    Copyright (C) 2014 - 2015  Volker Knollmann
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

#include <iostream>
#include <cassert>
#include <QMessageBox>

#include "CatTabWidget.h"
#include "TournamentDataDefs.h"
#include "dlgGroupAssignment.h"
#include "DlgSeedingEditor.h"
#include "PlayerMngr.h"
#include "Player.h"
#include "ui/commonCommands/cmdRegisterPlayer.h"
#include "ui/commonCommands/cmdUnregisterPlayer.h"
#include "ui/commonCommands/cmdRemovePlayerFromCategory.h"
#include "ui/commonCommands/cmdBulkAddPlayerToCat.h"
#include "ui/commonCommands/cmdBulkRemovePlayersFromCat.h"
#include "ui/commonCommands/cmdMoveOrCopyPlayerToCategory.h"
#include "ui/commonCommands/cmdMoveOrCopyPairToCategory.h"
#include "ui/commonCommands/cmdCreateNewPlayerInCat.h"
#include "MenuGenerator.h"

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

  // setup the context menu(s)
  initContextMenu();

  // tell the list widgets to emit signals if a context menu is requested
  ui.lwUnpaired->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(ui.lwUnpaired, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onUnpairedContextMenuRequested(QPoint)));
  ui.lwPaired->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(ui.lwPaired, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onPairedContextMenuRequested(QPoint)));
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
  auto tnmt = Tournament::getActiveTournament();
  connect(tnmt->getCatMngr(), &CatMngr::playerAddedToCategory, this, &CatTabWidget::onPlayerAddedToCategory);
  connect(tnmt->getCatMngr(), &CatMngr::playerRemovedFromCategory, this, &CatTabWidget::onPlayerRemovedFromCategory);
  connect(tnmt->getCatMngr(), &CatMngr::categoryStatusChanged, this, &CatTabWidget::onCatStateChanged);
  connect(tnmt->getPlayerMngr(), &PlayerMngr::playerRenamed, this, &CatTabWidget::onPlayerRenamed);
  connect(tnmt->getPlayerMngr(), SIGNAL(playerStatusChanged(int,int,OBJ_STATE,OBJ_STATE)), this, SLOT(onPlayerStateChanged(int,int,OBJ_STATE,OBJ_STATE)));

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
    ui.lwPaired->setEnabled(true);
    ui.lwPaired->setSelectionMode(isEditEnabled ? QListWidget::SingleSelection : QListWidget::NoSelection);
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
      QString itemLabel = QString("%1. %2").arg(nextPairedItemCount).arg(pp.getDisplayName(0, true));
      QListWidgetItem* item = new QListWidgetItem(itemLabel);
      item->setData(Qt::UserRole, pp.getPairId());
      ui.lwPaired->addItem(item);
      ++nextPairedItemCount;
    } else {
      QString itemLabel = QString("%1. %2").arg(nextUnPairedItemCount).arg(pp.getDisplayName(0, true));
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

void CatTabWidget::initContextMenu()
{
  //
  // The context menu for the list widget for the UNPAIRED players
  //

  // prepare the actions
  actRemovePlayer = new QAction(tr("Remove this player from category"), this);
  actRegister = new QAction(tr("Register"), this);
  actUnregister = new QAction(tr("Undo registration"), this);
  actAddPlayer = new QAction(tr("Add player(s)..."), this);
  actBulkRemovePlayers = new QAction(tr("Remove player(s)..."), this);
  actCreateNewPlayerInCat = new QAction(tr("Create new player in this category..."), this);
  actImportPlayerToCat = new QAction(tr("Import player to this category..."), this);

  // create the context menu and connect it to the actions
  lwUnpairedContextMenu = unique_ptr<QMenu>(new QMenu());
  listOfCats_CopyPlayerSubmenu = make_unique<QMenu>();
  listOfCats_CopyPlayerSubmenu->setTitle(tr("Copy player to"));
  listOfCats_MovePlayerSubmenu = make_unique<QMenu>();
  listOfCats_MovePlayerSubmenu->setTitle(tr("Move player to"));
  lwUnpairedContextMenu->addAction(actRegister);
  lwUnpairedContextMenu->addAction(actUnregister);
  lwUnpairedContextMenu->addSeparator();
  lwUnpairedContextMenu->addMenu(listOfCats_CopyPlayerSubmenu.get());
  lwUnpairedContextMenu->addMenu(listOfCats_MovePlayerSubmenu.get());
  lwUnpairedContextMenu->addAction(actRemovePlayer);
  lwUnpairedContextMenu->addSeparator();
  lwUnpairedContextMenu->addAction(actAddPlayer);
  lwUnpairedContextMenu->addAction(actCreateNewPlayerInCat);
  lwUnpairedContextMenu->addAction(actImportPlayerToCat);
  lwUnpairedContextMenu->addAction(actBulkRemovePlayers);

  // connect signals and slots
  connect(actRemovePlayer, SIGNAL(triggered(bool)), this, SLOT(onRemovePlayerFromCat()));
  connect(actRegister, SIGNAL(triggered(bool)), this, SLOT(onRegisterPlayer()));
  connect(actUnregister, SIGNAL(triggered(bool)), this, SLOT(onUnregisterPlayer()));
  connect(actAddPlayer, SIGNAL(triggered(bool)), this, SLOT(onAddPlayerToCat()));
  connect(actBulkRemovePlayers, SIGNAL(triggered(bool)), this, SLOT(onBulkRemovePlayersFromCat()));
  connect(actCreateNewPlayerInCat, SIGNAL(triggered(bool)), this, SLOT(onCreatePlayer()));
  connect(actImportPlayerToCat, SIGNAL(triggered(bool)), this, SLOT(onImportPlayer()));


  //
  // The context menu for the list widget for the PAIRED players
  //

  // prepare the actions
  actSplitPair = new QAction(tr("Split"), this);

  // create the context menu and connect it to the actions
  lwPairsContextMenu = unique_ptr<QMenu>(new QMenu());
  listOfCats_CopyPairSubmenu = make_unique<QMenu>();
  listOfCats_CopyPairSubmenu->setTitle(tr("Copy pair to"));
  listOfCats_MovePairSubmenu = make_unique<QMenu>();
  listOfCats_MovePairSubmenu->setTitle(tr("Move pair to"));
  lwPairsContextMenu->addAction(actSplitPair);
  lwPairsContextMenu->addSeparator();
  lwPairsContextMenu->addMenu(listOfCats_CopyPairSubmenu.get());
  lwPairsContextMenu->addMenu(listOfCats_MovePairSubmenu.get());

  // connect signals and slots
  connect(actSplitPair, SIGNAL(triggered(bool)), this, SLOT(onBtnSplitClicked()));
}

//----------------------------------------------------------------------------

upPlayer CatTabWidget::lwUnpaired_getSelectedPlayer() const
{
  // we can only handle exactly one selected item
  if (ui.lwUnpaired->selectedItems().length() != 1)
  {
    return nullptr;
  }

  auto selItem = ui.lwUnpaired->selectedItems().at(0);
  int playerId = selItem->data(Qt::UserRole).toInt();

  auto tnmt = Tournament::getActiveTournament();
  return tnmt->getPlayerMngr()->getPlayer_up(playerId);
}

//----------------------------------------------------------------------------

unique_ptr<PlayerPair> CatTabWidget::lwPaired_getSelectedPair() const
{
  // we can only handle exactly one selected item
  if (ui.lwPaired->selectedItems().length() != 1)
  {
    return nullptr;
  }

  auto selItem = ui.lwPaired->selectedItems().at(0);
  int pairId = selItem->data(Qt::UserRole).toInt();

  auto tnmt = Tournament::getActiveTournament();
  return tnmt->getPlayerMngr()->getPlayerPair_up(pairId);
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
  auto tnmt = Tournament::getActiveTournament();
  if ((unpairedPlayerId1 > 0) && (unpairedPlayerId2 > 0))
  {
    Player p1 = tnmt->getPlayerMngr()->getPlayer(unpairedPlayerId1);
    Player p2 = tnmt->getPlayerMngr()->getPlayer(unpairedPlayerId2);
    
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
  
  auto tnmt = Tournament::getActiveTournament();
  int id1 = selPlayers.at(0)->data(Qt::UserRole).toInt();
  int id2 = selPlayers.at(1)->data(Qt::UserRole).toInt();
  Category c = ui.catTableView->getSelectedCategory();
  Player p1 = tnmt->getPlayerMngr()->getPlayer(id1);
  Player p2 = tnmt->getPlayerMngr()->getPlayer(id2);
  if (c.canPairPlayers(p1, p2) != OK)
  {
    QMessageBox::warning(this, tr("These two players can't be paired for this category!"), tr("Pairing impossible"));
    updatePairs();
    return;
  }
  
  ERR e = tnmt->getCatMngr()->pairPlayers(c, p1, p2);
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
  auto tnmt = Tournament::getActiveTournament();
  CatMngr* cmngr = tnmt->getCatMngr();
  
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
  ui.catTableView->onAddCategory();
}

//----------------------------------------------------------------------------

void CatTabWidget::onMatchSystemChanged(int newIndex)
{
  // get the new match system
  int msId = ui.cbMatchSystem->itemData(newIndex, Qt::UserRole).toInt();
  MATCH_SYSTEM ms = static_cast<MATCH_SYSTEM>(msId);
  
  if (!(ui.catTableView->hasCategorySelected())) return;
  
  Category selectedCat = ui.catTableView->getSelectedCategory();
  auto tnmt = Tournament::getActiveTournament();
  tnmt->getCatMngr()->setMatchSystem(selectedCat, ms);
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
  ui.catTableView->onRunCategory();
}

//----------------------------------------------------------------------------

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

void CatTabWidget::onPlayerStateChanged(int playerId, int seqNum, const OBJ_STATE fromState, const OBJ_STATE toState)
{
  // is a category selected?
  if (!(ui.catTableView->hasCategorySelected())) return;

  // is the affected player in the currently selected category?
  // if not, there is nothing to do for us
  auto selectedCat = ui.catTableView->getSelectedCategory();
  auto tnmt = Tournament::getActiveTournament();
  Player pl = tnmt->getPlayerMngr()->getPlayer(playerId);
  if (!(selectedCat.hasPlayer(pl))) return;

  // okay, the changed player is obviously part of the currently selected,
  // displayed category

  // if a player changes from/to WAIT_FOR_REGISTRATION, we brute-force rebuild the list widgets
  // because we need to change the item label of the affected players for
  // adding or removing the paranthesis around the player names
  if (((fromState == STAT_PL_IDLE) && (toState == STAT_PL_WAIT_FOR_REGISTRATION)) ||
      ((fromState == STAT_PL_WAIT_FOR_REGISTRATION) && (toState == STAT_PL_IDLE)))
  {
    updatePairs();
  }
}

//----------------------------------------------------------------------------

void CatTabWidget::onRemovePlayerFromCat()
{
  auto selPlayer = lwUnpaired_getSelectedPlayer();
  if (selPlayer == nullptr) return;

  // the following call must always succeed... if no category
  // was selected there was no player to trigger the context menu on
  auto selCat = ui.catTableView->getSelectedCategory();

  cmdRemovePlayerFromCategory cmd{this, *selPlayer, selCat};
  cmd.exec();
}

//----------------------------------------------------------------------------

void CatTabWidget::onBulkRemovePlayersFromCat()
{
  if (!(ui.catTableView->hasCategorySelected()))
  {
    return;
  }

  cmdBulkRemovePlayersFromCategory cmd{this, ui.catTableView->getSelectedCategory()};
  cmd.exec();
}

//----------------------------------------------------------------------------

void CatTabWidget::onAddPlayerToCat()
{
  if (!(ui.catTableView->hasCategorySelected()))
  {
    return;
  }

  cmdBulkAddPlayerToCategory cmd{this, ui.catTableView->getSelectedCategory()};
  cmd.exec();
}

//----------------------------------------------------------------------------

void CatTabWidget::onUnpairedContextMenuRequested(const QPoint& pos)
{
  // map from scroll area coordinates to global widget coordinates
  QPoint globalPos = ui.lwUnpaired->viewport()->mapToGlobal(pos);

  // determine if there is an item under the mouse
  auto selItem = ui.lwUnpaired->itemAt(pos);
  upPlayer selPlayer;
  OBJ_STATE plStat = STAT_CO_DISABLED;   // arbitrary, non player-related, dummy default
  if (selItem != nullptr)
  {
    // clear old selection and select item under the mouse
    ui.lwUnpaired->clearSelection();
    selItem->setSelected(true);

    selPlayer = lwUnpaired_getSelectedPlayer();
    plStat = selPlayer->getState();
  }

  bool isPlayerClicked = (selPlayer != nullptr);

  bool hasCatSelected = ui.catTableView->hasCategorySelected();
  bool canAddPlayers = false;
  if (hasCatSelected)
  {
    Category cat = ui.catTableView->getSelectedCategory();
    canAddPlayers = cat.canAddPlayers();
  }

  // rebuild dynamic submenus
  MenuGenerator::allCategories(listOfCats_CopyPlayerSubmenu.get());
  MenuGenerator::allCategories(listOfCats_MovePlayerSubmenu.get());

  // enable / disable selection-specific actions
  auto tnmt = Tournament::getActiveTournament();
  actRemovePlayer->setEnabled(isPlayerClicked);
  actRegister->setEnabled(plStat == STAT_PL_WAIT_FOR_REGISTRATION);
  actUnregister->setEnabled(plStat == STAT_PL_IDLE);
  listOfCats_CopyPlayerSubmenu->setEnabled(isPlayerClicked);
  listOfCats_MovePlayerSubmenu->setEnabled(isPlayerClicked);
  actCreateNewPlayerInCat->setEnabled(hasCatSelected && canAddPlayers);
  actAddPlayer->setEnabled(canAddPlayers);
  actImportPlayerToCat->setEnabled(canAddPlayers && tnmt->getPlayerMngr()->hasExternalPlayerDatabaseOpen());
  actBulkRemovePlayers->setEnabled(hasCatSelected);

  // show the context menu
  QAction* selectedItem = lwUnpairedContextMenu->exec(globalPos);

  // check if one of the dynamically generated submenus was selected
  //
  // Actions belonging to these menus have a non-empty user data
  if ((selectedItem == nullptr) || (selectedItem->data().isNull()))
  {
    // no selection or manually created action, noting more to do here
    return;
  }

  //
  // manually trigger the reaction associated with the menu item
  //
  if (MenuGenerator::isActionInMenu(listOfCats_CopyPlayerSubmenu.get(), selectedItem))
  {
    onCopyOrMovePlayer(selectedItem->data().toInt(), false);
  }
  if (MenuGenerator::isActionInMenu(listOfCats_MovePlayerSubmenu.get(), selectedItem))
  {
    onCopyOrMovePlayer(selectedItem->data().toInt(), true);
  }
}

//----------------------------------------------------------------------------

void CatTabWidget::onPairedContextMenuRequested(const QPoint& pos)
{
  // map from scroll area coordinates to global widget coordinates
  QPoint globalPos = ui.lwPaired->viewport()->mapToGlobal(pos);

  // determine if there is an item under the mouse
  auto selItem = ui.lwPaired->itemAt(pos);
  upPlayerPair selPair;
  if (selItem != nullptr)
  {
    // clear old selection and select item under the mouse
    ui.lwPaired->clearSelection();
    selItem->setSelected(true);

    int selPairId = selItem->data(Qt::UserRole).toInt();
    auto tnmt = Tournament::getActiveTournament();
    selPair = tnmt->getPlayerMngr()->getPlayerPair_up(selPairId);
  }

  // we have no actions that are useful without a selected
  // pair. So if nothing is selected, we can quit here
  if (selPair == nullptr)
  {
    return;
  }

  // rebuild dynamic submenus
  MenuGenerator::allCategories(listOfCats_CopyPairSubmenu.get());
  MenuGenerator::allCategories(listOfCats_MovePairSubmenu.get());

  // show the context menu
  QAction* selectedItem = lwPairsContextMenu->exec(globalPos);

  // check if one of the dynamically generated submenus was selected
  //
  // Actions belonging to these menus have a non-empty user data
  if ((selectedItem == nullptr) || (selectedItem->data().isNull()))
  {
    // no selection or manually created action, noting more to do here
    return;
  }

  //
  // manually trigger the reaction associated with the menu item
  //
  if (MenuGenerator::isActionInMenu(listOfCats_CopyPairSubmenu.get(), selectedItem))
  {
    onCopyOrMovePair(*selPair, selectedItem->data().toInt(), false);
  }
  if (MenuGenerator::isActionInMenu(listOfCats_MovePairSubmenu.get(), selectedItem))
  {
    onCopyOrMovePair(*selPair, selectedItem->data().toInt(), true);
  }
}

//----------------------------------------------------------------------------

void CatTabWidget::onRegisterPlayer()
{
  auto selPlayer = lwUnpaired_getSelectedPlayer();
  if (selPlayer == nullptr) return;

  cmdRegisterPlayer cmd{this, *selPlayer};
  cmd.exec();
}

//----------------------------------------------------------------------------

void CatTabWidget::onUnregisterPlayer()
{
  auto selPlayer = lwUnpaired_getSelectedPlayer();
  if (selPlayer == nullptr) return;

  cmdUnregisterPlayer cmd{this, *selPlayer};
  cmd.exec();
}

//----------------------------------------------------------------------------

void CatTabWidget::onCopyOrMovePlayer(int targetCatId, bool isMove)
{
  auto selPlayer = lwUnpaired_getSelectedPlayer();
  if (selPlayer == nullptr) return;

  if (!(ui.catTableView->hasCategorySelected()))
  {
    return;
  }

  auto tnmt = Tournament::getActiveTournament();
  auto targetCat = tnmt->getCatMngr()->getCategoryById(targetCatId);

  cmdMoveOrCopyPlayerToCategory cmd{this, *selPlayer,
        ui.catTableView->getSelectedCategory(),
        targetCat, isMove};

  cmd.exec();
}

//----------------------------------------------------------------------------

void CatTabWidget::onCopyOrMovePair(const PlayerPair& selPair, int targetCatId, bool isMove)
{
  if (!(ui.catTableView->hasCategorySelected()))
  {
    return;
  }

  auto tnmt = Tournament::getActiveTournament();
  auto targetCat = tnmt->getCatMngr()->getCategoryById(targetCatId);

  cmdMoveOrCopyPairToCategory cmd{this, selPair,
        ui.catTableView->getSelectedCategory(),
        targetCat, isMove};

  cmd.exec();
}

//----------------------------------------------------------------------------

void CatTabWidget::onCreatePlayer()
{
  ui.catTableView->onCreatePlayer();
}

void CatTabWidget::onImportPlayer()
{
  ui.catTableView->onImportPlayer();
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

