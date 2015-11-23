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
#include "DlgBulkImportToExtDb.h"
#include "ExternalPlayerDB.h"
#include "DlgPickPlayerSex.h"

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

  // react on selection changes in the player list table
  connect(ui.playerView->selectionModel(),
    SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
    SLOT(onPlayerSelectionChanged(QItemSelection,QItemSelection)));
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
  actImportCSV = new QAction(tr("Import CSV data to database"), this);

  // create the context menu and connect it to the actions
  extDatabaseMenu = unique_ptr<QMenu>(new QMenu());
  extDatabaseMenu->addAction(actImportFromExtDatabase);
  extDatabaseMenu->addSeparator();
  extDatabaseMenu->addAction(actExportToExtDatabase);
  extDatabaseMenu->addAction(actSyncAllToExtDatabase);
  extDatabaseMenu->addSeparator();
  extDatabaseMenu->addAction(actImportCSV);

  // connect actions and slots
  connect(actImportFromExtDatabase, SIGNAL(triggered(bool)), this, SLOT(onImportFromExtDatabase()));
  connect(actExportToExtDatabase, SIGNAL(triggered(bool)), this, SLOT(onExportToExtDatabase()));
  connect(actSyncAllToExtDatabase, SIGNAL(triggered(bool)), this, SLOT(onSyncAllToExtDatabase()));
  connect(actImportCSV, SIGNAL(triggered(bool)), this, SLOT(onImportCSV()));

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
  ui.playerView->onExportToExtDatabase();
}

//----------------------------------------------------------------------------

void PlayerTabWidget::onSyncAllToExtDatabase()
{
  ui.playerView->onSyncAllToExtDatabase();
}

//----------------------------------------------------------------------------

void PlayerTabWidget::onExternalDatabaseChanged()
{
  PlayerMngr* pm = Tournament::getPlayerMngr();
  ui.btnExtDatabase->setEnabled(pm->hasExternalPlayerDatabaseOpen());
  onPlayerSelectionChanged(QItemSelection(), QItemSelection());
}

//----------------------------------------------------------------------------

void PlayerTabWidget::onPlayerSelectionChanged(const QItemSelection&, const QItemSelection&)
{
  auto selPlayer = ui.playerView->getSelectedPlayer();

  actExportToExtDatabase->setEnabled(selPlayer != nullptr);
}

//----------------------------------------------------------------------------

