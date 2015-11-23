/* 
 * File:   PlayerTableView.cpp
 * Author: volker
 * 
 * Created on March 17, 2014, 8:19 PM
 */

#include <QMessageBox>

#include "PlayerTableView.h"
#include "MainFrame.h"
#include "dlgEditPlayer.h"
#include "ui/commonCommands/cmdRegisterPlayer.h"
#include "ui/commonCommands/cmdUnregisterPlayer.h"
#include "ui/commonCommands/cmdImportSinglePlayerFromExternalDatabase.h"
#include "ui/commonCommands/cmdExportPlayerToExternalDatabase.h"
#include "ui/commonCommands/cmdCreatePlayerFromDialog.h"

PlayerTableView::PlayerTableView(QWidget* parent)
:QTableView(parent)
{
  // an empty model for clearing the table when
  // no tournament is open
  emptyModel = new QStringListModel();
  
  // prepare a proxy model to support sorting by columns
  sortedModel = new QSortFilterProxyModel();
  sortedModel->setSourceModel(emptyModel);
  setModel(sortedModel);

  connect(MainFrame::getMainFramePointer(), &MainFrame::tournamentOpened, this, &PlayerTableView::onTournamentOpened);
  
  // handle context menu requests
  setContextMenuPolicy(Qt::CustomContextMenu);
  connect(this, SIGNAL(customContextMenuRequested(const QPoint&)),
          this, SLOT(onContextMenuRequested(const QPoint&)));

  // define a delegate for drawing the player items
  itemDelegate = new PlayerItemDelegate(this);
  itemDelegate->setProxy(sortedModel);
  setItemDelegate(itemDelegate);
  
  // setup the context menu
  initContextMenu();

}

//----------------------------------------------------------------------------
    
PlayerTableView::~PlayerTableView()
{
  delete emptyModel;
  delete sortedModel;
  delete itemDelegate;
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
  int selectedSourceRow = sortedModel->mapToSource(indexes.at(0)).row();
  return Tournament::getPlayerMngr()->getPlayerBySeqNum(selectedSourceRow);
}

//----------------------------------------------------------------------------
    
void PlayerTableView::onTournamentOpened(Tournament* _tnmt)
{
  tnmt = _tnmt;
  sortedModel->setSourceModel(Tournament::getPlayerTableModel());
  setEnabled(true);
  
  // connect signals from the Tournament and TeamMngr with my slots
  connect(tnmt, &Tournament::tournamentClosed, this, &PlayerTableView::onTournamentClosed);
  
  // resize columns and rows to content once (we do not want permanent automatic resizing)
  horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
  verticalHeader()->resizeSections(QHeaderView::ResizeToContents);
}

//----------------------------------------------------------------------------
    
void PlayerTableView::onTournamentClosed()
{
  // disconnect from all signals, because
  // the sending objects don't exist anymore
  disconnect(tnmt, &Tournament::tournamentClosed, this, &PlayerTableView::onTournamentClosed);
  
  // invalidate the tournament handle and deactivate the view
  tnmt = nullptr;
  sortedModel->setSourceModel(emptyModel);
  setEnabled(false);
  
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
  //int clickedRow = rowAt(pos.y());
  //int clickedCol = columnAt(pos.x());
  //bool isRowClicked = ((clickedRow >= 0) && (clickedCol >= 0));

  // get the player status for enabling / disabling state-dependent actions
  auto selPlayer = getSelectedPlayer();
  bool isPlayerClicked = (selPlayer != nullptr);
  OBJ_STATE plStat = STAT_CO_DISABLED;   // an arbitrary, dummy default value that has nothing to do with players
  if (isPlayerClicked) plStat = selPlayer->getState();

  // if no player is clicked, we may only add a player.
  // in other cases, we may also try to edit or delete
  // players
  actAddPlayer->setEnabled(true);   // always possible
  actEditPlayer->setEnabled(isPlayerClicked);
  //actShowNextMatchesForPlayer->setEnabled(isPlayerClicked);
  actShowNextMatchesForPlayer->setEnabled(false);  // not yet implemented
  actRemovePlayer->setEnabled(isPlayerClicked);
  actRegister->setEnabled(isPlayerClicked & (plStat == STAT_PL_WAIT_FOR_REGISTRATION));
  actUnregister->setEnabled(isPlayerClicked & (plStat == STAT_PL_IDLE));

  bool hasExtDb = Tournament::getPlayerMngr()->hasExternalPlayerDatabaseOpen();
  actImportFromExtDatabase->setEnabled(hasExtDb);
  actSyncAllToExtDatabase->setEnabled(hasExtDb);
  actExportToExtDatabase->setEnabled(isPlayerClicked && hasExtDb);

  // show the context menu
  QAction* selectedItem = contextMenu->exec(globalPos);
}

