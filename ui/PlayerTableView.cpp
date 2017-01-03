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

#include <vector>

#include <QMessageBox>
#include <QScrollBar>

#include "PlayerTableView.h"
#include "MainFrame.h"
#include "dlgEditPlayer.h"
#include "ui/commonCommands/cmdRegisterPlayer.h"
#include "ui/commonCommands/cmdUnregisterPlayer.h"
#include "ui/commonCommands/cmdImportSinglePlayerFromExternalDatabase.h"
#include "ui/commonCommands/cmdExportPlayerToExternalDatabase.h"
#include "ui/commonCommands/cmdCreatePlayerFromDialog.h"
#include "PlayerMngr.h"
#include "TeamMngr.h"
#include "CatMngr.h"
#include "Match.h"
#include "MatchTimePredictor.h"

PlayerTableView::PlayerTableView(QWidget* parent)
:QTableView(parent), db(nullptr), curDataModel(nullptr)
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
  sortByColumn(PlayerTableModel::COL_NAME, Qt::AscendingOrder);

  // initiate the model(s) as empty
  setDatabase(nullptr);

  // handle context menu requests
  setContextMenuPolicy(Qt::CustomContextMenu);
  connect(this, SIGNAL(customContextMenuRequested(const QPoint&)),
          this, SLOT(onContextMenuRequested(const QPoint&)));

  // handle double clicks on a column header
  connect(horizontalHeader(), SIGNAL(sectionDoubleClicked(int)), this, SLOT(onSectionHeaderDoubleClicked()));

  // setup the context menu
  initContextMenu();

}

//----------------------------------------------------------------------------
    
PlayerTableView::~PlayerTableView()
{
  delete emptyModel;
  delete sortedModel;
  if (curDataModel != nullptr) delete curDataModel;
  if (defaultDelegate != nullptr) delete defaultDelegate;
}

//----------------------------------------------------------------------------

unique_ptr<Player> PlayerTableView::getSelectedPlayer() const
{
  // make sure we have non-empty model
  auto mod = model();
  if (mod == nullptr) return nullptr;
  if (mod->rowCount() == 0) return nullptr;

  // make sure we have one item selected
  QModelIndexList indexes = selectionModel()->selection().indexes();
  if (indexes.count() == 0)
  {
    return nullptr;
  }

  // return the selected item
  PlayerMngr pm{db};
  int selectedSourceRow = sortedModel->mapToSource(indexes.at(0)).row();
  return pm.getPlayerBySeqNum(selectedSourceRow);
}

//----------------------------------------------------------------------------

void PlayerTableView::resizeEvent(QResizeEvent* event)
{
  // call parent function
  QTableView::resizeEvent(event);

  autosizeColumns();

  // finish event processing
  event->accept();
}

//----------------------------------------------------------------------------

void PlayerTableView::autosizeColumns()
{
  // distribute the available space over the columns but
  // set a maximum to prevent too wide name fields
  int widthAvail = width();
  if ((verticalScrollBar() != nullptr) && (verticalScrollBar()->isVisible()))
  {
    widthAvail -= verticalScrollBar()->width();
  }
  double unitWidth = widthAvail / (1.0 * TOTAL_WIDTH_UNITS);
  bool isWidthExceeded = (widthAvail >= MAX_TOTAL_COL_WIDTH);
  int nameColWidth = isWidthExceeded ? MAX_NAME_COL_WIDTH : unitWidth * REL_NAME_COL_WIDTH;
  int sexColWidth = isWidthExceeded ? MAX_SEX_COL_WIDTH : unitWidth * REL_SEX_COL_WIDTH;
  int teamColWidth = isWidthExceeded ? MAX_TEAM_COL_WIDTH : unitWidth * REL_TEAM_COL_WIDTH;
  int catColWidth = isWidthExceeded ? MAX_CAT_COL_WIDTH : unitWidth * REL_CAT_COL_WIDTH;

  // set the column widths
  setColumnWidth(0, nameColWidth);
  setColumnWidth(1, sexColWidth);
  setColumnWidth(2, teamColWidth);
  setColumnWidth(3, catColWidth);

  // set the width of the fill column to what's left
  int fillWidth = widthAvail - (nameColWidth + sexColWidth + teamColWidth + catColWidth);
  if (fillWidth > 0)
  {
    setColumnHidden(PlayerTableModel::FILL_COL, false);
    setColumnWidth(PlayerTableModel::FILL_COL, fillWidth);
  } else {
    setColumnHidden(PlayerTableModel::FILL_COL, true);
  }
}

//----------------------------------------------------------------------------

