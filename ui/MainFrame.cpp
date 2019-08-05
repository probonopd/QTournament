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

#include <stdexcept>

#include <QMessageBox>
#include <QFileDialog>
#include <QFile>
#include <QTime>
#include <QPushButton>

#include "MainFrame.h"
#include "MatchMngr.h"
#include "CourtMngr.h"
#include "PlayerMngr.h"
#include "TeamMngr.h"
#include "CatMngr.h"
#include "ui/DlgTournamentSettings.h"
#include "CourtMngr.h"
#include "OnlineMngr.h"
#include "DlgPassword.h"
#include "commonCommands/cmdOnlineRegistration.h"
#include "commonCommands/cmdSetOrChangePassword.h"
#include "commonCommands/cmdStartOnlineSession.h"
#include "commonCommands/cmdDeleteFromServer.h"
#include "commonCommands/cmdConnectionSettings.h"

using namespace QTournament;

//----------------------------------------------------------------------------

MainFrame::MainFrame()
  :currentDb(nullptr), lastAutosaveDirtyCounterValue(0), lastDirtyState(false)
{
  ui.setupUi(this);
  showMaximized();

  testFileName = QDir().absoluteFilePath("tournamentTestFile.tdb");

  // no database file is initially active
  currentDatabaseFileName.clear();

  // prepare an action to toggle the test-menu's visibility
  scToggleTestMenuVisibility = new QShortcut(QKeySequence(Qt::CTRL + Qt::ALT + Qt::Key_T), this);
  scToggleTestMenuVisibility->setContext(Qt::ApplicationShortcut);
  connect(scToggleTestMenuVisibility, SIGNAL(activated()), this, SLOT(onToggleTestMenuVisibility()));
  isTestMenuVisible = true;
  onToggleTestMenuVisibility();

  // initialize timers for polling the database's dirty flag
  // and for triggering the autosave function
  dirtyFlagPollTimer = make_unique<QTimer>(this);
  connect(dirtyFlagPollTimer.get(), SIGNAL(timeout()), this, SLOT(onDirtyFlagPollTimerElapsed()));
  dirtyFlagPollTimer->start(DIRTY_FLAG_POLL_INTERVALL__MS);
  autosaveTimer = make_unique<QTimer>(this);
  connect(autosaveTimer.get(), SIGNAL(timeout()), this, SLOT(onAutosaveTimerElapsed()));
  autosaveTimer->start(AUTOSAVE_INTERVALL__MS);

  // prepare a status bar label that shows the last autosave time
  lastAutosaveTimeStatusLabel = new QLabel(statusBar());
  lastAutosaveTimeStatusLabel->clear();
  statusBar()->addPermanentWidget(lastAutosaveTimeStatusLabel);

  // prepare a timer and label that show the current sync state
  syncStatLabel = new QLabel(statusBar());
  syncStatLabel->clear();
  statusBar()->addWidget(syncStatLabel);
  serverSyncTimer = make_unique<QTimer>(this);
  connect(serverSyncTimer.get(), SIGNAL(timeout()), this, SLOT(onServerSyncTimerElapsed()));
  serverSyncTimer->start(ServerSyncStatusInterval_ms);

  // prepare a button for triggering a server ping test
  btnPingTest = new QPushButton(statusBar());
  btnPingTest->setText(tr("Ping"));
  statusBar()->addWidget(btnPingTest);
  connect(btnPingTest, SIGNAL(clicked(bool)), this, SLOT(onBtnPingTestClicked()));
  btnPingTest->setEnabled(false);


  // finally disable all widgets by setting their database instance to nullptr
  distributeCurrentDatabasePointerToWidgets();
  enableControls(false);

}

//----------------------------------------------------------------------------

MainFrame::~MainFrame()
{
}

//----------------------------------------------------------------------------

void MainFrame::newTournament()
{
  // show a dialog for setting the tournament parameters
  DlgTournamentSettings dlg{this};
  dlg.setModal(true);
  int result = dlg.exec();

  if (result != QDialog::Accepted)
  {
    return;
  }

  // make sure we get the settings from the dialog
  auto settings = dlg.getTournamentSettings();
  if (settings == nullptr)
  {
    QMessageBox::warning(this, tr("New tournament"), tr("Something went wrong; no new tournament created."));
    return;
  }

  // close any open tournament
  if (currentDb != nullptr)
  {
    bool isOkay = closeCurrentTournament();
    if (!isOkay) return;
  }

  ERR err;
  auto newDb = TournamentDB::createNew(":memory:", *settings, &err);
  if ((err != OK) || (newDb == nullptr))
  {
    // shouldn't happen, because the file has been
    // deleted before (see above)
    QMessageBox::warning(this, tr("New tournament"), tr("Something went wrong; no new tournament created."));
    return;
  }

  // make the new database the current one
  currentDb = std::move(newDb);
  distributeCurrentDatabasePointerToWidgets();

  // create the initial number of courts
  CourtMngr cm{currentDb.get()};
  for (int i=0; i < dlg.getCourtCount(); ++i)
  {
    ERR err;
    cm.createNewCourt(i+1, QString::number(i+1), &err);

    // no error checking here. Creation at this point must always succeed.
  }

  // prepare for autosaving
  lastDirtyState = false;
  lastAutosaveDirtyCounterValue = 0;
  onAutosaveTimerElapsed();
  enableControls(true);
  updateWindowTitle();
}

//----------------------------------------------------------------------------