void PlayerTabWidget::onImportCSV()
{
  PlayerMngr* pm = Tournament::getPlayerMngr();
  if (!(pm->hasExternalPlayerDatabaseOpen()))
  {
    return;
  }

  // display a dialog for data input
  DlgBulkImportToExtDb dlg{this};
  if (dlg.exec() != QDialog::Accepted)
  {
    return;
  }

  // trigger the bulk import and display results
  QString csv = dlg.getText();
  if (csv.isEmpty())
  {
    QString msg = tr("No text provided.");
    QMessageBox::warning(this, "Import CSV data", msg);
    return;
  }
  QList<int> newPlayerIds;
  QList<int> skippedPlayerIds;
  int errorCnt;
  tie(newPlayerIds, skippedPlayerIds, errorCnt) = pm->getExternalPlayerDatabaseHandle()->bulkImportCSV(csv);

  // do we actually have valid names in the list?
  bool hasValidNames = ((newPlayerIds.length() + skippedPlayerIds.length()) > 0);

  // did the user request an import to the tournament as well?
  if (dlg.getTargetTeamId() > 0)
  {
    // stop here if don't have any valid names
    if (!hasValidNames)
    {
      QString msg = tr("No valid names for the import to the\n");
      msg += tr("external database and to the tournament found.");
      QMessageBox::warning(this, "Import CSV data", msg);
      return;
    }

    TeamMngr* tm = Tournament::getTeamMngr();
    CatMngr* cm = Tournament::getCatMngr();

    // get the team for adding the players to. The dialog
    // guarantees that the ID is valid
    int targetTeamId = dlg.getTargetTeamId();
    Team targetTeam = tm->getTeamById(targetTeamId);
    QString targetTeamName = targetTeam.getName();

    // shall the players be added to a category as well?
    unique_ptr<Category> targetCat;
    if (dlg.getTargetCatId() > 0)
    {
      // The dialog guarantees that the ID is valid
      targetCat = cm->getCategory(dlg.getTargetCatId());
    }

    // add the players one by one
    QList<int> validExtPlayerIds{newPlayerIds};
    validExtPlayerIds.append(skippedPlayerIds);
    int dupeCnt = 0;
    int skipCnt = 0;
    int notAddedToCatCnt = 0;
    auto extDb = pm->getExternalPlayerDatabaseHandle();
    for (int extId : validExtPlayerIds)
    {
      auto extPlayer = extDb->getPlayer(extId);
      if (extPlayer == nullptr) continue;  // shouldn't happen

      // make sure the player has a valid sex assigned
      SEX playerSex = extPlayer->getSex();
      if (playerSex == DONT_CARE)
      {
        DlgPickPlayerSex dlg{this, extPlayer->getFirstname() + " " + extPlayer->getLastname()};
        if (dlg.exec() != QDialog::Accepted)
        {
          skipCnt;
          continue;
        }

        // update the database
        playerSex = dlg.getSelectedSex();
        extDb->updatePlayerSexIfUndefined(extId, playerSex);
      }

      // try to add the player to the tournament
      ERR err = pm->createNewPlayer(extPlayer->getFirstname(), extPlayer->getLastname(), playerSex, targetTeamName);
      if (err == NAME_EXISTS)
      {
        ++dupeCnt;
      }
      else if (err != OK)
      {
        // some other error occured and the player
        // has not been added to the tournament
        ++skipCnt;
        continue;
      }

      // at this point, we can be sure the player
      // is in the tournament.
      // Add the player to a category as well?
      if (targetCat != nullptr)
      {
        Player newPlayer = pm->getPlayer(extPlayer->getFirstname(), extPlayer->getLastname());
        err = cm->addPlayerToCategory(newPlayer, *targetCat);
        if ((err != OK) && (err != PLAYER_ALREADY_IN_CATEGORY))
        {
          ++notAddedToCatCnt;
        }
      }
    }

    // show a summary
    QString msg = tr("Import results:\n\n");
    msg += tr("\t%1 valid names found\n");
    msg += tr("\t\t%2 newly imported\n");
    msg += tr("\t\t%3 already existing in the player database\n");
    msg += tr("\t%4 players newly added to the tournament\n");
    msg += tr("\t%5 players were already in the tournament before the import\n");
    if (targetCat != nullptr)
    {
      msg += tr("\tOf %1 valid players, %6 were added to category %7 and %8 not.");
    }
    msg += tr("\t%9 lines contained errors were ignored\n");
    msg = msg.arg(validExtPlayerIds.length());
    msg = msg.arg(newPlayerIds.length());
    msg = msg.arg(skippedPlayerIds.length());
    msg = msg.arg(validExtPlayerIds.length() - dupeCnt - skipCnt);
    msg = msg.arg(dupeCnt);
    if (targetCat != nullptr)
    {
      msg = msg.arg(validExtPlayerIds.length() - notAddedToCatCnt);
      msg = msg.arg(targetCat->getName());
      msg = msg.arg(notAddedToCatCnt);
    }
    msg = msg.arg(errorCnt);
    QMessageBox::information(this, "Import CSV data", msg);
  }
  else
  {
    // the user chose to simply import players to the external
    // database, not to the tournament. Display a short import summary
    QString msg = tr("Import results:\n\n");
    msg += tr("\t%1 names imported\n");
    msg += tr("\t%2 names already existing and skipped\n");
    msg += tr("\t%3 line with errors and ignored\n");
    msg = msg.arg(newPlayerIds.length()).arg(skippedPlayerIds.length()).arg(errorCnt);
    QMessageBox::information(this, "Import CSV data", msg);
  }
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