void PlayerTableView::setDatabase(TournamentDB* _db)
{
  // set the new data model
  PlayerTableModel* newDataModel = nullptr;
  if (_db != nullptr)
  {
    newDataModel = new PlayerTableModel(_db);
    sortedModel->setSourceModel(newDataModel);

    // define a delegate for drawing the player items
    playerItemDelegate = make_unique<PlayerItemDelegate>(_db, this);
    playerItemDelegate->setProxy(sortedModel);
    setItemDelegate(playerItemDelegate.get());

    // resize columns and rows to content once (we do not want permanent automatic resizing)
    horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
    verticalHeader()->resizeSections(QHeaderView::ResizeToContents);

  } else {
    sortedModel->setSourceModel(emptyModel);
    setItemDelegate(defaultDelegate);
  }

  // delete the old data model, if it was a
  // CategoryTableModel instance
  if (curDataModel != nullptr)
  {
    delete curDataModel;
  }

  // store the new CategoryTableModel instance, if any
  curDataModel = newDataModel;

  // update the database pointer and set the widget's enabled state
  db = _db;
  setEnabled(db != nullptr);

  // trigger a resizing of the displayed columns
  autosizeColumns();
}

//----------------------------------------------------------------------------
    
QModelIndex PlayerTableView::mapToSource(const QModelIndex &proxyIndex)
{
  return sortedModel->mapToSource(proxyIndex);
}

//----------------------------------------------------------------------------

void PlayerTableView::onContextMenuRequested(const QPoint& pos)
{
  // map from scroll area coordinates to global widget coordinates
  QPoint globalPos = viewport()->mapToGlobal(pos);

  // resolve the click coordinates to the table row
  int clickedRow = rowAt(pos.y());
  int clickedCol = columnAt(pos.x());
  bool isRowClicked = ((clickedRow >= 0) && (clickedCol >= 0));

  // get the player status for enabling / disabling state-dependent actions
  auto selPlayer = getSelectedPlayer();
  bool isPlayerClicked = (selPlayer != nullptr);
  OBJ_STATE plStat = STAT_CO_DISABLED;   // an arbitrary, dummy default value that has nothing to do with players
  if (isPlayerClicked) plStat = selPlayer->getState();

  // if no player is clicked, we may only add a player.
  // in other cases, we may also try to edit or delete
  // players
  actAddPlayer->setEnabled(true);   // always possible
  actEditPlayer->setEnabled(isPlayerClicked & isRowClicked);
  actShowNextMatchesForPlayer->setEnabled(isPlayerClicked & isRowClicked);
  //actShowNextMatchesForPlayer->setEnabled(false);  // not yet implemented
  actRemovePlayer->setEnabled(isPlayerClicked & isRowClicked);
  actRegister->setEnabled(isPlayerClicked & (plStat == STAT_PL_WAIT_FOR_REGISTRATION) & isRowClicked);
  actUnregister->setEnabled(isPlayerClicked & (plStat == STAT_PL_IDLE) & isRowClicked);

  PlayerMngr pm{db};
  bool hasExtDb = pm.hasExternalPlayerDatabaseAvailable();
  actImportFromExtDatabase->setEnabled(hasExtDb);
  actSyncAllToExtDatabase->setEnabled(hasExtDb);
  actExportToExtDatabase->setEnabled(isPlayerClicked && hasExtDb);

  // show the context menu
  contextMenu->exec(globalPos);
}

//----------------------------------------------------------------------------

void PlayerTableView::onSectionHeaderDoubleClicked()
{
  autosizeColumns();
}

//----------------------------------------------------------------------------

void PlayerTableView::onAddPlayerTriggered()
{
  DlgEditPlayer dlg(db, this);

  dlg.setModal(true);
  cmdCreatePlayerFromDialog cmd{db, this, &dlg};
  cmd.exec();
}

//----------------------------------------------------------------------------