void MainFrame::openTournament()
{
  // ask for the file name
  QFileDialog fDlg{this};
  fDlg.setAcceptMode(QFileDialog::AcceptOpen);
  fDlg.setFileMode(QFileDialog::ExistingFile);
  fDlg.setNameFilter(tr("QTournament Files (*.tdb)"));
  int result = fDlg.exec();

  if (result != QDialog::Accepted)
  {
    return;
  }

  // get the filename
  QString filename = fDlg.selectedFiles().at(0);

  // try to open the tournament file DIRECTLY
  //
  // we operate only temporarily directly on the file
  // until possible format conversions etc. are completed.
  ERR err;
  auto newDb = TournamentDB::openExisting(filename, &err);

  // check for errors
  if (err == INCOMPATIBLE_FILE_FORMAT)
  {
    QString msg = tr("The file has been created with an incompatible\n");
    msg += tr("version of QTournament and can't be opened.");
    QMessageBox::critical(this, tr("Open tournament"), msg);
    return;
  }
  if (err == FILE_NOT_EXISTING)   // shouldn't happen after the previous checks
  {
    QString msg = tr("Couldn't open ") + filename;
    QMessageBox::critical(this, tr("Open tournament"), msg);
    return;
  }
  if ((err != OK) || (newDb == nullptr))
  {
    QMessageBox::warning(this, tr("Open tournament"), tr("Something went wrong; no tournament opened."));
    return;
  }

  // do we need to convert this database to a new format?
  if (newDb->needsConversion())
  {
    QString msg = tr("The file has been created with an older version of QTournament.\n\n");
    msg += tr("The file is not compatible with the current version but it can be updated ");
    msg += tr("to the current version. If you upgrade, you will no longer be able to open ");
    msg += tr("the file with older versions of QTournament.\n\n");
    msg += tr("The conversion cannot be undone.\n\n");
    msg += tr("Do you want to proceed and update the file?");
    int result = QMessageBox::question(this, tr("Convert file format?"), msg);
    if (result != QMessageBox::Yes) return;

    bool conversionOk = newDb->convertToLatestDatabaseVersion();
    if (conversionOk)
    {
      msg = tr("The file was successfully converted!");
      QMessageBox::information(this, tr("Convert file format"), msg);
    } else {
      msg = tr("The file conversion failed. The tournament could not be opened.");
      QMessageBox::critical(this, tr("Convert file format"), msg);
      return;
    }
  }

  // close any open tournament
  if (currentDb != nullptr)
  {
    bool isOkay = closeCurrentTournament();
    if (!isOkay) return;
  }

  // close the temporarily opened tournament database and
  // re-open it as a copy in memory
  if (!(newDb->close()))    // this is very unlikely to happen...
  {
    QString msg = tr("An internal error occured. No tournament opened.");
    QMessageBox::warning(this, tr("Open failed"), msg);
    return;
  }
  int dbErr;
  newDb = SqliteDatabase::get<TournamentDB>(":memory:", true);
  newDb->restoreFromFile(filename.toUtf8().constData(), &dbErr);
  newDb->setLogLevel(Sloppy::Logger::SeverityLevel::error);

  // handle errors
  QString msg;
  if (dbErr == SQLITE_ERROR)
  {
    msg = tr("Could not read from the source file:\n\n");
    msg += filename + "\n\n";
    msg += tr("The tournament has not been opened.");
  }
  else if (dbErr != SQLITE_OK)
  {
    msg = tr("A database error occured while opening.\n\n");
    msg += tr("Internal hint: SQLite error code = %1");
    msg = msg.arg(dbErr);
  }
  if (!(msg.isEmpty()))
  {
    QMessageBox::warning(this, tr("Opening failed"), msg);
    return;
  }

  // opening was successfull ==> distribute the database handle to all widgets
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  currentDb = std::move(newDb);
  distributeCurrentDatabasePointerToWidgets();
  enableControls(true);
  QApplication::restoreOverrideCursor();
  currentDatabaseFileName = filename;
  ui.actionCreate_baseline->setEnabled(true);
  lastDirtyState = false;
  lastAutosaveDirtyCounterValue = 0;
  onAutosaveTimerElapsed();

  // BAAAD HACK: when the OnlineManager instance was created, the config table
  // was still empty because instanciation takes place before
  // restoreFromFile() is triggered.
  // Thus, the OnlineManager does not read custom server settings, if
  // existent. We need to trigger this manually here.
  //
  // Remember: in this application it is bad, bad, bad to keep state in the
  // xManagers... or somewhere else
  currentDb->getOnlineManager()->applyCustomServerSettings();

  // show the tournament name in the main window's title
  updateWindowTitle();

  // open the external player database file, if configured
  PlayerMngr pm(currentDb.get());
  if (pm.hasExternalPlayerDatabaseConfigured())
  {
    ERR err = pm.openConfiguredExternalPlayerDatabase();

    if (err == OK) return;

    QString msg;
    switch (err)
    {
    case EPD__NOT_FOUND:
      msg = tr("Could not find the player database\n\n");
      msg += pm.getExternalDatabaseName() + "\n\n";
      msg += tr("Please make sure the file exists and is valid.");
      break;

    default:
      msg = tr("The player database\n\n");
      msg += pm.getExternalDatabaseName() + "\n\n";
      msg += tr("is invalid.");
    }

    QMessageBox::warning(this, "Open player database", msg);
  }
}

//----------------------------------------------------------------------------

void MainFrame::onSave()
{
  if (currentDb == nullptr) return;

  execCmdSave();
}

//----------------------------------------------------------------------------

void MainFrame::onSaveAs()
{
  if (currentDb == nullptr) return;

  execCmdSaveAs();
}

//----------------------------------------------------------------------------

void MainFrame::onSaveCopy()
{
  if (currentDb == nullptr) return;

  QString dstFileName = askForTournamentFileName(tr("Save a copy"));
  if (dstFileName.isEmpty()) return;

  bool isOkay = saveCurrentDatabaseToFile(dstFileName);
  if (isOkay)
  {
    lastAutosaveDirtyCounterValue = currentDb->getDirtyCounter();
    onAutosaveTimerElapsed();
  }
}

//----------------------------------------------------------------------------

void MainFrame::onCreateBaseline()
{
  if (currentDb == nullptr) return;
  if (currentDatabaseFileName.isEmpty()) return;

  // determine the basename of the current database file
  if (!(currentDatabaseFileName.endsWith(".tdb", Qt::CaseInsensitive)))
  {
    QString msg = tr("The current file name is unexpectedly ugly. Can't derive\n");
    msg += tr("baseline names from it.\n\n");
    msg += tr("Nothing has been saved.");
    QMessageBox::warning(this, tr("Baseline creation failed"), msg);
    return;
  }
  QString basename = currentDatabaseFileName.left(currentDatabaseFileName.length() - 4);

  // determine a suitable name for the baseline
  QString dstName;
  int cnt = 0;
  bool nameFound = false;
  while (!nameFound)
  {
    dstName = basename + "__%1.tdb";
    dstName = dstName.arg(cnt, 4, 10, QLatin1Char{'0'});
    nameFound = !(QFile::exists(dstName));
    ++cnt;
  }

  bool isOkay = saveCurrentDatabaseToFile(dstName);
  if (isOkay)
  {
    QString msg = tr("A snapshot of the current tournament status has been saved to:\n\n%1");
    msg = msg.arg(dstName);
    QMessageBox::information(this, "Create baseline", msg);
  }
}

//----------------------------------------------------------------------------

void MainFrame::onClose()
{
  if (currentDb == nullptr) return;

  if (closeCurrentTournament()) enableControls(false);
}

//----------------------------------------------------------------------------

