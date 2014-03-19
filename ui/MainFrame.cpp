/*
 * File:   MainFrame.cpp
 * Author: volker
 *
 * Created on February 16, 2014, 5:16 PM
 */

#include "MainFrame.h"

#include "Tournament.h"
#include "PlayerTableView.h"
#include "dlgEditPlayer.h"

#include <QMessageBox>

using namespace QTournament;

MainFrame::MainFrame()
{
  tnmt = NULL;
  
  ui.setupUi(this);
  enableControls(false);
  
  testFileName = QDir().absoluteFilePath("tournamentTestFile.tdb");
  
  // connect my own signals and slots (not those handled by QtDesigner)
  connect(this, &MainFrame::tournamentOpened, ui.teamList, &TeamListView::onTournamentOpened);
  connect(this, &MainFrame::tournamentOpened, ui.playerView, &PlayerTableView::onTournamentOpened);
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
    
    PlayerMngr* pmngr = Tournament::getPlayerMngr();
    pmngr->createNewPlayer("First1", "Last1", M, "Team 1");
    pmngr->createNewPlayer("First2", "Last2", F, "Team 1");
    pmngr->createNewPlayer("First3", "Last3", M, "Team 1");
    pmngr->createNewPlayer("First4", "Last4", F, "Team 1");
    pmngr->createNewPlayer("First5", "Last5", M, "Team 2");
    pmngr->createNewPlayer("First6", "Last6", F, "Team 2");
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

void MainFrame::onCreateTeamClicked()
{
  int cnt = 0;
  
  // try to create new teams using a
  // canonical name until it finally succeeds
  TeamMngr* tmngr = Tournament::getTeamMngr();
  ERR e = NAME_EXISTS;
  
  while (e != OK)
  {
    QString teamName = tr("New Team ") + QString::number(cnt);
    
    e = tmngr->createNewTeam(teamName);
    cnt++;
  }
}

//----------------------------------------------------------------------------

void MainFrame::onCreatePlayerClicked()
{
  DlgEditPlayer dlg;
  
  dlg.setModal(true);
  int result = dlg.exec();
  
  if (result != QDialog::Accepted)
  {
    return;
  }
  
  // we can be sure that all selected data in the dialog
  // is valid. That has been checked before the dialog
  // returns with "Accept". So we can directly step
  // into the creation of the new player
  ERR e = Tournament::getPlayerTableModel()->createNewPlayer(
                                                       dlg.getFirstName(),
                                                       dlg.getLastName(),
                                                       dlg.getSex(),
                                                       dlg.getTeam().getName()
                                                       );
  
  if (e != OK)
  {
    QString msg = tr("Something went wrong when inserting the player. This shouldn't happen.");
    msg += tr("For the records: error code = ") + QString::number(static_cast<int>(e));
    QMessageBox::warning(this, tr("WTF??"), msg);
  }
}

//----------------------------------------------------------------------------

void MainFrame::onPlayerDoubleClicked(const QModelIndex& index)
{
  if (!(index.isValid()))
  {
    return;
  }
  
  Player selectedPlayer = Tournament::getPlayerMngr()->getPlayerBySeqNum(index.row());
  
  DlgEditPlayer dlg(&selectedPlayer);
  
  dlg.setModal(true);
  int result = dlg.exec();
  
  if (result != QDialog::Accepted)
  {
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


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


