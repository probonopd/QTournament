/*
 * File:   PlayerTabWidget.cpp
 * Author: volker
 *
 * Created on March 19, 2014, 7:38 PM
 */

#include <QMessageBox>

#include "PlayerTabWidget.h"
#include "MainFrame.h"
#include "ui/commonCommands/cmdImportSinglePlayerFromExternalDatabase.h"
#include "ui/commonCommands/cmdExportPlayerToExternalDatabase.h"

PlayerTabWidget::PlayerTabWidget()
:QWidget()
{
  ui.setupUi(this);

  // prepare a label with the total number of players
  onPlayerCountChanged();

  // subscribe to the tournament-opened- and -closed-signal
  auto mf = MainFrame::getMainFramePointer();
  connect(mf, SIGNAL(tournamentOpened(Tournament*)), this, SLOT(onTournamentOpened()), Qt::DirectConnection);
  connect(mf, SIGNAL(tournamentClosed()), this, SLOT(onTournamentClosed()), Qt::DirectConnection);

  // initialize the registration popup menu
  initRegistrationMenu();

  // initialize the external database popup menu
  initExternalDatabaseMenu();
}

//----------------------------------------------------------------------------

PlayerTabWidget::~PlayerTabWidget()
{
}

void PlayerTabWidget::initRegistrationMenu()
{
  // prepare all actions
  actRegisterAll = new QAction(tr("Register all..."), this);
  actUnregisterAll = new QAction(tr("Unegister all..."), this);

  // create the context menu and connect it to the actions
  registrationMenu = unique_ptr<QMenu>(new QMenu());
  registrationMenu->addAction(actRegisterAll);
  registrationMenu->addAction(actUnregisterAll);

  // connect actions and slots
  connect(actRegisterAll, SIGNAL(triggered(bool)), this, SLOT(onRegisterAllTriggered()));
  connect(actUnregisterAll, SIGNAL(triggered(bool)), this, SLOT(onUnregisterAllTriggered()));

  // assign the menu to the tool button
  ui.btnRegistration->setMenu(registrationMenu.get());
  ui.btnRegistration->setPopupMode(QToolButton::InstantPopup);
}

//----------------------------------------------------------------------------

void PlayerTabWidget::initExternalDatabaseMenu()
{
  // prepare actions
  actImportFromExtDatabase = new QAction(tr("Import player..."), this);
  actExportToExtDatabase = new QAction(tr("Export selected player..."), this);
  actSyncAllToExtDatabase = new QAction(tr("Sync all players to database"), this);

  // create the context menu and connect it to the actions
  extDatabaseMenu = unique_ptr<QMenu>(new QMenu());
  extDatabaseMenu->addAction(actImportFromExtDatabase);
  extDatabaseMenu->addSeparator();
  extDatabaseMenu->addAction(actExportToExtDatabase);
  extDatabaseMenu->addAction(actSyncAllToExtDatabase);

  // connect actions and slots
  connect(actImportFromExtDatabase, SIGNAL(triggered(bool)), this, SLOT(onImportFromExtDatabase()));
  connect(actExportToExtDatabase, SIGNAL(triggered(bool)), this, SLOT(onExportToExtDatabase()));
  connect(actSyncAllToExtDatabase, SIGNAL(triggered(bool)), this, SLOT(onSyncAllToExtDatabase()));

  // assign the menu to the tool button
  ui.btnExtDatabase->setMenu(extDatabaseMenu.get());
  ui.btnExtDatabase->setPopupMode(QToolButton::InstantPopup);
}

//----------------------------------------------------------------------------

void PlayerTabWidget::onCreatePlayerClicked()
{
  ui.playerView->onAddPlayerTriggered();
}

//----------------------------------------------------------------------------

void PlayerTabWidget::onPlayerDoubleClicked(const QModelIndex& index)
{
  ui.playerView->onEditPlayerTriggered();
}

//----------------------------------------------------------------------------