void MainFrame::enableControls(bool doEnable)
{
  ui.centralwidget->setEnabled(doEnable);
  ui.actionSettings->setEnabled(doEnable);
  ui.menuExternal_player_database->setEnabled(doEnable);
  ui.actionSave->setEnabled(doEnable);
  ui.actionSave_as->setEnabled(doEnable);
  ui.actionSave_a_copy->setEnabled(doEnable);
  ui.actionCreate_baseline->setEnabled(doEnable && !(currentDatabaseFileName.isEmpty()));
  ui.actionClose->setEnabled(doEnable);

  ui.menuOnline->setEnabled(doEnable);
  if (doEnable)
  {
    updateOnlineMenu();
  }

  btnPingTest->setEnabled(doEnable);
}

//----------------------------------------------------------------------------

bool MainFrame::closeCurrentTournament()
{
  // close other possibly open tournaments
  if (currentDb != nullptr)
  {
    if (currentDb->isDirty())
    {
      QString msg = tr("Warning: all unsaved changes to the current tournament\n");
      msg += tr("will be lost.\n\n");
      msg += tr("Do you want to save your changes?");

      QMessageBox msgBox{this};
      msgBox.setText(msg);
      msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
      msgBox.setWindowTitle(tr("Save changes?"));
      msgBox.setIcon(QMessageBox::Warning);
      int result = msgBox.exec();

      if (result == QMessageBox::Cancel) return false;

      if (result == QMessageBox::Save)
      {
        bool isOkay = execCmdSave();
        if (!isOkay) return false;
      }
    }

    //
    // At this point, the user either decided to discard all changes
    // or the database has been saved successfully
    //
    // ==> we can shut the current database down
    //

    // disconnect from the external player database, if any
    PlayerMngr pm{currentDb.get()};
    pm.closeExternalPlayerDatabase();

    // force all widgets to forget the database handle
    // BEFORE we actually close the database
    distributeCurrentDatabasePointerToWidgets(true);

    // close the database
    currentDb->close();
    currentDb.reset();
    currentDatabaseFileName.clear();
    onAutosaveTimerElapsed();
  }
  
  // delete the test file, if existing
  if (QFile::exists(testFileName))
  {
    QFile::remove(testFileName);
  }
  
  lastDirtyState = false;
  onAutosaveTimerElapsed();
  enableControls(false);
  updateWindowTitle();

  return true;
}

//----------------------------------------------------------------------------

void MainFrame::distributeCurrentDatabasePointerToWidgets(bool forceNullptr)
{
  TournamentDB* db = forceNullptr ? nullptr : currentDb.get();

  ui.tabPlayers->setDatabase(db);
  ui.tabCategories->setDatabase(db);
  ui.tabTeams->setDatabase(db);
  ui.tabSchedule->setDatabase(db);
  ui.tabReports->setDatabase(db);
  ui.tabMatchLog->setDatabase(db);
}

//----------------------------------------------------------------------------

bool MainFrame::saveCurrentDatabaseToFile(const QString& dstFileName)
{
  // Precondition:
  // All checks for valid filenames, overwriting of files etc. have to
  // be done before calling this function.
  //
  // This function unconditionally writes to the destination file, whether
  // it exists or not.

  if (currentDb == nullptr) return false;

  // write the database to the file
  int dbErr;
  bool isOkay = currentDb->backupToFile(dstFileName.toUtf8().constData(), &dbErr);

  // handle erros
  QString msg;
  if (dbErr == SQLITE_ERROR)
  {
    msg = tr("Could not write to the destination file:\n\n");
    msg += dstFileName + "\n\n";
    msg += tr("The tournament has not been saved.");
  }
  else if (dbErr != SQLITE_OK)
  {
    msg = tr("A database error occured while saving.\n\n");
    msg += tr("Internal hint: SQLite error code = %1");
    msg = msg.arg(dbErr);
  }
  if (!(msg.isEmpty()))
  {
    QMessageBox::warning(this, tr("Saving failed"), msg);
  }

  return isOkay;
}

//----------------------------------------------------------------------------

bool MainFrame::execCmdSave()
{
  if (currentDb == nullptr) return false;

  if (currentDatabaseFileName.isEmpty())
  {
    return execCmdSaveAs();
  }

  bool isOkay = saveCurrentDatabaseToFile(currentDatabaseFileName);
  if (isOkay)
  {
    currentDb->resetDirtyFlag();

    // reset the autosave state machine and status indication
    lastAutosaveDirtyCounterValue = currentDb->getDirtyCounter();
    onAutosaveTimerElapsed();
  }

  return isOkay;
}

//----------------------------------------------------------------------------

bool MainFrame::execCmdSaveAs()
{
  if (currentDb == nullptr) return false;

  QString dstFileName = askForTournamentFileName(tr("Save tournament as"));
  if (dstFileName.isEmpty()) return false;  // user abort counts as "failed"

  bool isOkay = saveCurrentDatabaseToFile(dstFileName);

  if (isOkay)
  {
    currentDb->resetDirtyFlag();
    currentDatabaseFileName = dstFileName;
    ui.actionCreate_baseline->setEnabled(true);

    // reset the autosave state machine and status indication
    lastAutosaveDirtyCounterValue = currentDb->getDirtyCounter();
    onAutosaveTimerElapsed();

    // show the file name in the window title
    updateWindowTitle();
  }

  return isOkay;
}

//----------------------------------------------------------------------------

QString MainFrame::askForTournamentFileName(const QString& dlgTitle)
{
  // ask for the file name
  QFileDialog fDlg{this};
  fDlg.setAcceptMode(QFileDialog::AcceptSave);
  fDlg.setNameFilter(tr("QTournament Files (*.tdb)"));
  fDlg.setWindowTitle(dlgTitle);
  int result = fDlg.exec();

  if (result != QDialog::Accepted)
  {
    return "";
  }

  // get the filename and fix the extension, if necessary
  QString filename = fDlg.selectedFiles().at(0);
  QString ext = filename.right(4).toLower();
  if (ext != ".tdb") filename += ".tdb";

  return filename;
}

//----------------------------------------------------------------------------

void MainFrame::updateWindowTitle()
{
  QString title = "QTournament";

  if (currentDb != nullptr)
  {
    // determine the tournament title
    auto cfg = KeyValueTab::getTab(currentDb.get(), TAB_CFG);
    QString tnmtTitle = QString(cfg.operator[](CFG_KEY_TNMT_NAME).data());
    title += " - " + tnmtTitle + " (%1)";

    // insert the current filename, if any
    title = currentDatabaseFileName.isEmpty() ? title.arg(tr("unsaved")) : title.arg(currentDatabaseFileName);

    // append an asterisk to the windows title if the
    // database has changed since the last saving
    if (currentDb->isDirty()) title += " *";
  }

  setWindowTitle(title);
}

//----------------------------------------------------------------------------

