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

#include <QMessageBox>

#include "PlayerTabWidget.h"
#include "MainFrame.h"
#include "ui/commonCommands/cmdImportSinglePlayerFromExternalDatabase.h"
#include "ui/commonCommands/cmdExportPlayerToExternalDatabase.h"
#include "DlgBulkImportToExtDb.h"
#include "ExternalPlayerDB.h"
#include "DlgPickPlayerSex.h"
#include "CentralSignalEmitter.h"
#include "PlayerMngr.h"
#include "TeamMngr.h"
#include "CatMngr.h"
#include "ui/DlgImportCSV_Step1.h"
#include "ui/DlgImportCSV_Step2.h"

PlayerTabWidget::PlayerTabWidget()
:QWidget(), db(nullptr)
{
  ui.setupUi(this);

  // prepare a label with the total number of players
  onPlayerCountChanged();

  // initialize the registration popup menu
  initRegistrationMenu();

  // initialize the external database popup menu
  initExternalDatabaseMenu();

  // react on selection changes in the player list table
  connect(ui.playerView->selectionModel(),
    SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
    SLOT(onPlayerSelectionChanged(QItemSelection,QItemSelection)));

  // connect to all events that modify the number of players
  CentralSignalEmitter* cse = CentralSignalEmitter::getInstance();
  connect(cse, SIGNAL(endCreatePlayer(int)), this, SLOT(onPlayerCountChanged()));
  connect(cse, SIGNAL(endDeletePlayer()), this, SLOT(onPlayerCountChanged()));

  // connect to the "external player database changed" signel emitted by the player manager
  connect(cse, SIGNAL(externalPlayerDatabaseChanged()), this, SLOT(onExternalDatabaseChanged()), Qt::DirectConnection);

}

//----------------------------------------------------------------------------

PlayerTabWidget::~PlayerTabWidget()
{
}

//----------------------------------------------------------------------------

void PlayerTabWidget::setDatabase(TournamentDB* _db)
{
  db = _db;

  // distribute to childs
  ui.playerView->setDatabase(db);

  setEnabled(db != nullptr);

  // update the player count information
  onPlayerCountChanged();
}

//----------------------------------------------------------------------------

void PlayerTabWidget::initRegistrationMenu()
{
  // prepare all actions
  actRegisterAll = new QAction(tr("Register all..."), this);
  actUnregisterAll = new QAction(tr("Unegister all..."), this);

  // create the context menu and connect it to the actions
  registrationMenu = make_unique<QMenu>();
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
  extDatabaseMenu = make_unique<QMenu>();
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
  ui.playerView->onPlayerDoubleCLicked();
}

//----------------------------------------------------------------------------

void PlayerTabWidget::onPlayerCountChanged()
{
  if (db == nullptr)
  {
    ui.laPlayerCount->setText(QString()); // no tournament started / opem
    return;
  }

  PlayerMngr pm{db};
  QString txt = QString::number(pm.getTotalPlayerCount());
  txt += tr(" players in tournament");
  ui.laPlayerCount->setText(txt);
}

//----------------------------------------------------------------------------

void PlayerTabWidget::onRegisterAllTriggered()
{
  QString msg = tr("Do you really want to set all players\n");
  msg += tr("to \"registered\"?");
  int result = QMessageBox::question(this, tr("Register all"), msg);
  if (result != QMessageBox::Yes) return;

  // loop over all players and set them to "registered"
  PlayerMngr pm{db};
  for (const Player& pl : pm.getAllPlayers())
  {
    pm.setWaitForRegistration(pl, false);
  }
}

//----------------------------------------------------------------------------

void PlayerTabWidget::onUnregisterAllTriggered()
{
  QString msg = tr("Do you really want to set all players\n");
  msg += tr("to \"wait for registration\"?");
  int result = QMessageBox::question(this, tr("Unregister all"), msg);
  if (result != QMessageBox::Yes) return;

  // loop over all players and set them to "Wait for registration"
  PlayerMngr pm{db};
  bool allModified = true;
  ERR err;
  for (const Player& pl : pm.getAllPlayers())
  {
    err = pm.setWaitForRegistration(pl, true);
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
  cmdImportSinglePlayerFromExternalDatabase cmd{db, this};

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
  PlayerMngr pm{db};
  ui.btnExtDatabase->setEnabled(pm.hasExternalPlayerDatabaseAvailable());
  onPlayerSelectionChanged(QItemSelection(), QItemSelection());
}

//----------------------------------------------------------------------------

void PlayerTabWidget::onPlayerSelectionChanged(const QItemSelection&, const QItemSelection&)
{
  auto selPlayer = ui.playerView->getSelectedPlayer();

  actExportToExtDatabase->setEnabled(selPlayer != nullptr);
}

//----------------------------------------------------------------------------

void PlayerTabWidget::onImportCsv()
{
  DlgImportCSV_Step1 dlg1{this, db};

  int rc = dlg1.exec();
  if (rc != QDialog::Accepted) return;

  auto sourceImportRecords = convertCSVfromPlainText(db, dlg1.getSplitData());

  DlgImportCSV_Step2 dlg2{this, db, sourceImportRecords};
  rc = dlg2.exec();
  if (rc != QDialog::Accepted) return;

  // get the "consolidated" records
  vector<CSVImportRecord> records = dlg2.getRecords();
  if (records.empty()) return;  // nothing to do

  // make sure the data set is error free.
  // this SHOULD have been checked by the DlgImportCSV_Step2 before,
  // but better safe than sorry
  vector<CSVError> errList = analyseCSV(db, records);
  for (const CSVError& err : errList)
  {
    if (err.isFatal)
    {
      QString msg = tr("The CSV records contain errors and thus can't be imported!");
      QMessageBox::critical(this, tr("Import CSV"), msg);
      return;
    }
  }

  // the actual import
  TeamMngr tm{db};
  PlayerMngr pm{db};
  CatMngr cm{db};
  ERR err = OK;
  QString msg;
  for (const CSVImportRecord& rec : records)
  {
    if (!(tm.hasTeam(rec.getTeamName())))
    {
      err = tm.createNewTeam(rec.getTeamName());
      if (err != OK)
      {
        msg = tr("Error when creating the team '%1'.");
        msg = msg.arg(rec.getTeamName());
        break;
      }
    }

    if (!(rec.hasExistingName()))
    {
      err = pm.createNewPlayer(rec.getFirstName(), rec.getLastName(), rec.getSex(), rec.getTeamName());
      if (err != OK)
      {
        msg = tr("Error when creating the player '%1 %2'.");
        msg = msg.arg(rec.getFirstName()).arg(rec.getLastName());
        break;
      }
    }

    Player p = *(rec.getExistingPlayer());   // should return the newly created or existing player

    for (const QString& cName : rec.getCatNames())
    {
      // skip non-existing categories
      if (!(cm.hasCategory(cName))) continue;

      Category cat = cm.getCategory(cName);

      // try to add the player to the category.
      // ignore potential errors; successfull category assignment is optional...
      cm.addPlayerToCategory(p, cat);
    }

    // done.
  }
  if ((err != OK) && !(msg.isEmpty()))
  {
    msg += tr("Import aborted.\n\nThe import might have ");
    msg += tr("partially succeeded up to this point.");
    QMessageBox::critical(this, tr("Import CSV"), msg);
    return;
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