void PlayerTabWidget::onPlayerCountChanged()
{
  PlayerMngr* pm = Tournament::getPlayerMngr();
  if (pm == nullptr)
  {
    ui.laPlayerCount->setText(QString()); // no tournament started / opem
  } else {
    QString txt = QString::number(pm->getTotalPlayerCount());
    txt += tr(" players in tournament");
    ui.laPlayerCount->setText(txt);
  }
}

//----------------------------------------------------------------------------

void PlayerTabWidget::onTournamentOpened()
{
  // connect to all events that modify the number of players
  PlayerMngr* pm = Tournament::getPlayerMngr();
  if (pm == nullptr) return;
  connect(pm, SIGNAL(endCreatePlayer(int)), this, SLOT(onPlayerCountChanged()));
  connect(pm, SIGNAL(endDeletePlayer()), this, SLOT(onPlayerCountChanged()));

  // connect to the "external player database changed" signel emitted by the player manager
  connect(pm, SIGNAL(externalPlayerDatabaseChanged()), this, SLOT(onExternalDatabaseChanged()), Qt::DirectConnection);

  onPlayerCountChanged();
}

//----------------------------------------------------------------------------

void PlayerTabWidget::onTournamentClosed()
{
  // disconnect all signals
  PlayerMngr* pm = Tournament::getPlayerMngr();
  if (pm == nullptr) return;
  disconnect(pm, SIGNAL(endCreatePlayer(int)), this, SLOT(onPlayerCountChanged()));
  disconnect(pm, SIGNAL(endDeletePlayer()), this, SLOT(onPlayerCountChanged()));
  disconnect(pm, SIGNAL(externalPlayerDatabaseChanged()), this, SLOT(onExternalDatabaseChanged()));

  onPlayerCountChanged();
}

//----------------------------------------------------------------------------

void PlayerTabWidget::onRegisterAllTriggered()
{
  QString msg = tr("Do you really want to set all players\n");
  msg += "to \"registered\"?";
  int result = QMessageBox::question(this, tr("Register all"), msg);
  if (result != QMessageBox::Yes) return;

  // loop over all players and set them to "registered"
  PlayerMngr* pm = Tournament::getPlayerMngr();
  for (const Player& pl : pm->getAllPlayers())
  {
    pm->setWaitForRegistration(pl, false);
  }
}

//----------------------------------------------------------------------------

void PlayerTabWidget::onUnregisterAllTriggered()
{
  QString msg = tr("Do you really want to set all players\n");
  msg += "to \"wait for registration\"?";
  int result = QMessageBox::question(this, tr("Unregister all"), msg);
  if (result != QMessageBox::Yes) return;

  // loop over all players and set them to "Wait for registration"
  PlayerMngr* pm = Tournament::getPlayerMngr();
  bool allModified = true;
  ERR err;
  for (const Player& pl : pm->getAllPlayers())
  {
    err = pm->setWaitForRegistration(pl, true);
    if (err != OK) allModified = false;
  }

  // display an information text if not all
  // players could be updated
  if (!allModified)
  {
    msg = "Could not update all players because some\n";
    msg += "players are already assigned to matches\n";
    msg += "or running categories.";

    QMessageBox::information(this, "Unregister all", msg);
  }
}

//----------------------------------------------------------------------------

void PlayerTabWidget::onImportFromExtDatabase()
{
  cmdImportSinglePlayerFromExternalDatabase cmd{this};

  cmd.exec();
}

//----------------------------------------------------------------------------

void PlayerTabWidget::onExportToExtDatabase()
{
  // check if any player is selected
  auto selPlayer = ui.playerView->getSelectedPlayer();
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

void PlayerTabWidget::onSyncAllToExtDatabase()
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

void PlayerTabWidget::onExternalDatabaseChanged()
{
  PlayerMngr* pm = Tournament::getPlayerMngr();
  ui.btnExtDatabase->setEnabled(pm->hasExternalPlayerDatabaseOpen());
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