void MainFrame::updateOnlineMenu()
{
  if (currentDb == nullptr)
  {
    ui.menuOnline->setEnabled(false);
    return;
  }

  OnlineMngr* om = currentDb->getOnlineManager();
  bool hasReg = om->hasRegistrationSubmitted();
  SyncState st = om->getSyncState();


  // disable registration if we've already registered once
  ui.actionRegister->setEnabled(!hasReg);

  // if we've not registered yet, there's no point in
  // setting / changing the password
  // ==> online enable the password item for registered tournaments
  ui.actionSet_Change_Password->setEnabled(hasReg);

  // deletion of the tournament only if we've registered before
  ui.actionDelete_from_Server->setEnabled(hasReg);

  // connect only if disconnected
  ui.actionConnect->setEnabled(hasReg && !(st.hasSession()));

  // disconnect only if connected
  ui.actionDisconnect->setEnabled(hasReg && st.hasSession());

  // change server settings only when disconnected
  ui.actionConnection_Settings->setEnabled(!(st.hasSession()));
}

//----------------------------------------------------------------------------

void MainFrame::setupTestScenario(int scenarioID)
{
  if ((scenarioID < 0) || (scenarioID > 8))
  {
    QMessageBox::critical(this, "Setup Test Scenario", "The scenario ID " + QString::number(scenarioID) + " is invalid!");
  }
  
  // shutdown whatever is open right now
  closeCurrentTournament();
  
  // prepare a brand-new scenario
  TournamentSettings cfg;
  cfg.organizingClub = "SV Whatever";
  cfg.tournamentName = "World Championship";
  cfg.useTeams = true;
  cfg.refereeMode = REFEREE_MODE::NONE;
  currentDb = TournamentDB::createNew(testFileName, cfg);
  distributeCurrentDatabasePointerToWidgets();
  
  // the empty scenario
  if (scenarioID == 0)
  {
  }
  
  // a scenario with just a few teams already existing
  TeamMngr tmngr{currentDb.get()};
  PlayerMngr pmngr{currentDb.get()};
  CatMngr cmngr{currentDb.get()};
  MatchMngr mm{currentDb.get()};
  CourtMngr courtm{currentDb.get()};
  
  auto scenario01 = [&]()
  {
    tmngr.createNewTeam("Team 1");
    tmngr.createNewTeam("Team 2");
    
    pmngr.createNewPlayer("First1", "Last1", M, "Team 1");
    pmngr.createNewPlayer("First2", "Last2", F, "Team 1");
    pmngr.createNewPlayer("First3", "Last3", M, "Team 1");
    pmngr.createNewPlayer("First4", "Last4", F, "Team 1");
    pmngr.createNewPlayer("First5", "Last5", M, "Team 2");
    pmngr.createNewPlayer("First6", "Last6", F, "Team 2");

    // create one category of every kind
    cmngr.createNewCategory("MS");
    Category ms = cmngr.getCategory("MS");
    ms.setMatchType(SINGLES);
    ms.setSex(M);

    cmngr.createNewCategory("MD");
    Category md = cmngr.getCategory("MD");
    md.setMatchType(DOUBLES);
    md.setSex(M);

    cmngr.createNewCategory("LS");
    Category ls = cmngr.getCategory("LS");
    ls.setMatchType(SINGLES);
    ls.setSex(F);

    cmngr.createNewCategory("LD");
    Category ld = cmngr.getCategory("LD");
    ld.setMatchType(DOUBLES);
    ld.setSex(F);

    cmngr.createNewCategory("MX");
    Category mx = cmngr.getCategory("MX");
    mx.setMatchType(MIXED);
    mx.setSex(M); // shouldn't matter at all
  };
  
  auto scenario02 = [&]()
  {
    scenario01();
    Category md = cmngr.getCategory("MD");
    Category ms = cmngr.getCategory("MS");
    Category mx = cmngr.getCategory("MX");
    
    Player m1 = pmngr.getPlayer(1);
    Player m2 = pmngr.getPlayer(3);
    Player m3 = pmngr.getPlayer(5);
    Player l1 = pmngr.getPlayer(2);
    Player l2 = pmngr.getPlayer(4);
    Player l3 = pmngr.getPlayer(6);
    
    cmngr.addPlayerToCategory(m1, md);
    cmngr.addPlayerToCategory(m2, md);
    
    cmngr.addPlayerToCategory(m1, ms);
    cmngr.addPlayerToCategory(m2, ms);
    cmngr.addPlayerToCategory(m3, ms);
    
    cmngr.addPlayerToCategory(m1, mx);
    cmngr.addPlayerToCategory(m2, mx);
    cmngr.addPlayerToCategory(m3, mx);
    cmngr.addPlayerToCategory(l1, mx);
    cmngr.addPlayerToCategory(l2, mx);
    cmngr.addPlayerToCategory(l3, mx);
  };
  
  // a scenario with a lot of participants, including a group of
  // forty players in one category
  auto scenario03 = [&]()
  {
    scenario02();
    tmngr.createNewTeam("Massive");
    Category ls = cmngr.getCategory("LS");
    
    for (int i=0; i < 250; i++)
    {
      QString lastName = "Massive" + QString::number(i);
      pmngr.createNewPlayer("Lady", lastName, F, "Massive");
      Player p = pmngr.getPlayer(i + 7);   // the first six IDs are already used by previous ini-functions above
      if (i < 40) ls.addPlayer(p);
      //if (i < 17) ls.addPlayer(p);
    }
    
    // create and set a valid group configuration for LS
    GroupDef d = GroupDef(5, 8);
    GroupDefList gdl;
    gdl.append(d);
    KO_Config cfg(QUARTER, false, gdl);
    ls.setParameter(GROUP_CONFIG, cfg.toString());
  };

  // extend scenario 3 to already start category "LS"
  // and add a few players to LD and start this category, too
  auto scenario04 = [&]()
  {
    scenario03();
    Category ls = cmngr.getCategory("LS");

    // run the category
    std::unique_ptr<Category> specialCat = ls.convertToSpecializedObject();
    ERR e = cmngr.freezeConfig(ls);
    assert(e == OK);

    // fake a list of player-pair-lists for the group assignments
    std::vector<PlayerPairList> ppListList;
    for (int grpNum=0; grpNum < 8; ++grpNum)
    {
        PlayerPairList thisGroup;
        for (int pNum=0; pNum < 5; ++pNum)
        {
            int playerId = (grpNum * 5) + pNum + 7;  // the first six IDs are already in use; see above

            Player p = pmngr.getPlayer(playerId);
            PlayerPair pp(p, (playerId-6));   // PlayerPairID starts at 1
            thisGroup.push_back(pp);
        }
        ppListList.push_back(thisGroup);
    }

    // make sure the faked group assignment is valid
    e = specialCat->canApplyGroupAssignment(ppListList);
    assert(e == OK);

    // prepare an empty list for the (not required) initial ranking
    PlayerPairList initialRanking;

    // actually run the category
    e = cmngr.startCategory(ls, ppListList, initialRanking);
    assert(e == OK);

    // we're done with LS here...

    // add 16 players to LD
    Category ld = cmngr.getCategory("LD");
    for (int id=100; id < 116; ++id)
    {
      e = ld.addPlayer(pmngr.getPlayer(id));
      assert(e == OK);
    }

    // generate eight player pairs
    for (int id=100; id < 116; id+=2)
    {
      Player p1 = pmngr.getPlayer(id);
      Player p2 = pmngr.getPlayer(id+1);
      e = cmngr.pairPlayers(ld, p1, p2);
      assert(e == OK);
    }

    // set the config to be 2 groups of 4 players each
    // and that KOs start with the final
    GroupDef d = GroupDef(4, 2);
    GroupDefList gdl;
    gdl.append(d);
    KO_Config cfg(FINAL, false, gdl);
    assert(ld.setParameter(GROUP_CONFIG, cfg.toString()) == true);

    // freeze
    specialCat = ld.convertToSpecializedObject();
    e = cmngr.freezeConfig(ld);
    assert(e == OK);

    // fake a list of player-pair-lists for the group assignments
    ppListList.clear();
    PlayerPairList allPairsInCat = ld.getPlayerPairs();
    for (int grpNum=0; grpNum < 2; ++grpNum)
    {
        PlayerPairList thisGroup;
        for (int pNum=0; pNum < 4; ++pNum)
        {
            thisGroup.push_back(allPairsInCat.at(grpNum * 4 + pNum));
        }
        ppListList.push_back(thisGroup);
    }

    // make sure the faked group assignment is valid
    e = specialCat->canApplyGroupAssignment(ppListList);
    assert(e == OK);

    // actually run the category
    e = cmngr.startCategory(ld, ppListList, initialRanking);  // "initialRanking" is reused from above
    assert(e == OK);
  };

  // extend scenario 4 to already stage and schedule a few match groups
  // in category "LS" and "LD"
  // Additionally, we add 4 courts to the tournament
  auto scenario05 = [&]()
  {
    scenario04();
    Category ls = cmngr.getCategory("LS");

    ERR e;
    auto mg = mm.getMatchGroup(ls, 1, 3, &e);  // round 1, players group 3
    assert(e == OK);
    mm.stageMatchGroup(*mg);
    mm.scheduleAllStagedMatchGroups();

    Category ld = cmngr.getCategory("LD");
    mg = mm.getMatchGroup(ld, 1, 1, &e);  // round 1, players group 1
    assert(e == OK);
    mm.stageMatchGroup(*mg);
    mg = mm.getMatchGroup(ld, 1, 2, &e);  // round 1, players group 2
    assert(e == OK);
    mm.stageMatchGroup(*mg);
    mg = mm.getMatchGroup(ld, 2, 1, &e);  // round 2, players group 1
    assert(e == OK);
    mm.stageMatchGroup(*mg);
    mm.scheduleAllStagedMatchGroups();

    // add four courts
    for (int i=1; i <= 4; ++i)
    {
      courtm.createNewCourt(i, "XX", &e);
      assert(e == OK);
    }
  };

  // extend scenario 5 to already play all matches in the round-robin rounds
  // of category "LS" and "LD"
  auto scenario06 = [&]()
  {
    scenario05();
    Category ls = cmngr.getCategory("LS");
    Category ld = cmngr.getCategory("LD");

    // stage and schedule all matches in round 1
    // of LS and LD
    bool canStageMatchGroups = true;
    while (canStageMatchGroups)
    {
      canStageMatchGroups = false;
      for (MatchGroup mg : mm.getMatchGroupsForCat(ls))
      {
        if (mg.getState() != STAT_MG_IDLE) continue;
        if (mm.canStageMatchGroup(mg) != OK) continue;
        mm.stageMatchGroup(mg);
        canStageMatchGroups = true;
      }
      for (MatchGroup mg : mm.getMatchGroupsForCat(ld))
      {
        if (mg.getState() != STAT_MG_IDLE) continue;
        if (mm.canStageMatchGroup(mg) != OK) continue;
        mm.stageMatchGroup(mg);
        canStageMatchGroups = true;
      }
    }
    mm.scheduleAllStagedMatchGroups();

    // play all scheduled matches
    QDateTime curDateTime = QDateTime::currentDateTimeUtc();
    uint epochSecs = curDateTime.toTime_t();
    DbTab* matchTab = currentDb->getTab(TAB_MATCH);
    while (true)
    {
      int nextMacthId;
      int nextCourtId;
      mm.getNextViableMatchCourtPair(&nextMacthId, &nextCourtId);
      if (nextMacthId <= 0) break;

      auto nextMatch = mm.getMatch(nextMacthId);
      if (nextMatch == nullptr) break;
      auto nextCourt = courtm.getCourtById(nextCourtId);
      if (nextCourt == nullptr) break;

      if (mm.assignMatchToCourt(*nextMatch, *nextCourt) != OK) break;
      auto score = MatchScore::genRandomScore();
      mm.setMatchScoreAndFinalizeMatch(*nextMatch, *score);

      // overwrite the match finish time to get a fake match duration
      // the duration is at least 15 minutes and max 25 minutes
      int fakeDuration = 15 * 60  +  10 * 60 * (qrand() / (RAND_MAX * 1.0));
      TabRow maRow = matchTab->operator [](nextMacthId);
      maRow.update(MA_FINISH_TIME, to_string(epochSecs + fakeDuration));
    }
  };

  // extend scenario 3, set the LS match system to "single elimination",
  // run the category, stage the first three rounds, play the first
  // round and start the second
  auto scenario07 = [&]()
  {
    scenario03();
    Category ls = cmngr.getCategory("LS");

    // set the match system to Single Elimination
    ERR e = ls.setMatchSystem(SINGLE_ELIM) ;
    assert(e == OK);

    // run the category
    std::unique_ptr<Category> specialCat = ls.convertToSpecializedObject();
    e = cmngr.freezeConfig(ls);
    assert(e == OK);

    // prepare an empty list for the not-required initial group assignment
    std::vector<PlayerPairList> ppListList;

    // prepare a list for the (faked) initial ranking
    PlayerPairList initialRanking = ls.getPlayerPairs();

    // actually run the category
    e = cmngr.startCategory(ls, ppListList, initialRanking);
    assert(e == OK);

    // stage all match groups
    auto mg = mm.getMatchGroup(ls, 1, GROUP_NUM__ITERATION, &e);  // round 1
    assert(e == OK);
    mm.stageMatchGroup(*mg);
    mg = mm.getMatchGroup(ls, 2, GROUP_NUM__ITERATION, &e);  // round 2
    assert(e == OK);
    mm.stageMatchGroup(*mg);
    mg = mm.getMatchGroup(ls, 3, GROUP_NUM__L16, &e);  // round 3
    assert(e == OK);
    mm.stageMatchGroup(*mg);
    mg = mm.getMatchGroup(ls, 4, GROUP_NUM__QUARTERFINAL, &e);  // round 4
    assert(e == OK);
    mm.stageMatchGroup(*mg);
    mg = mm.getMatchGroup(ls, 5, GROUP_NUM__SEMIFINAL, &e);  // round 5
    assert(e == OK);
    mm.stageMatchGroup(*mg);
    mg = mm.getMatchGroup(ls, 6, GROUP_NUM__FINAL, &e);  // round 6
    assert(e == OK);
    mm.stageMatchGroup(*mg);
    mm.scheduleAllStagedMatchGroups();

    // add four courts
    for (int i=1; i <= 4; ++i)
    {
      courtm.createNewCourt(i, "XX", &e);
      assert(e == OK);
    }

    // play all matches
    while (true)
    {
      int nextMacthId;
      int nextCourtId;
      mm.getNextViableMatchCourtPair(&nextMacthId, &nextCourtId);
      if (nextMacthId <= 0) break;

      auto nextMatch = mm.getMatch(nextMacthId);
      if (nextMatch == nullptr) break;
      //if (nextMatch->getMatchGroup().getRound() == 2) break;
      auto nextCourt = courtm.getCourtById(nextCourtId);
      if (nextCourt == nullptr) break;

      if (mm.assignMatchToCourt(*nextMatch, *nextCourt) != OK) break;
      auto score = MatchScore::genRandomScore();
      mm.setMatchScoreAndFinalizeMatch(*nextMatch, *score);
    }
  };

  // a scenario with up to 10 players in a ranking1-bracket
  auto scenario08 = [&]()
  {
    scenario02();
    tmngr.createNewTeam("Ranking Team");
    Category ls = cmngr.getCategory("LS");
    Category ld = cmngr.getCategory("LD");

    int evenPlayerId = -1;
    for (int i=0; i < 28; i++)   // must be an even number, for doubles!
    {
      QString lastName = "Ranking" + QString::number(i+1);
      pmngr.createNewPlayer("Lady", lastName, F, "Ranking Team");
      Player p = pmngr.getPlayer(i + 7);   // the first six IDs are already used by previous ini-functions above
      ls.addPlayer(p);
      ld.addPlayer(p);

      // pair every two players
      if ((i % 2) == 0)
      {
        evenPlayerId = p.getId();
      } else {
        Player evenPlayer = pmngr.getPlayer(evenPlayerId);
        cmngr.pairPlayers(ld, p, evenPlayer);
      }
    }

    ls.setMatchSystem(MATCH_SYSTEM::RANKING);
    ld.setMatchSystem(MATCH_SYSTEM::RANKING);

    // freeze the LS category
    ERR e = cmngr.freezeConfig(ls);
    assert(e == OK);

    // prepare an empty list for the not-required initial group assignment
    std::vector<PlayerPairList> ppListList;

    // prepare a list for the (faked) initial ranking
    PlayerPairList initialRanking = ls.getPlayerPairs();

    // actually run the category
    e = cmngr.startCategory(ls, ppListList, initialRanking);
    assert(e == OK);

    // freeze the LD category
    e = cmngr.freezeConfig(ld);
    assert(e == OK);

    // prepare a list for the (faked) initial ranking
    initialRanking = ld.getPlayerPairs();

    // actually run the category
    e = cmngr.startCategory(ld, ppListList, initialRanking);
    assert(e == OK);
  };

  switch (scenarioID)
  {
  case 1:
    scenario01();
    break;
  case 2:
    scenario02();
    break;
  case 3:
    scenario03();
    break;
  case 4:
    scenario04();
    break;
  case 5:
    scenario05();
    break;
  case 6:
    scenario06();
    break;
  case 7:
    scenario07();
    break;
  case 8:
    scenario08();
    break;
  }

  enableControls(true);

  return;
}

