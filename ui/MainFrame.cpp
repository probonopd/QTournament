/*
 * File:   MainFrame.cpp
 * Author: volker
 *
 * Created on February 16, 2014, 5:16 PM
 */

#include "MainFrame.h"

#include "Tournament.h"

#include <QMessageBox>

using namespace QTournament;

MainFrame::MainFrame()
{
  tnmt = NULL;
  
  ui.setupUi(this);
  enableControls(false);
  
  testFileName = QDir().absoluteFilePath("tournamentTestFile.tdb");
  
  // connect my own signals and slots (not those handled by QtDesigner)
  connect(this, &MainFrame::tournamentClosed, ui.listView, &TeamListView::onTournamentClosed);
  connect(this, &MainFrame::tournamentOpened, ui.listView, &TeamListView::onTournamentOpened);
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
  
  emit tournamentClosed();
}

//----------------------------------------------------------------------------

void MainFrame::setupTestScenario(int scenarioID)
{
  if ((scenarioID < 0) || (scenarioID > 1))
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
  if (scenarioID == 1)
  {
    TeamMngr* tmngr = Tournament::getTeamMngr();
    tmngr->createNewTeam("Team 1");
    tmngr->createNewTeam("Team 2");
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

