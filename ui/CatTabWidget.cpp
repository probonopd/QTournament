/*
 *    This is QTournament, a badminton tournament management program.
 *    Copyright (C) 2014 - 2017  Volker Knollmann
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
#include <memory>

#include <QMessageBox>

#include "CatTabWidget.h"
#include "TournamentDataDefs.h"
#include "dlgGroupAssignment.h"
#include "DlgSeedingEditor.h"
#include "PlayerMngr.h"
#include "CatMngr.h"
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
#include "CentralSignalEmitter.h"

using namespace std;
using namespace QTournament;

CatTabWidget::CatTabWidget()
  :db(nullptr)
{
  ui.setupUi(this);
  
  // connect to the view's signal that tells us that the model has changed
  connect(ui.catTableView, SIGNAL(catModelChanged()), this, SLOT(onCatModelChanged()));
  
  // connect the selection change signal of the two list widgets
  connect(ui.lwUnpaired, &QListWidget::itemSelectionChanged, this, &CatTabWidget::onUnpairedPlayersSelectionChanged);
  connect(ui.lwPaired, &QListWidget::itemSelectionChanged, this, &CatTabWidget::onPairedPlayersSelectionChanged);
  
  // connect to the change signal of the group config widget
  connect(ui.grpCfgWidget, &GroupConfigWidget::groupConfigChanged, this, &CatTabWidget::onGroupConfigChanged);

  // hide unused settings groups
  ui.gbGroups->hide();
  
  // initialize the entries in the drop box
  ui.cbMatchSystem->addItem(tr("Swiss ladder"), static_cast<int>(MatchSystem::SwissLadder));
  ui.cbMatchSystem->addItem(tr("Group matches with KO rounds"), static_cast<int>(MatchSystem::GroupsWithKO));
  //ui.cbMatchSystem->addItem(tr("Random matches (for fun tournaments)"), static_cast<int>(MatchSystem::Randomize));
  ui.cbMatchSystem->addItem(tr("Tree-like ranking system"), static_cast<int>(MatchSystem::Ranking));
  ui.cbMatchSystem->addItem(tr("Single Elimination"), static_cast<int>(MatchSystem::SingleElim));
  ui.cbMatchSystem->addItem(tr("Round robin matches"), static_cast<int>(MatchSystem::RoundRobin));

  // setup the context menu(s)
  initContextMenu();

  // get information about changed player names or category assignments
  CentralSignalEmitter* cse = CentralSignalEmitter::getInstance();
  connect(cse, SIGNAL(playerAddedToCategory(Player,Category)), this, SLOT(onPlayerAddedToCategory(Player,Category)));
  connect(cse, SIGNAL(playerRemovedFromCategory(Player,Category)), this, SLOT(onPlayerRemovedFromCategory(Player,Category)));
  connect(cse, SIGNAL(categoryStatusChanged(Category,ObjState,ObjState)), this, SLOT(onCatStateChanged(Category,ObjState,ObjState)));
  connect(cse, SIGNAL(playerRenamed(Player)), this, SLOT(onPlayerRenamed(Player)));
  connect(cse, SIGNAL(playerStatusChanged(int,int,ObjState,ObjState)), this, SLOT(onPlayerStateChanged(int,int,ObjState,ObjState)));
  connect(cse, SIGNAL(categoryRemovedFromTournament(int,int)), this, SLOT(onCategoryRemoved()));

  // tell the list widgets to emit signals if a context menu is requested
  ui.lwUnpaired->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(ui.lwUnpaired, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onUnpairedContextMenuRequested(QPoint)));
  ui.lwPaired->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(ui.lwPaired, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onPairedContextMenuRequested(QPoint)));

  // create a delegate for painting indexed, sorted player names
  playerItemDelegate = make_unique<CatTabPlayerItemDelegate>(this, true);
  ui.lwUnpaired->setItemDelegate(playerItemDelegate.get());
  ui.lwPaired->setItemDelegate(playerItemDelegate.get());
}

//----------------------------------------------------------------------------
    
CatTabWidget::~CatTabWidget()
{
}

//----------------------------------------------------------------------------

void CatTabWidget::setDatabase(TournamentDB* _db)
{
  db = _db;

  ui.catTableView->setDatabase(db);
  ui.grpCfgWidget->setDatabase(db);

  setEnabled(db != nullptr);
}

//----------------------------------------------------------------------------

void CatTabWidget::onCatModelChanged()
{
  // react on selection changes in the category table
  connect(ui.catTableView->selectionModel(),
	  SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
	  SLOT(onCatSelectionChanged(const QItemSelection&, const QItemSelection&)));
  
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
    ui.gbRoundRobin->hide();
    return;
  }
  
  Category selectedCat = ui.catTableView->getSelectedCategory();
  
  //
  // if we made it to this point, we can be sure to have a valid category selected
  //
  Sex sex = selectedCat.getSex();
  MatchType mt = selectedCat.getMatchType();
  bool isEditEnabled = (selectedCat.getState() == ObjState::CAT_Config);

  ui.gbGeneric->setEnabled(isEditEnabled);
  
  // update the list box showing the match system
  int matchSysId = static_cast<int>(selectedCat.getMatchSystem());
  ui.cbMatchSystem->setCurrentIndex(ui.cbMatchSystem->findData(matchSysId, Qt::UserRole));
  
  // activate the applicable group with the special settings
  MatchSystem ms = selectedCat.getMatchSystem();
  if (ms == MatchSystem::GroupsWithKO)
  {
    ui.gbGroups->show();
    ui.gbRandom->hide();
    ui.gbRoundRobin->hide();
    
    // read the current group settings from the database and
    // copy them to the widget
    KO_Config cfg = KO_Config(selectedCat.getParameter_string(CatParameter::GroupConfig));
    ui.grpCfgWidget->applyConfig(cfg);
  }
  else if (ms == MatchSystem::Randomize)
  {
    ui.gbGroups->hide();
    ui.gbRandom->show();
    ui.gbRoundRobin->hide();
  }
  else if (ms == MatchSystem::RoundRobin)
  {
    ui.gbGroups->hide();
    ui.gbRandom->hide();
    ui.gbRoundRobin->show();

    // read the number of iterations that are
    // currently configured for this category
    int it = selectedCat.getParameter_int(CatParameter::RoundRobinIterations);
    ui.cbRoundRobinTwoIterations->setChecked(it > 1);
  }
  else
  {
    ui.gbGroups->hide();
    ui.gbRandom->hide();
    ui.gbRoundRobin->hide();
  }
  
  // update the match type
  ui.rbSingles->setChecked(mt == MatchType::Singles);
  ui.rbDoubles->setChecked(mt == MatchType::Doubles);
  ui.rbMixed->setChecked(mt == MatchType::Mixed);
  
  // disable radio buttons for male / female for mixed categories
  ui.rbMen->setEnabled(mt != MatchType::Mixed);
  ui.rbLadies->setEnabled(mt != MatchType::Mixed);
  if (mt == MatchType::Mixed)
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
  if (sex == Sex::DontCare)
  {
    ui.rbgSex->setExclusive(false);
    ui.rbMen->setChecked(false);
    ui.rbLadies->setChecked(false);
    ui.rbgSex->setExclusive(true);
  } else {
    ui.rbMen->setChecked((sex == Sex::M) && (mt != MatchType::Mixed));
    ui.rbLadies->setChecked((sex == Sex::F) && (mt != MatchType::Mixed));
  }
  ui.cbDontCare->setChecked(sex == Sex::DontCare);
  
  // the "accept draw" checkbox
  bool allowDraw = selectedCat.getParameter(CatParameter::AllowDraw).toBool();
  ui.cbDraw->setChecked(allowDraw);

  // the checkbox must be disabled under two conditions:
  // 1) we are in a match system with elimination rounds
  // 2) we have round robins resulting directly in finals
  //
  // In the latter case, we need the second player for the
  // match for 3rd place
  bool enableDrawCheckbox = true;
  if ((ms == MatchSystem::SingleElim) || (ms == MatchSystem::Ranking))
  {
    enableDrawCheckbox = false;
  }
  if (ms == MatchSystem::GroupsWithKO)
  {
    KO_Config cfg = KO_Config(selectedCat.getParameter_string(CatParameter::GroupConfig));
    if (cfg.getStartLevel() == KO_Start::Final) enableDrawCheckbox = false;
  }
  ui.cbDraw->setEnabled(enableDrawCheckbox);

  // FIX ME / TO DO: playing "draw" is not yet implemented, so for now
  // we hardwire the checkbox to disabled.
  ui.cbDraw->setEnabled(false);
  
  // the score spinboxes
  int drawScore = selectedCat.getParameter(CatParameter::DrawScore).toInt();
  int winScore = selectedCat.getParameter(CatParameter::WinScore).toInt();
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
  if (mt == MatchType::Singles)
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
  ui.gbRoundRobin->setEnabled(isEditEnabled);

  // change the label of the "run" button and enable or
  // disable it
  ObjState catState = selectedCat.getState();
  if (catState == ObjState::CAT_WaitForIntermediateSeeding)
  {
    ui.btnRunCategory->setText("Continue");
  } else {
    ui.btnRunCategory->setText("Run");
  }
  bool enableRunButton = ((catState == ObjState::CAT_Config) || (catState == ObjState::CAT_WaitForIntermediateSeeding));
  ui.btnRunCategory->setEnabled(enableRunButton);
}

//----------------------------------------------------------------------------

void CatTabWidget::onCbDrawChanged(bool newState)
{
  Category c = ui.catTableView->getSelectedCategory();
  c.setParameter(CatParameter::AllowDraw, newState);
  updateControls();
}

//----------------------------------------------------------------------------

void CatTabWidget::onWinScoreChanged(int newVal)
{
  Category c = ui.catTableView->getSelectedCategory();
  c.setParameter(CatParameter::WinScore, newVal);
  updateControls();
}

//----------------------------------------------------------------------------
    
void CatTabWidget::onDrawScoreChanged(int newVal)
{
  Category c = ui.catTableView->getSelectedCategory();
  c.setParameter(CatParameter::DrawScore, newVal);
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
  
  for (const auto& pp : pairList)
  {    
    if (pp.hasPlayer2())
    {
      QListWidgetItem* item = new QListWidgetItem(pp.getDisplayName(0, true));
      item->setData(Qt::UserRole, pp.getPairId());
      ui.lwPaired->addItem(item);
    } else {
      QListWidgetItem* item = new QListWidgetItem(pp.getDisplayName(0, true));
      item->setData(Qt::UserRole, pp.getPlayer1().getId());
      ui.lwUnpaired->addItem(item);
    }
  }
  
  // update the "required players" label in the GroupConfigWidget
  if (selCat.getMatchType() == MatchType::Singles)
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
  lwUnpairedContextMenu = make_unique<QMenu>();
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
  lwPairsContextMenu = make_unique<QMenu>();
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

std::optional<Player> CatTabWidget::lwUnpaired_getSelectedPlayer() const
{
  // we can only handle exactly one selected item
  if (ui.lwUnpaired->selectedItems().length() != 1)
  {
    return {};
  }

  auto selItem = ui.lwUnpaired->selectedItems().at(0);
  int playerId = selItem->data(Qt::UserRole).toInt();

  PlayerMngr pm{*db};
  return pm.getPlayer2(playerId);
}

//----------------------------------------------------------------------------

std::optional<QTournament::PlayerPair> CatTabWidget::lwPaired_getSelectedPair() const
{
  // we can only handle exactly one selected item
  if (ui.lwPaired->selectedItems().length() != 1)
  {
    return {};
  }

  auto selItem = ui.lwPaired->selectedItems().at(0);
  int pairId = selItem->data(Qt::UserRole).toInt();

  PlayerMngr pm{*db};
  return pm.getPlayerPair(pairId);
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
  PlayerMngr pm{*db};
  if ((unpairedPlayerId1 > 0) && (unpairedPlayerId2 > 0))
  {
    Player p1 = pm.getPlayer(unpairedPlayerId1);
    Player p2 = pm.getPlayer(unpairedPlayerId2);
    
    Category c = ui.catTableView->getSelectedCategory();
    canPair = (c.canPairPlayers(p1, p2) == Error::OK);
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
  
  PlayerMngr pm{*db};
  int id1 = selPlayers.at(0)->data(Qt::UserRole).toInt();
  int id2 = selPlayers.at(1)->data(Qt::UserRole).toInt();
  Category c = ui.catTableView->getSelectedCategory();
  Player p1 = pm.getPlayer(id1);
  Player p2 = pm.getPlayer(id2);
  if (c.canPairPlayers(p1, p2) != Error::OK)
  {
    QMessageBox::warning(this, tr("These two players can't be paired for this category!"), tr("Pairing impossible"));
    updatePairs();
    return;
  }
  
  CatMngr cm{*db};
  Error e = cm.pairPlayers(c, p1, p2);
  if (e != Error::OK)
  {
    QMessageBox::warning(
          this,
          tr("Something went wrong during pairing. This shouldn't happen. For the records: Error code = ") + QString::number(static_cast<int>(e)),
          tr("Pairing impossible")
          );
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
  CatMngr cmngr{*db};
  
  for (int i=0; i < selPairs.count(); i++)
  {
    int pairId = selPairs.at(i)->data(Qt::UserRole).toInt();
    Error e = cmngr.splitPlayers(c, pairId);
    if (e != Error::OK)
    {
      QMessageBox::warning(
            this,
            tr("Something went wrong during splitting. This shouldn't happen. For the records: Error code = ") + QString::number(static_cast<int>(e)),
            tr("Splitting impossible")
            );
    }
  }
  
  updatePairs();
}

//----------------------------------------------------------------------------

void CatTabWidget::onMatchTypeButtonClicked(int btn)
{
  Category selCat = ui.catTableView->getSelectedCategory();
  
  MatchType oldType = selCat.getMatchType();
  
  MatchType newType = MatchType::Singles;
  if (ui.rbDoubles->isChecked()) newType = MatchType::Doubles;
  if (ui.rbMixed->isChecked()) newType = MatchType::Mixed;
  
  // do we actually have a change?
  if (oldType == newType)
  {
    return;  // nope, no action required
  }
  
  // change the type
  Error e = selCat.setMatchType(newType);
  if (e != Error::OK)
  {
    QMessageBox::warning(
          this,
          tr("Error"),
          tr("Could change match type. Error number = ") + QString::number(static_cast<int>(e))
          );
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
  Sex newSex = (ui.rbLadies->isChecked()) ? Sex::F : Sex::M;
  
  // in any case, we can de-select the "don't care" box
  ui.cbDontCare->setChecked(false);
  
  // actually change the sex
  Category selCat = ui.catTableView->getSelectedCategory();
  Error e = selCat.setSex(newSex);
  if (e != Error::OK)
  {
    QMessageBox::warning(
          this,
          tr("Error"),
          tr("Could change sex. Error number = ") + QString::number(static_cast<int>(e))
          );
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
  if (!newState && (selCat.getMatchType() != MatchType::Mixed))
  {
    QString msg = tr("Please deactivate 'Don't care' by selecting a specific sex!");
    QMessageBox::information(this, tr("Change category sex"), msg);
    
    // re-check the checkbox
    ui.cbDontCare->setChecked(true);
    
    return;
  }
  
  Sex newSex = Sex::DontCare;
  
  // if we are in a mixed category, allow simple de-activation of "Don't care"
  if (!newState && (selCat.getMatchType() == MatchType::Mixed))
  {
    // set a dummy default value that is not "Don't care"
    newSex = Sex::M;
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
  Error e = selCat.setSex(newSex);
  if (e != Error::OK)
  {
    QMessageBox::warning(
          this,
          tr("Error"),
          tr("Could change sex. Error number = ") + QString::number(static_cast<int>(e))
          );
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
  MatchSystem ms = static_cast<MatchSystem>(msId);
  
  if (!(ui.catTableView->hasCategorySelected())) return;
  
  Category selectedCat = ui.catTableView->getSelectedCategory();
  CatMngr cm{*db};
  cm.setMatchSystem(selectedCat, ms);
  updateControls();
}

//----------------------------------------------------------------------------

void CatTabWidget::onGroupConfigChanged(const KO_Config& newCfg)
{
  if (!(ui.catTableView->hasCategorySelected())) return;
  
  Category selectedCat = ui.catTableView->getSelectedCategory();
  selectedCat.setParameter(CatParameter::GroupConfig, newCfg.toString());
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
void CatTabWidget::onCatStateChanged(const Category &c, const ObjState fromState, const ObjState toState)
{
  updateControls();
}

//----------------------------------------------------------------------------

void CatTabWidget::onPlayerStateChanged(int playerId, int seqNum, const ObjState fromState, const ObjState toState)
{
  // is a category selected?
  if (!(ui.catTableView->hasCategorySelected())) return;

  // is the affected player in the currently selected category?
  // if not, there is nothing to do for us
  auto selectedCat = ui.catTableView->getSelectedCategory();
  PlayerMngr pm{*db};
  Player pl = pm.getPlayer(playerId);
  if (!(selectedCat.hasPlayer(pl))) return;

  // okay, the changed player is obviously part of the currently selected,
  // displayed category

  // if a player changes from/to WAIT_FOR_REGISTRATION, we brute-force rebuild the list widgets
  // because we need to change the item label of the affected players for
  // adding or removing the paranthesis around the player names
  if (((fromState == ObjState::PL_Idle) && (toState == ObjState::PL_WaitForRegistration)) ||
      ((fromState == ObjState::PL_WaitForRegistration) && (toState == ObjState::PL_Idle)))
  {
    updatePairs();
  }
}

//----------------------------------------------------------------------------

void CatTabWidget::onRemovePlayerFromCat()
{
  auto selPlayer = lwUnpaired_getSelectedPlayer();
  if (!selPlayer) return;

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
  ObjState plStat = ObjState::CO_Disabled;   // arbitrary, non player-related, dummy default
  std::optional<Player> selPlayer;
  if (selItem != nullptr)
  {
    // clear old selection and select item under the mouse
    ui.lwUnpaired->clearSelection();
    selItem->setSelected(true);

    selPlayer = lwUnpaired_getSelectedPlayer();
    plStat = selPlayer->getState();
  }

  bool isPlayerClicked = (selPlayer.has_value());

  bool hasCatSelected = ui.catTableView->hasCategorySelected();
  bool canAddPlayers = false;
  if (hasCatSelected)
  {
    Category cat = ui.catTableView->getSelectedCategory();
    canAddPlayers = cat.canAddPlayers();
  }

  // rebuild dynamic submenus
  MenuGenerator::allCategories(*db, listOfCats_CopyPlayerSubmenu.get());
  MenuGenerator::allCategories(*db, listOfCats_MovePlayerSubmenu.get());

  // enable / disable selection-specific actions
  PlayerMngr pm{*db};
  actRemovePlayer->setEnabled(isPlayerClicked);
  actRegister->setEnabled(plStat == ObjState::PL_WaitForRegistration);
  actUnregister->setEnabled(plStat == ObjState::PL_Idle);
  listOfCats_CopyPlayerSubmenu->setEnabled(isPlayerClicked);
  listOfCats_MovePlayerSubmenu->setEnabled(isPlayerClicked);
  actCreateNewPlayerInCat->setEnabled(hasCatSelected && canAddPlayers);
  actAddPlayer->setEnabled(canAddPlayers);
  actImportPlayerToCat->setEnabled(canAddPlayers && pm.hasExternalPlayerDatabaseAvailable());
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
  std::optional<PlayerPair> selPair;
  if (selItem != nullptr)
  {
    // clear old selection and select item under the mouse
    ui.lwPaired->clearSelection();
    selItem->setSelected(true);

    int selPairId = selItem->data(Qt::UserRole).toInt();
    PlayerMngr pm{*db};
    selPair = pm.getPlayerPair(selPairId);
  }

  // we have no actions that are useful without a selected
  // pair. So if nothing is selected, we can quit here
  if (!selPair)
  {
    return;
  }

  // rebuild dynamic submenus containing all applicable categories
  //
  // Only doubles or mixed categories are suitable targets for
  // copying or moving player pairs
  CatMngr cm{*db};
  CategoryList targetCats;
  auto selCat = ui.catTableView->getSelectedCategory();  // this MUST succeed; a category MUST be selected at his point
  for (const Category& cat : cm.getAllCategories())
  {
    // skip singles
    if (cat.getMatchType() == MatchType::Singles) continue;

    // skip the selected category because it doesn't make
    // sense to copy/move if source and target are identical
    if (cat == selCat) continue;

    targetCats.push_back(cat);
  }
  if (targetCats.empty())
  {
    // deactivate the copy/move menus completely if no
    // suitable target categories are available
    listOfCats_CopyPairSubmenu->setEnabled(false);
    listOfCats_MovePairSubmenu->setEnabled(false);
  } else {
    listOfCats_CopyPairSubmenu->setEnabled(true);
    listOfCats_MovePairSubmenu->setEnabled(true);
    MenuGenerator::fromCategoryList(*db, targetCats, listOfCats_CopyPairSubmenu.get());
    MenuGenerator::fromCategoryList(*db, targetCats, listOfCats_MovePairSubmenu.get());
  }

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
  if (!selPlayer) return;

  cmdRegisterPlayer cmd{this, *selPlayer};
  cmd.exec();
}

//----------------------------------------------------------------------------

void CatTabWidget::onUnregisterPlayer()
{
  auto selPlayer = lwUnpaired_getSelectedPlayer();
  if (!selPlayer) return;

  cmdUnregisterPlayer cmd{this, *selPlayer};
  cmd.exec();
}

//----------------------------------------------------------------------------

void CatTabWidget::onCopyOrMovePlayer(int targetCatId, bool isMove)
{
  auto selPlayer = lwUnpaired_getSelectedPlayer();
  if (!selPlayer) return;

  if (!(ui.catTableView->hasCategorySelected()))
  {
    return;
  }

  CatMngr cm{*db};
  auto targetCat = cm.getCategoryById(targetCatId);

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

  CatMngr cm{*db};
  auto targetCat = cm.getCategoryById(targetCatId);

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

//----------------------------------------------------------------------------

void CatTabWidget::onImportPlayer()
{
  ui.catTableView->onImportPlayer();
}

//----------------------------------------------------------------------------

void CatTabWidget::onCategoryRemoved()
{
  updateControls();
  updatePairs();
}

//----------------------------------------------------------------------------

void CatTabWidget::onTwoIterationsChanged()
{
  if (!(ui.catTableView->hasCategorySelected()))
  {
    return;
  }

  int it = (ui.cbRoundRobinTwoIterations->isChecked()) ? 2 : 1;
  Category selCat = ui.catTableView->getSelectedCategory();
  selCat.setParameter(CatParameter::RoundRobinIterations, it);
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