//----------------------------------------------------------------------------

void MainFrame::setupEmptyScenario()
{
  setupTestScenario(0);
}

//----------------------------------------------------------------------------

void MainFrame::setupScenario01()
{
  setupTestScenario(1);
}

//----------------------------------------------------------------------------

void MainFrame::setupScenario02()
{
  setupTestScenario(2);
}

//----------------------------------------------------------------------------

void MainFrame::setupScenario03()
{
  setupTestScenario(3);
}

//----------------------------------------------------------------------------

void MainFrame::setupScenario04()
{
  setupTestScenario(4);
}

//----------------------------------------------------------------------------

void MainFrame::setupScenario05()
{
  setupTestScenario(5);
}

//----------------------------------------------------------------------------

void MainFrame::setupScenario06()
{
  /*
   * For performance tests
   *
  QDateTime startTime = QDateTime::currentDateTime();
  for (int i=0; i < 5; ++i)
  {
    setupTestScenario(6);
  }
  QDateTime endTime = QDateTime::currentDateTime();
  auto runtime = startTime.msecsTo(endTime);
  int secs = runtime / 1000;
  int msecs = runtime % 1000;
  QString msg = "%1,%2 secs";
  msg = msg.arg(secs).arg(msecs);
  QMessageBox::information(this, "sdlfkjsdf", msg);
  */

  setupTestScenario(6);
}