void PlayerTableView::onEditPlayerTriggered()
{
  auto selectedPlayer = getSelectedPlayer();
  if (selectedPlayer == nullptr) return;

  DlgEditPlayer dlg(db, this, selectedPlayer.get());

  dlg.setModal(true);
  int result = dlg.exec();

  if (result != QDialog::Accepted)
  {
    return;
  }

  //
  // apply the changes, if any
  //

  // name changes
  if (dlg.hasNameChange())
  {
    ERR e = selectedPlayer->rename(dlg.getFirstName(), dlg.getLastName());

    if (e != OK)
    {
      QString msg = tr("Something went wrong when renaming the player. This shouldn't happen.");
      msg += tr("For the records: error code = ") + QString::number(static_cast<int> (e));
      QMessageBox::warning(this, tr("WTF??"), msg);
    }
  }

  // category changes
  CatMngr cmngr{db};

  QHash<Category, bool> catSelection = dlg.getCategoryCheckState();
  QHash<Category, bool>::const_iterator it = catSelection.constBegin();
  while (it != catSelection.constEnd()) {
    Category cat = it.key();
    bool isAlreadyInCat = cat.hasPlayer(*selectedPlayer);
    bool isCatSelected = it.value();

    if (isAlreadyInCat && !isCatSelected) {    // remove player from category
      ERR e = cmngr.removePlayerFromCategory(*selectedPlayer, cat);

      if (e != OK) {
        QString msg = tr("Something went wrong when removing the player from a category. This shouldn't happen.");
        msg += tr("For the records: error code = ") + QString::number(static_cast<int> (e));
        QMessageBox::warning(this, tr("WTF??"), msg);
      }
    }

    if (!isAlreadyInCat && isCatSelected) {    // add player to category
      ERR e = cmngr.addPlayerToCategory(*selectedPlayer, cat);

      if (e != OK) {
        QString msg = tr("Something went wrong when adding the player to a category. This shouldn't happen.");
        msg += tr("For the records: error code = ") + QString::number(static_cast<int> (e));
        QMessageBox::warning(this, tr("WTF??"), msg);
      }
    }
    ++it;
  }

  // Team changes
  TeamMngr tmngr{db};
  Team newTeam = dlg.getTeam();
  if (newTeam != selectedPlayer->getTeam())
  {
    ERR e = tmngr.changeTeamAssigment(*selectedPlayer, newTeam);

    if (e != OK) {
      QString msg = tr("Something went wrong when changing the player's team assignment. This shouldn't happen.");
      msg += tr("For the records: error code = ") + QString::number(static_cast<int> (e));
      QMessageBox::warning(this, tr("WTF??"), msg);
    }
  }
}

//----------------------------------------------------------------------------

void PlayerTableView::onRemovePlayerTriggered()
{
  auto p = getSelectedPlayer();
  if (p == nullptr) return;

  PlayerMngr pm{db};

  // can the player be deleted at all?
  ERR err = pm.canDeletePlayer(*p);

  // player is still paired in a not-yet-started
  // category
  if (err == PLAYER_ALREADY_PAIRED)
  {
    QString msg = tr("The player can't be removed from all categories.\n");
    msg += tr("Please make sure that the player is not assigned to any\n");
    msg += tr("partners in doubles or mixed categories!");
    QMessageBox::critical(this, tr("Delete player"), msg);
    return;
  }

  // player in started category
  if ((err != OK) && (err != PLAYER_ALREADY_PAIRED))
  {
    QString msg = tr("The player can't be deleted anymore. The player is\n");
    msg += tr("most likely already involved/scheduled in matches.");
    QMessageBox::critical(this, tr("Delete player"), msg);
    return;
  }

  // okay, the player can be deleted. Get a confirmation
  QString msg = tr("Note: this will remove the player from all categories\n");
  msg += tr("and from the whole tournament.\n\n");
  msg += tr("This step is irrevocable!\n\n");
  msg += tr("Proceed?");
  int result = QMessageBox::question(this, tr("Delete player"), msg);
  if (result != QMessageBox::Yes) return;

  // we can actually delete the player. Let's go!
  err = pm.deletePlayer(*p);
  if (err != OK) {
    QString msg = tr("Something went wrong when deleting the player. This shouldn't happen.\n\n");
    msg += tr("For the records: error code = ") + QString::number(static_cast<int> (err));
    QMessageBox::warning(this, tr("WTF??"), msg);
  }
}

//----------------------------------------------------------------------------

void PlayerTableView::onShowNextMatchesForPlayerTriggered()
{
  auto selectedPlayer = getSelectedPlayer();
  if (selectedPlayer == nullptr) return;

  PlayerMngr pm{db};
  vector<Match> nextMatches = pm.getAllScheduledMatchesForPlayer(*selectedPlayer);

  QString msg;
  if (nextMatches.empty())
  {
    msg = tr("%1 has currently no scheduled matches.");
    msg = msg.arg(selectedPlayer->getDisplayName_FirstNameFirst());
  } else {
    msg = tr("Next scheduled matches for %1:\n\n");
    msg = msg.arg(selectedPlayer->getDisplayName_FirstNameFirst());

    // get a new MatchTimePredictor for retrieving the
    // estimated start times
    MatchTimePredictor predictor{db};

    for (const Match& ma : nextMatches)
    {
      QString maTxt = tr("   Match %1, %2, round %3 ; estimated call at %4\n\n");
      maTxt = maTxt.arg(ma.getMatchNumber());
      maTxt = maTxt.arg(ma.getCategory().getName());
      maTxt = maTxt.arg(ma.getMatchGroup().getRound());

      MatchTimePrediction mtp = predictor.getPredictionForMatch(ma);
      if (mtp.estStartTime__UTC > 0)
      {
        QDateTime start = QDateTime::fromTime_t(mtp.estStartTime__UTC);
        maTxt = maTxt.arg(start.toString("HH:mm"));
      } else {
        maTxt = maTxt.arg("??");
      }

      msg += maTxt;
    }
  }

  QMessageBox::information(this, "Next matches for player", msg);
}

