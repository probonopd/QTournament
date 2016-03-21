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

#include <stdexcept>
#include <QMessageBox>
#include <QFileDialog>
#include <QFile>

#include "MainFrame.h"
#include "Tournament.h"
#include "ui/DlgTournamentSettings.h"

using namespace QTournament;

//----------------------------------------------------------------------------

MainFrame* MainFrame::mainFramePointer = nullptr;

//----------------------------------------------------------------------------

MainFrame::MainFrame()
{
  if (MainFrame::mainFramePointer != nullptr)
  {
    throw std::runtime_error("Only one MainFrame instance is allowed!!");
  }
  MainFrame::mainFramePointer = this;
  
  tnmt = nullptr;
  
  ui.setupUi(this);
  showMaximized();
  enableControls(false);
  
  testFileName = QDir().absoluteFilePath("tournamentTestFile.tdb");

  // prepare an action to toggle the test-menu's visibility
  scToggleTestMenuVisibility = new QShortcut(QKeySequence(Qt::CTRL + Qt::ALT + Qt::Key_T), this);
  scToggleTestMenuVisibility->setContext(Qt::ApplicationShortcut);
  connect(scToggleTestMenuVisibility, SIGNAL(activated()), this, SLOT(onToggleTestMenuVisibility()));
  isTestMenuVisible = true;
  onToggleTestMenuVisibility();
}

//----------------------------------------------------------------------------