//----------------------------------------------------------------------------

void MainFrame::setupScenario07()
{
  setupTestScenario(7);
}

//----------------------------------------------------------------------------

void MainFrame::setupScenario08()
{
  setupTestScenario(8);
}

//----------------------------------------------------------------------------

void MainFrame::closeEvent(QCloseEvent* ev)
{
  if (currentDb == nullptr)
  {
    ev->accept();
  } else {
    bool isOkay = closeCurrentTournament();

    if (!isOkay) ev->ignore();
    else ev->accept();
  }
}

//----------------------------------------------------------------------------

void MainFrame::onCurrentTabChanged(int newCurrentTab)
{
  if (newCurrentTab < 0) return;

  // get the newly selected tab widget
  auto selectedTabWidget = ui.mainTab->currentWidget();
  if (selectedTabWidget == nullptr) return;

  // check if the new tab is the reports tab
  if (selectedTabWidget == ui.tabReports)
  {
    ui.tabReports->onReloadRequested();
  }
}

//----------------------------------------------------------------------------

void MainFrame::onNewExternalPlayerDatabase()
{
  // ask for the file name
  QFileDialog fDlg{this};
  fDlg.setAcceptMode(QFileDialog::AcceptSave);
  fDlg.setNameFilter(tr("QTournament Player Database (*.pdb)"));
  int result = fDlg.exec();

  if (result != QDialog::Accepted)
  {
    return;
  }

  // get the filename and fix the extension, if necessary
  QString filename = fDlg.selectedFiles().at(0);
  QString ext = filename.right(4).toLower();
  if (ext != ".pdb") filename += ".pdb";

  // if the file exists, delete it.
  // the user has consented to the deletion in the
  // dialog
  if (QFile::exists(filename))
  {
    bool removeResult =  QFile::remove(filename);

    if (!removeResult)
    {
      QMessageBox::warning(this, tr("New player database"), tr("Could not delete ") + filename + tr(", no new database created."));
      return;
    }
  }

  // actually create and actiate the new database
  PlayerMngr pm{currentDb.get()};
  ERR e = pm.setExternalPlayerDatabase(filename, true);
  if (e != OK)
  {
    QMessageBox::warning(this, tr("New player database"), tr("Could not create ") + filename);
    return;
  }
}

