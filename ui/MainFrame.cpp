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
#include <qt4/QtCore/qnamespace.h>

using namespace QTournament;

//----------------------------------------------------------------------------

MainFrame* MainFrame::mainFramePointer = NULL;

//----------------------------------------------------------------------------

MainFrame::MainFrame()
{
  if (MainFrame::mainFramePointer != NULL)
  {
    throw std::runtime_error("Only one MainFrame instance is allowed!!");
  }
  MainFrame::mainFramePointer = this;
  
  tnmt = NULL;
  
  ui.setupUi(this);
  showMaximized();
  enableControls(false);
  
  testFileName = QDir().absoluteFilePath("tournamentTestFile.tdb");
}

//----------------------------------------------------------------------------

MainFrame::~MainFrame()
{
  closeCurrentTournament();
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
  if (tnmt != NULL)
  {
    // this emits a signal to inform everyone that the
    // current tournament is about to die
    tnmt->close();
    
    delete tnmt;
    tnmt = NULL;
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
  if ((scenarioID < 0) || (scenarioID > 2))
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
  
  if ((scenarioID > 0) && (scenarioID < 99))
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
  
  if ((scenarioID > 1) && (scenarioID < 99))
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

//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

MainFrame* MainFrame::getMainFramePointer()
{
  return mainFramePointer;
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


//----------------------------------------------------------------------------