//----------------------------------------------------------------------------

void PlayerTableView::onRegisterPlayerTriggered()
{
  auto selectedPlayer = getSelectedPlayer();
  if (selectedPlayer == nullptr) return;

  // remove the "wait for registration"-flag
  cmdRegisterPlayer cmd{this, *selectedPlayer};
  cmd.exec();
}

//----------------------------------------------------------------------------

void PlayerTableView::onUnregisterPlayerTriggered()
{
  auto selectedPlayer = getSelectedPlayer();
  if (selectedPlayer == nullptr) return;

  // remove the "wait for registration"-flag
  cmdUnregisterPlayer cmd{this, *selectedPlayer};
  cmd.exec();
}

//----------------------------------------------------------------------------

void PlayerTableView::onImportFromExtDatabase()
{
  cmdImportSinglePlayerFromExternalDatabase cmd{db, this};

  cmd.exec();
}

//----------------------------------------------------------------------------

void PlayerTableView::onExportToExtDatabase()
{
  // check if any player is selected
  auto selPlayer = getSelectedPlayer();
  if (selPlayer == nullptr)
  {
    QMessageBox::warning(this, tr("Export player"), tr("No player selected!"));
    return;
  }

  cmdExportPlayerToExternalDatabase cmd{this, *selPlayer};
  if (cmd.exec() == OK)
  {
    QMessageBox::information(this, tr("Export player"), tr("Player data successfully exported."));
  }
}

//----------------------------------------------------------------------------

void PlayerTableView::onSyncAllToExtDatabase()
{
  PlayerMngr pm{db};

  ERR err = pm.syncAllPlayersToExternalDatabase();
  if (err != OK)
  {
    QMessageBox::warning(this, tr("Sync players"), tr("No database open!"));
    return;
  }

  QMessageBox::information(this, tr("Sync players"), tr("Player data successfully synced."));
}

//----------------------------------------------------------------------------

void PlayerTableView::initContextMenu()
{
  // prepare all actions
  actAddPlayer = new QAction(tr("Add Player..."), this);
  actEditPlayer = new QAction(tr("Edit Player..."), this);
  actRemovePlayer = new QAction(tr("Remove Player..."), this);
  actShowNextMatchesForPlayer = new QAction(tr("Show next Matches..."), this);
  actRegister = new QAction(tr("Register"), this);
  actUnregister = new QAction(tr("Undo registration"), this);
  actImportFromExtDatabase = new QAction(tr("Import player..."), this);
  actExportToExtDatabase = new QAction(tr("Export selected player..."), this);
  actSyncAllToExtDatabase = new QAction(tr("Sync all players to database"), this);

  // create the context menu and connect it to the actions
  contextMenu = make_unique<QMenu>();
  contextMenu->addAction(actRegister);
  contextMenu->addAction(actUnregister);
  contextMenu->addSeparator();
  contextMenu->addAction(actAddPlayer);
  contextMenu->addAction(actEditPlayer);
  contextMenu->addSeparator();
  contextMenu->addAction(actShowNextMatchesForPlayer);
  contextMenu->addSeparator();
  contextMenu->addAction(actRemovePlayer);
  contextMenu->addSeparator();
  contextMenu->addAction(actExportToExtDatabase);
  contextMenu->addAction(actImportFromExtDatabase);
  contextMenu->addAction(actSyncAllToExtDatabase);

  // connect actions and slots
  connect(actAddPlayer, SIGNAL(triggered(bool)), this, SLOT(onAddPlayerTriggered()));
  connect(actEditPlayer, SIGNAL(triggered(bool)), this, SLOT(onEditPlayerTriggered()));
  connect(actShowNextMatchesForPlayer, SIGNAL(triggered(bool)), this, SLOT(onShowNextMatchesForPlayerTriggered()));
  connect(actRemovePlayer, SIGNAL(triggered(bool)), this, SLOT(onRemovePlayerTriggered()));
  connect(actRegister, SIGNAL(triggered(bool)), this, SLOT(onRegisterPlayerTriggered()));
  connect(actUnregister, SIGNAL(triggered(bool)), this, SLOT(onUnregisterPlayerTriggered()));
  connect(actExportToExtDatabase, SIGNAL(triggered(bool)), this, SLOT(onExportToExtDatabase()));
  connect(actImportFromExtDatabase, SIGNAL(triggered(bool)), this, SLOT(onImportFromExtDatabase()));
  connect(actSyncAllToExtDatabase, SIGNAL(triggered(bool)), this, SLOT(onSyncAllToExtDatabase()));
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
    