//----------------------------------------------------------------------------

void MainFrame::onSelectExternalPlayerDatabase()
{
  // ask for the file name
  QFileDialog fDlg{this};
  fDlg.setAcceptMode(QFileDialog::AcceptOpen);
  fDlg.setFileMode(QFileDialog::ExistingFile);
  fDlg.setNameFilter(tr("QTournament Player Database (*.pdb)"));
  int result = fDlg.exec();

  if (result != QDialog::Accepted)
  {
    return;
  }

  // get the filename
  QString filename = fDlg.selectedFiles().at(0);

  // open and activate the database
  PlayerMngr pm{currentDb.get()};
  ERR e = pm.setExternalPlayerDatabase(filename, false);
  if (e != OK)
  {
    QString msg = tr("Could not open ") + filename + "\n\n";
    if (pm.hasExternalPlayerDatabaseOpen())
    {
      msg += "Database not changed.";
    } else  {
      msg += "No player database active.";
    }
    QMessageBox::warning(this, tr("Select player database"), msg);
    return;
  }
}

//----------------------------------------------------------------------------

void MainFrame::onInfoMenuTriggered()
{
  QString msg = tr("This is QTournament version %1.<br>");
  msg += tr("© Volker Knollmann, 2014 - 2017<br><br>");
  msg += tr("This program is free software: you can redistribute it and/or modify ");
  msg += tr("it under the terms of the GNU General Public License as published by ");
  msg += tr("the Free Software Foundation, either version 3 of the License, or ");
  msg += tr("any later version.<br>");

  msg += tr("This program is distributed in the hope that it will be useful, ");
  msg += tr("but WITHOUT ANY WARRANTY; without even the implied warranty of ");
  msg += tr("MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the ");
  msg += tr("GNU General Public License for more details.<br>");

  msg += tr("You should have received a copy of the GNU General Public License ");
  msg += tr("along with this program. If not, see <a href='http://www.gnu.org/licenses/'>http://www.gnu.org/licenses/</a>.<br><br>");

  msg += tr("The source code for this program is hosted on Github:<br>");
  msg += "<a href='https://github.com/Foorgol/QTournament'>https://github.com/Foorgol/QTournament</a><br><br>";

  msg += tr("For more information please visit:<br>");
  msg += "<a href='http://tournament.de'>http://tournament.de</a> (German)<br>";
  msg += "<a href='http://tournament.de/en'>http://tournament.de/en</a> (English)<br><br>";
  msg += "or send an email to <a href='mailto:info@qtournament.de'>info@qtournament.de</a>";
  msg = msg.arg(PRG_VERSION_STRING);

  QMessageBox msgBox{this};
  msgBox.setWindowTitle(tr("About QTournament"));
  msgBox.setTextFormat(Qt::RichText);
  msgBox.setText(msg);
  msgBox.exec();
}

//----------------------------------------------------------------------------

void MainFrame::onEditTournamentSettings()
{
  if (currentDb == nullptr) return;

  // show a dialog for setting the tournament parameters
  DlgTournamentSettings dlg{currentDb.get(), this};
  dlg.setModal(true);
  int result = dlg.exec();

  if (result != QDialog::Accepted)
  {
    return;
  }

  // get the new settings
  std::unique_ptr<TournamentSettings> newSettings = dlg.getTournamentSettings();
  if (newSettings == nullptr)
  {
    QMessageBox::warning(this, tr("Edit tournament settings"),
                         tr("The tournament settings could not be updated."));
    return;
  }

  // check for changes and apply them.
  //
  // start with the tournament organizer
  auto cfg = SqliteOverlay::KeyValueTab::getTab(currentDb.get(), TAB_CFG, false);
  string tmp = (*cfg)[CFG_KEY_TNMT_ORGA];
  QString oldTnmtOrga = QString::fromUtf8(tmp.c_str());
  if (oldTnmtOrga != newSettings->organizingClub)
  {
    tmp = (newSettings->organizingClub).toUtf8().constData();
    cfg.set(CFG_KEY_TNMT_ORGA, tmp);
  }

  // the tournament name
  tmp = (*cfg)[CFG_KEY_TNMT_NAME];
  QString oldTnmtName = QString::fromUtf8(tmp.c_str());
  if (oldTnmtName != newSettings->tournamentName)
  {
    tmp = (newSettings->tournamentName).toUtf8().constData();
    cfg.set(CFG_KEY_TNMT_NAME, tmp);

    // refresh the window title to show the new name
    updateWindowTitle();
  }

  // the umpire mode
  int oldRefereeModeId = cfg.getInt(CFG_KEY_DEFAULT_REFEREE_MODE);
  REFEREE_MODE oldRefereeMode = static_cast<REFEREE_MODE>(oldRefereeModeId);
  if (oldRefereeMode != newSettings->refereeMode)
  {
    cfg.set(CFG_KEY_DEFAULT_REFEREE_MODE, static_cast<int>(newSettings->refereeMode));
    ui.tabSchedule->updateRefereeColumn();
  }
}

//----------------------------------------------------------------------------

void MainFrame::onSetPassword()
{
  if (currentDb == nullptr) return;

  cmdSetOrChangePassword cmd{this, currentDb.get()};
  cmd.exec();
  updateOnlineMenu();
}

//----------------------------------------------------------------------------

void MainFrame::onRegisterTournament()
{
  if (currentDb == nullptr) return;

  // the online registration is a complex task
  // so I've moved it to a separate file

  cmdOnlineRegistration cmd{this, currentDb.get()};
  cmd.exec();
  updateOnlineMenu();
}

//----------------------------------------------------------------------------

void MainFrame::onStartSession()
{
  if (currentDb == nullptr) return;

  cmdStartOnlineSession cmd{this, currentDb.get()};
  cmd.exec();
  updateOnlineMenu();
}