MainFrame::~MainFrame()
{
  closeCurrentTournament();

  mainFramePointer = nullptr;
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

  // ask for the file name
  QFileDialog fDlg{this};
  fDlg.setAcceptMode(QFileDialog::AcceptSave);
  fDlg.setNameFilter(tr("QTournament Files (*.tdb)"));
  result = fDlg.exec();

  if (result != QDialog::Accepted)
  {
    return;
  }

  // get the filename and fix the extension, if necessary
  QString filename = fDlg.selectedFiles().at(0);
  QString ext = filename.right(4).toLower();
  if (ext != ".tdb") filename += ".tdb";

  // if the file exists, delete it.
  // the user has consented to the deletion in the
  // dialog
  if (QFile::exists(filename))
  {
    bool removeResult =  QFile::remove(filename);

    if (!removeResult)
    {
      QMessageBox::warning(this, tr("New tournament"), tr("Could not delete ") + filename + tr(", no new tournament created."));
      return;
    }
  }

  ERR err;
  auto newTnmt = Tournament::createNew(filename, *settings, &err);
  if ((err == FILE_ALREADY_EXISTS) || (newTnmt == nullptr))
  {
    // shouldn't happen, because the file has been
    // deleted before (see above)
    QMessageBox::warning(this, tr("New tournament"), tr("Something went wrong; no new tournament created."));
    return;
  }

  // close other possibly open tournaments
  if (tnmt != nullptr)
  {
    closeCurrentTournament();
  }

  // make the new tournament the new active,
  // globally accessible tournament
  tnmt = std::move(newTnmt);
  Tournament::setActiveTournament(tnmt.get());

  emit tournamentOpened(tnmt.get());
  enableControls(true);
  setWindowTitle("QTournament - " + settings->tournamentName + "  (" + filename + ")");
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

  // try to open the tournament file
  ERR err;
  auto newTnmt = Tournament::openExisting(filename, &err);

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
  if ((err != OK) || (newTnmt == nullptr))
  {
    QMessageBox::warning(this, tr("Open tournament"), tr("Something went wrong; no tournament opened."));
    return;
  }

  // do we need to convert this database to a new format?
  TournamentDB* db = newTnmt->getDatabaseHandle();
  if (db->needsConversion())
  {
    QString msg = tr("The file has been created with an older version of QTournament.\n\n");
    msg += tr("The file is not compatible with the current version but it can be updated ");
    msg += tr("to the current version. If you upgrade, you will no longer be able to open ");
    msg += tr("the file with older versions of QTournament.\n\n");
    msg += tr("The conversion cannot be undone.\n\n");
    msg += tr("Do you want to proceed and update the file?");
    int result = QMessageBox::question(this, tr("Convert file format?"), msg);
    if (result != QMessageBox::Yes) return;

    bool conversionOk = db->convertToLatestDatabaseVersion();
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

  // close other possibly open tournaments
  if (tnmt != nullptr)
  {
    closeCurrentTournament();
  }

  // open the tournament
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  tnmt = std::move(newTnmt);
  Tournament::setActiveTournament(tnmt.get());
  emit tournamentOpened(tnmt.get());
  enableControls(true);
  QApplication::restoreOverrideCursor();

  // determine the tournament title
  auto cfg = KeyValueTab::getTab(tnmt->getDatabaseHandle(), TAB_CFG);
  QString tnmtTitle = QString(cfg->operator[](CFG_KEY_TNMT_NAME).data());
  setWindowTitle("QTournament - " + tnmtTitle + "  (" + filename + ")");

  // open the external player database file, if configured
  PlayerMngr* pm = tnmt->getPlayerMngr();
  if (pm->hasExternalPlayerDatabaseConfigured())
  {
    ERR err = pm->openConfiguredExternalPlayerDatabase();

    if (err == OK) return;

    QString msg;
    switch (err)
    {
    case EPD__NOT_FOUND:
      msg = tr("Could not find the player database\n\n");
      msg += pm->getExternalDatabaseName() + "\n\n";
      msg += tr("Please make sure the file exists and is valid.");
      break;

    default:
      msg = tr("The player database\n\n");
      msg += pm->getExternalDatabaseName() + "\n\n";
      msg += tr("is invalid.");
    }

    QMessageBox::warning(this, "Open player database", msg);
  }
}

//----------------------------------------------------------------------------

void MainFrame::enableControls(bool doEnable)
{
  ui.centralwidget->setEnabled(doEnable);
  ui.actionSettings->setEnabled(doEnable);
  ui.menuExternal_player_database->setEnabled(doEnable);
}

//----------------------------------------------------------------------------

void MainFrame::closeCurrentTournament()
{
  // close the tournament
  if (tnmt != nullptr)
  {
    // disconnect from the external player database, if any
    PlayerMngr* pm = tnmt->getPlayerMngr();
    pm->closeExternalPlayerDatabase();

    // this emits a signal to inform everyone that the
    // current tournament is about to die
    tnmt->close();
    
    tnmt.reset();
  }
  
  // delete the test file, if existing
  if (QFile::exists(testFileName))
  {
    QFile::remove(testFileName);
  }
  
  enableControls(false);
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
  tnmt = Tournament::createNew(testFileName, cfg);
  Tournament::setActiveTournament(tnmt.get());
  
  // the empty scenario
  if (scenarioID == 0)
  {
  }
  
  // a scenario with just a few teams already existing
  TeamMngr* tmngr = tnmt->getTeamMngr();
  PlayerMngr* pmngr = tnmt->getPlayerMngr();
  CatMngr* cmngr = tnmt->getCatMngr();
  
  auto scenario01 = [&]()
  {
    tmngr->createNewTeam("Team 1");
    tmngr->createNewTeam("Team 2");
    
    pmngr->createNewPlayer("First1", "Last1", M, "Team 1");
    pmngr->createNewPlayer("First2", "Last2", F, "Team 1");
    pmngr->createNewPlayer("First3", "Last3", M, "Team 1");
    pmngr->createNewPlayer("First4", "Last4", F, "Team 1");
    pmngr->createNewPlayer("First5", "Last5", M, "Team 2");
    pmngr->createNewPlayer("First6", "Last6", F, "Team 2");

    // create one category of every kind
    cmngr->createNewCategory("MS");
    Category ms = cmngr->getCategory("MS");
    ms.setMatchType(SINGLES);
    ms.setSex(M);

    cmngr->createNewCategory("MD");
    Category md = cmngr->getCategory("MD");
    md.setMatchType(DOUBLES);
    md.setSex(M);

    cmngr->createNewCategory("LS");
    Category ls = cmngr->getCategory("LS");
    ls.setMatchType(SINGLES);
    ls.setSex(F);

    cmngr->createNewCategory("LD");
    Category ld = cmngr->getCategory("LD");
    ld.setMatchType(DOUBLES);
    ld.setSex(F);

    cmngr->createNewCategory("MX");
    Category mx = cmngr->getCategory("MX");
    mx.setMatchType(MIXED);
    mx.setSex(M); // shouldn't matter at all
  };
  
  auto scenario02 = [&]()
  {
    scenario01();
    Category md = cmngr->getCategory("MD");
    Category ms = cmngr->getCategory("MS");
    Category mx = cmngr->getCategory("MX");
    
    Player m1 = pmngr->getPlayer(1);
    Player m2 = pmngr->getPlayer(3);
    Player m3 = pmngr->getPlayer(5);
    Player l1 = pmngr->getPlayer(2);
    Player l2 = pmngr->getPlayer(4);
    Player l3 = pmngr->getPlayer(6);
    
    cmngr->addPlayerToCategory(m1, md);
    cmngr->addPlayerToCategory(m2, md);
    
    cmngr->addPlayerToCategory(m1, ms);
    cmngr->addPlayerToCategory(m2, ms);
    cmngr->addPlayerToCategory(m3, ms);
    
    cmngr->addPlayerToCategory(m1, mx);
    cmngr->addPlayerToCategory(m2, mx);
    cmngr->addPlayerToCategory(m3, mx);
    cmngr->addPlayerToCategory(l1, mx);
    cmngr->addPlayerToCategory(l2, mx);
    cmngr->addPlayerToCategory(l3, mx);
  };
  
  // a scenario with a lot of participants, including a group of
  // forty players in one category
  auto scenario03 = [&]()
  {
    scenario02();
    tmngr->createNewTeam("Massive");
    Category ls = cmngr->getCategory("LS");
    
    for (int i=0; i < 250; i++)
    {
      QString lastName = "Massive" + QString::number(i);
      pmngr->createNewPlayer("Lady", lastName, F, "Massive");
      Player p = pmngr->getPlayer(i + 7);   // the first six IDs are already used by previous ini-functions above
      //if (i < 40) ls.addPlayer(p);
      if (i < 17) ls.addPlayer(p);
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
    Category ls = cmngr->getCategory("LS");

    // run the category
    unique_ptr<Category> specialCat = ls.convertToSpecializedObject();
    ERR e = cmngr->freezeConfig(ls);
    assert(e == OK);

    // fake a list of player-pair-lists for the group assignments
    vector<PlayerPairList> ppListList;
    for (int grpNum=0; grpNum < 8; ++grpNum)
    {
        PlayerPairList thisGroup;
        for (int pNum=0; pNum < 5; ++pNum)
        {
            int playerId = (grpNum * 5) + pNum + 7;  // the first six IDs are already in use; see above

            Player p = pmngr->getPlayer(playerId);
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
    e = cmngr->startCategory(ls, ppListList, initialRanking);
    assert(e == OK);

    // we're done with LS here...

    // add 16 players to LD
    Category ld = cmngr->getCategory("LD");
    for (int id=100; id < 116; ++id)
    {
      e = ld.addPlayer(pmngr->getPlayer(id));
      assert(e == OK);
    }

    // generate eight player pairs
    for (int id=100; id < 116; id+=2)
    {
      Player p1 = pmngr->getPlayer(id);
      Player p2 = pmngr->getPlayer(id+1);
      e = cmngr->pairPlayers(ld, p1, p2);
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
    e = cmngr->freezeConfig(ld);
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
    e = cmngr->startCategory(ld, ppListList, initialRanking);  // "initialRanking" is reused from above
    assert(e == OK);
  };

  // extend scenario 4 to already stage and schedule a few match groups
  // in category "LS" and "LD"
  // Additionally, we add 4 courts to the tournament
  auto scenario05 = [&]()
  {
    scenario04();
    Category ls = cmngr->getCategory("LS");
    MatchMngr* mm = tnmt->getMatchMngr();

    ERR e;
    auto mg = mm->getMatchGroup(ls, 1, 3, &e);  // round 1, players group 3
    assert(e == OK);
    mm->stageMatchGroup(*mg);
    mm->scheduleAllStagedMatchGroups();

    Category ld = cmngr->getCategory("LD");
    mg = mm->getMatchGroup(ld, 1, 1, &e);  // round 1, players group 1
    assert(e == OK);
    mm->stageMatchGroup(*mg);
    mg = mm->getMatchGroup(ld, 1, 2, &e);  // round 1, players group 2
    assert(e == OK);
    mm->stageMatchGroup(*mg);
    mg = mm->getMatchGroup(ld, 2, 1, &e);  // round 2, players group 1
    assert(e == OK);
    mm->stageMatchGroup(*mg);
    mm->scheduleAllStagedMatchGroups();

    // add four courts
    auto cm = tnmt->getCourtMngr();
    for (int i=1; i <= 4; ++i)
    {
      cm->createNewCourt(i, "XX", &e);
      assert(e == OK);
    }
  };

  // extend scenario 5 to already play all matches in the round-robin rounds
  // of category "LS" and "LD"
  auto scenario06 = [&]()
  {
    scenario05();
    Category ls = cmngr->getCategory("LS");
    Category ld = cmngr->getCategory("LD");
    MatchMngr* mm = tnmt->getMatchMngr();
    CourtMngr* cm = tnmt->getCourtMngr();

    // stage and schedule all matches in round 1
    // of LS and LD
    bool canStageMatchGroups = true;
    while (canStageMatchGroups)
    {
      canStageMatchGroups = false;
      for (MatchGroup mg : mm->getMatchGroupsForCat(ls))
      {
        if (mg.getState() != STAT_MG_IDLE) continue;
        if (mm->canStageMatchGroup(mg) != OK) continue;
        mm->stageMatchGroup(mg);
        canStageMatchGroups = true;
      }
      for (MatchGroup mg : mm->getMatchGroupsForCat(ld))
      {
        if (mg.getState() != STAT_MG_IDLE) continue;
        if (mm->canStageMatchGroup(mg) != OK) continue;
        mm->stageMatchGroup(mg);
        canStageMatchGroups = true;
      }
    }
    mm->scheduleAllStagedMatchGroups();

    // play all scheduled matches
    QDateTime curDateTime = QDateTime::currentDateTimeUtc();
    uint epochSecs = curDateTime.toTime_t();
    DbTab* matchTab = tnmt->getDatabaseHandle()->getTab(TAB_MATCH);
    while (true)
    {
      int nextMacthId;
      int nextCourtId;
      mm->getNextViableMatchCourtPair(&nextMacthId, &nextCourtId);
      if (nextMacthId <= 0) break;

      auto nextMatch = mm->getMatch(nextMacthId);
      if (nextMatch == nullptr) break;
      auto nextCourt = cm->getCourtById(nextCourtId);
      if (nextCourt == nullptr) break;

      if (mm->assignMatchToCourt(*nextMatch, *nextCourt) != OK) break;
      auto score = MatchScore::genRandomScore();
      mm->setMatchScoreAndFinalizeMatch(*nextMatch, *score);

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
    Category ls = cmngr->getCategory("LS");

    // set the match system to Single Elimination
    ERR e = ls.setMatchSystem(SINGLE_ELIM) ;
    assert(e == OK);

    // run the category
    unique_ptr<Category> specialCat = ls.convertToSpecializedObject();
    e = cmngr->freezeConfig(ls);
    assert(e == OK);

    // prepare an empty list for the not-required initial group assignment
    vector<PlayerPairList> ppListList;

    // prepare a list for the (faked) initial ranking
    PlayerPairList initialRanking = ls.getPlayerPairs();

    // actually run the category
    e = cmngr->startCategory(ls, ppListList, initialRanking);
    assert(e == OK);

    // stage all match groups
    MatchMngr* mm = tnmt->getMatchMngr();
    auto mg = mm->getMatchGroup(ls, 1, GROUP_NUM__ITERATION, &e);  // round 1
    assert(e == OK);
    mm->stageMatchGroup(*mg);
    mg = mm->getMatchGroup(ls, 2, GROUP_NUM__ITERATION, &e);  // round 2
    assert(e == OK);
    mm->stageMatchGroup(*mg);
    mg = mm->getMatchGroup(ls, 3, GROUP_NUM__L16, &e);  // round 3
    assert(e == OK);
    mm->stageMatchGroup(*mg);
    mg = mm->getMatchGroup(ls, 4, GROUP_NUM__QUARTERFINAL, &e);  // round 4
    assert(e == OK);
    mm->stageMatchGroup(*mg);
    mg = mm->getMatchGroup(ls, 5, GROUP_NUM__SEMIFINAL, &e);  // round 5
    assert(e == OK);
    mm->stageMatchGroup(*mg);
    mg = mm->getMatchGroup(ls, 6, GROUP_NUM__FINAL, &e);  // round 6
    assert(e == OK);
    mm->stageMatchGroup(*mg);
    mm->scheduleAllStagedMatchGroups();

    // add four courts
    auto cm = tnmt->getCourtMngr();
    for (int i=1; i <= 4; ++i)
    {
      cm->createNewCourt(i, "XX", &e);
      assert(e == OK);
    }

    // play all matches
    while (true)
    {
      int nextMacthId;
      int nextCourtId;
      mm->getNextViableMatchCourtPair(&nextMacthId, &nextCourtId);
      if (nextMacthId <= 0) break;

      auto nextMatch = mm->getMatch(nextMacthId);
      if (nextMatch == nullptr) break;
      //if (nextMatch->getMatchGroup().getRound() == 2) break;
      auto nextCourt = cm->getCourtById(nextCourtId);
      if (nextCourt == nullptr) break;

      if (mm->assignMatchToCourt(*nextMatch, *nextCourt) != OK) break;
      auto score = MatchScore::genRandomScore();
      mm->setMatchScoreAndFinalizeMatch(*nextMatch, *score);
    }
  };

  // a scenario with up to 10 players in a ranking1-bracket
  auto scenario08 = [&]()
  {
    scenario02();
    tmngr->createNewTeam("Ranking Team");
    Category ls = cmngr->getCategory("LS");
    Category ld = cmngr->getCategory("LD");

    int evenPlayerId = -1;
    for (int i=0; i < 28; i++)   // must be an even number, for doubles!
    {
      QString lastName = "Ranking" + QString::number(i+1);
      pmngr->createNewPlayer("Lady", lastName, F, "Ranking Team");
      Player p = pmngr->getPlayer(i + 7);   // the first six IDs are already used by previous ini-functions above
      ls.addPlayer(p);
      ld.addPlayer(p);

      // pair every two players
      if ((i % 2) == 0)
      {
        evenPlayerId = p.getId();
      } else {
        Player evenPlayer = pmngr->getPlayer(evenPlayerId);
        cmngr->pairPlayers(ld, p, evenPlayer);
      }
    }

    ls.setMatchSystem(MATCH_SYSTEM::RANKING);
    ld.setMatchSystem(MATCH_SYSTEM::RANKING);

    // freeze the LS category
    ERR e = cmngr->freezeConfig(ls);
    assert(e == OK);

    // prepare an empty list for the not-required initial group assignment
    vector<PlayerPairList> ppListList;

    // prepare a list for the (faked) initial ranking
    PlayerPairList initialRanking = ls.getPlayerPairs();

    // actually run the category
    e = cmngr->startCategory(ls, ppListList, initialRanking);
    assert(e == OK);

    // freeze the LD category
    e = cmngr->freezeConfig(ld);
    assert(e == OK);

    // prepare a list for the (faked) initial ranking
    initialRanking = ld.getPlayerPairs();

    // actually run the category
    e = cmngr->startCategory(ld, ppListList, initialRanking);
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

  emit tournamentOpened(tnmt.get());
  
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

MainFrame* MainFrame::getMainFramePointer()
{
  return mainFramePointer;
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
  PlayerMngr* pm = tnmt->getPlayerMngr();
  ERR e = pm->setExternalPlayerDatabase(filename, true);
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
  PlayerMngr* pm = tnmt->getPlayerMngr();
  ERR e = pm->setExternalPlayerDatabase(filename, false);
  if (e != OK)
  {
    QString msg = tr("Could not open ") + filename + "\n\n";
    if (pm->hasExternalPlayerDatabaseOpen())
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
  msg += tr("© Volker Knollmann, 2014 - 2015<br><br>");
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
  msg += "<a href='https://github.com/Foorgol/QTournament'>https://github.com/Foorgol/QTournament</a><br>";

  msg = msg.arg(PRG_VERSION_STRING);

  QMessageBox msgBox{this};
  msgBox.setWindowTitle(tr("About QTournament"));
  msgBox.setTextFormat(Qt::RichText);
  msgBox.setText(msg);
  msgBox.exec();
}

//----------------------------------------------------------------------------

void MainFrame::onToggleTestMenuVisibility()
{
  ui.menubar->clear();
  ui.menubar->addMenu(ui.menuTournament);
  ui.menubar->addMenu(ui.menuAbout_QTournament);

  if (!isTestMenuVisible)
  {
    ui.menubar->addMenu(ui.menuTesting);
  }

  isTestMenuVisible = !isTestMenuVisible;
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