//----------------------------------------------------------------------------

void PlayerTableView::onAddPlayerTriggered()
{
  DlgEditPlayer dlg(this);

  dlg.setModal(true);
  cmdCreatePlayerFromDialog cmd{this, &dlg};
  cmd.exec();
}

//----------------------------------------------------------------------------

void PlayerTableView::onEditPlayerTriggered()
{
  auto selectedPlayer = getSelectedPlayer();
  if (selectedPlayer == nullptr) return;

  DlgEditPlayer dlg(this, selectedPlayer.get());

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
  CatMngr* cmngr = Tournament::getCatMngr();

  QHash<Category, bool> catSelection = dlg.getCategoryCheckState();
  QHash<Category, bool>::const_iterator it = catSelection.constBegin();
  while (it != catSelection.constEnd()) {
    Category cat = it.key();
    bool isAlreadyInCat = cat.hasPlayer(*selectedPlayer);
    bool isCatSelected = it.value();

    if (isAlreadyInCat && !isCatSelected) {    // remove player from category
      ERR e = cmngr->removePlayerFromCategory(*selectedPlayer, cat);

      if (e != OK) {
        QString msg = tr("Something went wrong when removing the player from a category. This shouldn't happen.");
        msg += tr("For the records: error code = ") + QString::number(static_cast<int> (e));
        QMessageBox::warning(this, tr("WTF??"), msg);
      }
    }

    if (!isAlreadyInCat && isCatSelected) {    // add player to category
      ERR e = cmngr->addPlayerToCategory(*selectedPlayer, cat);

      if (e != OK) {
        QString msg = tr("Something went wrong when adding the player to a category. This shouldn't happen.");
        msg += tr("For the records: error code = ") + QString::number(static_cast<int> (e));
        QMessageBox::warning(this, tr("WTF??"), msg);
      }
    }
    ++it;
  }

  // Team changes
  TeamMngr* tmngr = Tournament::getTeamMngr();
  Team newTeam = dlg.getTeam();
  if (newTeam != selectedPlayer->getTeam())
  {
    ERR e = tmngr->changeTeamAssigment(*selectedPlayer, newTeam);

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

  auto pm = Tournament::getPlayerMngr();

  // can the player be deleted at all?
  ERR err = pm->canDeletePlayer(*p);

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
  err = pm->deletePlayer(*p);
  if (err != OK) {
    QString msg = tr("Something went wrong when deleting the player. This shouldn't happen.\n\n");
    msg += tr("For the records: error code = ") + QString::number(static_cast<int> (err));
    QMessageBox::warning(this, tr("WTF??"), msg);
  }
}

//----------------------------------------------------------------------------

void PlayerTableView::onShowNextMatchesForPlayerTriggered()
{

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
  cmdImportSinglePlayerFromExternalDatabase cmd{this};

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
  PlayerMngr* pm = Tournament::getPlayerMngr();

  ERR err = pm->syncAllPlayersToExternalDatabase();
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
  contextMenu = unique_ptr<QMenu>(new QMenu());
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
    