//----------------------------------------------------------------------------

void MainFrame::onTerminateSession()
{
  OnlineMngr* om = currentDb->getOnlineManager();
  bool isOkay = om->disconnect();

  if (isOkay)
  {
    QMessageBox::information(this, tr("Server Disconnect"), tr("You are now disconnected from the server"));
  } else {
    QString msg = tr("An error occurred while disconnecting. Nevertheless that, the syncing with the server has now been stopped.");
    QMessageBox::warning(this, tr("Server Disconnect"), msg);
  }

  updateOnlineMenu();
}

//----------------------------------------------------------------------------

void MainFrame::onDeleteFromServer()
{
  if (currentDb == nullptr) return;

  cmdDeleteFromServer cmd{this, currentDb.get()};
  cmd.exec();
  updateOnlineMenu();
}

//----------------------------------------------------------------------------

void MainFrame::onEditConnectionSettings()
{
  if (currentDb == nullptr) return;

  cmdConnectionSetting cmd{this, currentDb.get()};
  cmd.exec();
  updateOnlineMenu();
}

//----------------------------------------------------------------------------

void MainFrame::onToggleTestMenuVisibility()
{
  ui.menubar->clear();
  ui.menubar->addMenu(ui.menuTournament);
  ui.menubar->addMenu(ui.menuOnline);
  ui.menubar->addMenu(ui.menuAbout_QTournament);

  if (!isTestMenuVisible)
  {
    ui.menubar->addMenu(ui.menuTesting);
  }

  isTestMenuVisible = !isTestMenuVisible;
}

//----------------------------------------------------------------------------

void MainFrame::onDirtyFlagPollTimerElapsed()
{
  if (currentDb == nullptr) return;

  if (currentDb->isDirty() != lastDirtyState)
  {
    lastDirtyState = !lastDirtyState;
    updateWindowTitle();
  }
}

//----------------------------------------------------------------------------

void MainFrame::onAutosaveTimerElapsed()
{
  if (currentDb == nullptr)
  {
    lastAutosaveTimeStatusLabel->clear();
    return;
  }

  if (!(currentDb->isDirty()))
  {
    lastAutosaveTimeStatusLabel->clear();
    return;
  }

  QString msg = tr("Last autosave: ");
  if (currentDatabaseFileName.isEmpty())
  {
    lastAutosaveTimeStatusLabel->setText(msg + tr("not yet possible"));
    return;
  }

  // do we need an autosave?
  if (currentDb->getDirtyCounter() != lastAutosaveDirtyCounterValue)
  {
    QString fname = currentDatabaseFileName + ".autosave";
    bool isOkay = saveCurrentDatabaseToFile(fname);

    if (isOkay)
    {
      msg += QTime::currentTime().toString("HH:mm:ss");
      lastAutosaveDirtyCounterValue = currentDb->getDirtyCounter();
    } else {
      msg += tr("failed");
    }
    lastAutosaveTimeStatusLabel->setText(msg);
  }
}

//----------------------------------------------------------------------------

void MainFrame::onServerSyncTimerElapsed()
{
  if (currentDb == nullptr)
  {
    syncStatLabel->clear();
    btnPingTest->setVisible(false);
    return;
  }

  // retrieve the status from the online manager
  OnlineMngr* om = currentDb->getOnlineManager();

  // show nothing if we've never registered the tournament
  if (!(om->hasRegistrationSubmitted()))
  {
    syncStatLabel->clear();
    btnPingTest->setVisible(false);
    return;
  } else {
    btnPingTest->setVisible(true);
  }

  // get the current sync state
  SyncState st = om->getSyncState();

  // if we're offline, everything's easy
  if (!(st.hasSession()))
  {
    syncStatLabel->setText(tr("<span style='color: red; font-weight: bold;'>Offline</span>"));
    return;
  }

  // we're online, thus we'll first update the labels
  // with the current status
  QString msg = tr("<span style='color: green; font-weight: bold;'>Online</span>");
  msg += tr(", %1 syncs committed, %2 changes pending");
  msg = msg.arg(st.partialSyncCounter);
  msg = msg.arg(currentDb->getChangeLogLength());

  // attach the last request time, if available
  int dt = om->getLastReqTime_ms();
  if (dt > 0)
  {
    msg += tr(" ; the last request took %1 ms");
    msg = msg.arg(dt);
  }

  // set the label and we're done with the cosmetics
  syncStatLabel->setText(msg);

  // next we check if the OnlineMngr wants to
  // do a sync call
  if (!(om->wantsToSync())) return;

  //
  // yes, a sync is necessary
  //

  QString errMsgFromServer;
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  OnlineError err = om->doPartialSync(errMsgFromServer);
  QApplication::restoreOverrideCursor();

  // maybe the database is locked by a different process,
  // e.g. an open dialog
  if (err == OnlineError::LocalDatabaseBusy) return; // try again later

  // handle connection / transport errors
  msg.clear();
  if ((err != OnlineError::Okay) && (err != OnlineError::TransportOkay_AppError))
  {
    switch (err)
    {
    case OnlineError::Timeout:
      msg = tr("The server is currently not available.\n\n");
      msg += tr("Maybe the server is temporarily down or you are offline.");
      break;

    case OnlineError::BadRequest:
      msg = tr("The server did not accept our sync request (400, BadRequest).");
      break;

    default:
      msg = tr("Sync failed due to an unspecified network or server error!");
    }
  }

  if (err == OnlineError::TransportOkay_AppError)
  {
    if (errMsgFromServer == "DatabaseError")
    {
      msg = tr("Syncing failed because of a server-side database error.");
    }
    if (errMsgFromServer == "CSVError")
    {
      msg = tr("Syncing failed because the server couldn't digest our CSV data!\n");
      msg += tr("Strange, this shouldn't happen...");
    }
    if (msg.isEmpty())
    {
      msg = tr("Sync failed because of an unexpected server error.\n");
    }
  }

  if (!(msg.isEmpty()))
  {
    om->disconnect();
    msg += "\n\nThe server connection has been shut-down. Try to connect again later. Good luck!";
    QMessageBox::warning(this, tr("Server sync failed"), msg);
  }
}

//----------------------------------------------------------------------------

void MainFrame::onBtnPingTestClicked()
{
  if (currentDb == nullptr) return;
  OnlineMngr* om = currentDb->getOnlineManager();

  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  int t = om->ping();
  QApplication::restoreOverrideCursor();

  QString msg;
  if (t > 0)
  {
    msg = tr("The server responded within %1 ms");
    msg = msg.arg(t);
    QMessageBox::information(this, tr("Server Ping Test"), msg);
  } else {
    msg = tr("The server is not reachable");
    QMessageBox::warning(this, tr("Server Ping Test"), msg);
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


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


