/*
 * File:   MainFrame.cpp
 * Author: volker
 *
 * Created on February 16, 2014, 5:16 PM
 */

#include "MainFrame.h"

#include "Tournament.h"

#include <QMessageBox>
#include <stdexcept>
#include <QtCore/qnamespace.h>

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
  enableControls(true);
}

//----------------------------------------------------------------------------

void MainFrame::openTournament()
{
  enableControls(false);
}

//----------------------------------------------------------------------------

void MainFrame::enableControls(bool doEnable)
{
  ui.centralwidget->setEnabled(doEnable);
}

//----------------------------------------------------------------------------

void MainFrame::closeCurrentTournament()
{
  // close the tournament
  if (tnmt != nullptr)
  {
    // this emits a signal to inform everyone that the
    // current tournament is about to die
    tnmt->close();
    
    delete tnmt;
    tnmt = nullptr;
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
  if ((scenarioID < 0) || (scenarioID > 6))
  {
    QMessageBox::critical(this, "Setup Test Scenario", "The scenario ID " + QString::number(scenarioID) + " is invalid!");
  }
  
  // shutdown whatever is open right now
  closeCurrentTournament();
  
  // prepare a brand-new scenario
  TournamentSettings cfg;
  cfg.organizingClub = "club";
  cfg.tournamentName = "name";
  cfg.useTeams = true;
  tnmt = new Tournament(testFileName, cfg);
  
  // the empty scenario
  if (scenarioID == 0)
  {
  }
  
  // a scenario with just a few teams already existing
  TeamMngr* tmngr = Tournament::getTeamMngr();
  PlayerMngr* pmngr = Tournament::getPlayerMngr();
  CatMngr* cmngr = Tournament::getCatMngr();
  
  if ((scenarioID > 0) && (scenarioID < 99))   // Scenario 1...
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
  }
  
  if ((scenarioID > 1) && (scenarioID < 99))  // Scenario 2...
  {
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
  }
  
  // a scenario with a lot of participants, including a group of
  // forty players in one category
  if ((scenarioID > 2) && (scenarioID < 99))  // Scenario 3...
  {
    tmngr->createNewTeam("Massive");
    Category ls = cmngr->getCategory("LS");
    
    for (int i=0; i < 250; i++)
    {
      QString lastName = "Massive" + QString::number(i);
      pmngr->createNewPlayer("Lady", lastName, F, "Massive");
      Player p = pmngr->getPlayer(i + 7);   // the first six IDs are already used by previous ini-functions above
      if (i < 40) ls.addPlayer(p);
    }
    
    // create and set a valid group configuration for LS
    GroupDef d = GroupDef(5, 8);
    GroupDefList gdl;
    gdl.append(d);
    KO_Config cfg(QUARTER, false, gdl);
    ls.setParameter(GROUP_CONFIG, cfg.toString());
  }

  // extend scenario 3 to already start category "LS"
  // and add a few players to LD and start this category, too
  if ((scenarioID > 3) && (scenarioID < 99))  // Scenario 4...
  {
    Category ls = cmngr->getCategory("LS");

    // run the category
    unique_ptr<Category> specialCat = ls.convertToSpecializedObject();
    ERR e = cmngr->freezeConfig(ls);
    assert(e == OK);

    // fake a list of player-pair-lists for the group assignments
    QList<PlayerPairList> ppListList;
    for (int grpNum=0; grpNum < 8; ++grpNum)
    {
        PlayerPairList thisGroup;
        for (int pNum=0; pNum < 5; ++pNum)
        {
            int playerId = (grpNum * 5) + pNum + 7;  // the first six IDs are already in use; see above

            Player p = pmngr->getPlayer(playerId);
            PlayerPair pp(p, (playerId-6));   // PlayerPairID starts at 1
            thisGroup.append(pp);
        }
        ppListList.append(thisGroup);
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
            thisGroup.append(allPairsInCat.at(grpNum * 4 + pNum));
        }
        ppListList.append(thisGroup);
    }

    // make sure the faked group assignment is valid
    e = specialCat->canApplyGroupAssignment(ppListList);
    assert(e == OK);

    // actually run the category
    e = cmngr->startCategory(ld, ppListList, initialRanking);  // "initialRanking" is reused from above
    assert(e == OK);
  }

  // extend scenario 4 to already stage and schedule a few match groups
  // in category "LS" and "LD"
  // Additionally, we add 4 courts to the tournament
  if ((scenarioID > 4) && (scenarioID < 99))  // Scenario 5...
  {
    Category ls = cmngr->getCategory("LS");
    MatchMngr* mm = Tournament::getMatchMngr();

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
    auto cm = Tournament::getCourtMngr();
    for (int i=1; i <= 4; ++i)
    {
      cm->createNewCourt(i, "XX", &e);
      assert(e == OK);
    }
  }

  // extend scenario 5 to already play all matches in the round-robin rounds
  // of category "LS" and "LD"
  if ((scenarioID > 5) && (scenarioID < 99))  // Scenario 6...
  {
    Category ls = cmngr->getCategory("LS");
    Category ld = cmngr->getCategory("LD");
    MatchMngr* mm = Tournament::getMatchMngr();
    CourtMngr* cm = Tournament::getCourtMngr();
    ERR e;

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
    }
  }

  enableControls(true);
  
  emit tournamentOpened(tnmt);
  
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
  setupTestScenario(6);
}

//----------------------------------------------------------------------------

MainFrame* MainFrame::getMainFramePointer()
{
  return mainFramePointer;
}

//----------------------------------------------------------------------------

void MainFrame::onCurrentTabChanged(int newCurrentTab)
{

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


