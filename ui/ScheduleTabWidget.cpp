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

#include <QMessageBox>

#include "ScheduleTabWidget.h"
#include "ui_ScheduleTabWidget.h"
#include "GuiHelpers.h"
#include "Score.h"
#include "ui/DlgMatchResult.h"
#include "ui/MainFrame.h"

ScheduleTabWidget::ScheduleTabWidget(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ScheduleTabWidget)
{
    ui->setupUi(this);

    // initialize sorting and filtering in the two match group views
    ui->mgIdleView->setFilter(MatchGroupTableView::FilterType::IDLE);
    ui->mgStagedView->setFilter(MatchGroupTableView::FilterType::STAGED);
    ui->mgStagedView->sortByColumn(MatchGroupTableModel::STAGE_SEQ_COL_ID, Qt::AscendingOrder);

    // subscribe to the tournament-opened-signal
    connect(MainFrame::getMainFramePointer(), &MainFrame::tournamentOpened, this, &ScheduleTabWidget::onTournamentOpened);

    // react on selection changes in the IDLE match groups view
    connect(ui->mgIdleView->selectionModel(),
      SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
      SLOT(onIdleSelectionChanged(const QItemSelection&, const QItemSelection&)));

    // react on selection changes in the STAGED match groups view
    connect(ui->mgStagedView->selectionModel(),
      SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
      SLOT(onStagedSelectionChanged(const QItemSelection&, const QItemSelection&)));

    // default all buttons to "disabled"
    ui->btnSchedule->setEnabled(false);
    ui->btnStage->setEnabled(false);
    ui->btnUnstage->setEnabled(false);

    // initial status bar value
    QString rawProgressBarString = tr("No match status available");
    updateProgressBar();
}

//----------------------------------------------------------------------------

ScheduleTabWidget::~ScheduleTabWidget()
{
    delete ui;
}

//----------------------------------------------------------------------------

void ScheduleTabWidget::onBtnStageClicked()
{
  // lets check if a valid match group is selected
  auto mg = ui->mgIdleView->getSelectedMatchGroup();
  if (mg == nullptr) return;

  auto mm = Tournament::getMatchMngr();
  if (mm->canStageMatchGroup(*mg) != OK) return;

  mm->stageMatchGroup(*mg);
}

//----------------------------------------------------------------------------

void ScheduleTabWidget::onBtnUnstageClicked()
{
  // lets check if a valid match group is selected
  auto mg = ui->mgStagedView->getSelectedMatchGroup();
  if (mg == nullptr) return;

  auto mm = Tournament::getMatchMngr();
  if (mm->canUnstageMatchGroup(*mg) != OK) return;

  mm->unstageMatchGroup(*mg);
}

//----------------------------------------------------------------------------

void ScheduleTabWidget::onBtnScheduleClicked()
{
  // is at least one match group staged?
  auto mm = Tournament::getMatchMngr();
  if (mm->getMaxStageSeqNum() == 0) return;

  mm->scheduleAllStagedMatchGroups();
  updateButtons();

  // resize columns to fit new matches best
  ui->tvMatches->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
}

//----------------------------------------------------------------------------

void ScheduleTabWidget::onIdleSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
  updateButtons();
}

//----------------------------------------------------------------------------

void ScheduleTabWidget::onStagedSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
  updateButtons();
}

//----------------------------------------------------------------------------

void ScheduleTabWidget::updateButtons()
{
  auto mm = Tournament::getMatchMngr();

  // update the "stage"-button
  auto mg = ui->mgIdleView->getSelectedMatchGroup();
  if ((mg != nullptr) && (mm->canStageMatchGroup(*mg) == OK))
  {
    ui->btnStage->setEnabled(true);
  } else {
    ui->btnStage->setEnabled(false);
  }

  // update the "unstage"-button
  mg = ui->mgStagedView->getSelectedMatchGroup();
  if ((mg != nullptr) && (mm->canUnstageMatchGroup(*mg) == OK))
  {
    ui->btnUnstage->setEnabled(true);
  } else {
    ui->btnUnstage->setEnabled(false);
  }

  // update the "Schedule"-button
  ui->btnSchedule->setEnabled(mm->getMaxStageSeqNum() != 0);
}

//----------------------------------------------------------------------------

void ScheduleTabWidget::onCourtDoubleClicked(const QModelIndex &index)
{
  auto court = ui->tvCourts->getSelectedCourt();
  if (court == nullptr) return;

  auto ma = court->getMatch();
  if (ma == nullptr)
  {
    return;  // no match assigned to this court
  }

  askAndStoreMatchResult(*ma);
}

//----------------------------------------------------------------------------

void ScheduleTabWidget::onRoundCompleted(int catId, int round)
{
  Category cat = Tournament::getCatMngr()->getCategoryById(catId);

  QString txt = tr("Round %1 of category %2 finished!").arg(round).arg(cat.getName());

  QMessageBox::information(this, tr("Round finished"), txt);
}

//----------------------------------------------------------------------------

void ScheduleTabWidget::askAndStoreMatchResult(const Match &ma)
{
  // only accept results for running matches
  if (ma.getState() != STAT_MA_RUNNING)
  {
    return;
  }

  // ask the user for the match result
  DlgMatchResult dlg(this, ma);
  dlg.setModal(true);
  int dlgResult = dlg.exec();
  if (dlgResult != QDialog::Accepted)
  {
    return;
  }
  auto matchResult = dlg.getMatchScore();
  assert(matchResult != nullptr);

  // create a (rather ugly) confirmation message box
  QString msg = tr("Please confirm:\n\n");
  msg += ma.getPlayerPair1().getDisplayName() + "\n";
  msg += "\tvs.\n";
  msg += ma.getPlayerPair2().getDisplayName() + "\n\n";
  msg += tr("Result: ");
  QString sResult = matchResult->toString();
  sResult = sResult.replace(",", ", ");
  msg += sResult + "\n\n\n";
  if (matchResult->getWinner() == 1)
  {
    msg += tr("WINNER: ");
    msg += ma.getPlayerPair1().getDisplayName();
    msg += "\n\n";
    msg += tr("LOSER: ");
    msg += ma.getPlayerPair2().getDisplayName();
  }
  if (matchResult->getWinner() == 2)
  {
    msg += tr("WINNER: ");
    msg += ma.getPlayerPair2().getDisplayName();
    msg += "\n\n";
    msg += tr("LOSER: ");
    msg += ma.getPlayerPair1().getDisplayName();
  }
  if (matchResult->getWinner() == 0)
  {
    msg += tr("The match result is DRAW");
  }
  msg += "\n\n";

  int confirm = QMessageBox::question(this, tr("Please confirm match result"), msg);
  if (confirm != QMessageBox::Yes) return;

  // actually store the data and update the internal object states
  MatchMngr* mm = Tournament::getMatchMngr();
  mm->setMatchScoreAndFinalizeMatch(ma, *matchResult);

  // ask the user if the next available match should be started on the
  // now free court
  //
  // first of all, check if there is a next match available
  int nextMatchId;
  int nextCourtId;
  ERR e = mm->getNextViableMatchCourtPair(&nextMatchId, &nextCourtId, true);
  if ((e == NO_MATCH_AVAIL) || (nextMatchId < 1))
  {
    return;
  }

  // now ask if the match should be started
  confirm = QMessageBox::question(this, tr("Next Match"), tr("Start the next available match on the free court?"));
  if (confirm != QMessageBox::Yes) return;

  // instead of the court determined by getNextViableMatchCourtPair we use
  // the court of the previous match
  auto oldCourt = ma.getCourt();
  assert(oldCourt != nullptr);

  // can we assign the next match to the old court?
  auto nextMatch = mm->getMatch(nextMatchId);
  assert(nextMatch != nullptr);
  e = mm->canAssignMatchToCourt(*nextMatch, *oldCourt);
  if (e != OK)
  {
    QString msg = tr("The match cannot be started on this court. Please start the next match manually.");
    QMessageBox::critical(this, tr("Next Match"), msg);
    return;
  }

  // ok, we're all set. Call the match
  //
  // TODO: this is redundant code
  QString call = GuiHelpers::prepCall(*nextMatch, *oldCourt);
  int result = QMessageBox::question(this, tr("Assign match to court"), call);

  if (result == QMessageBox::Yes)
  {
    // after all the checks before, the following call
    // should always yield "ok"
    e = mm->assignMatchToCourt(*nextMatch, *oldCourt);
    if (e != OK)
    {
      QString msg = tr("An unexpected error occured.\n");
      msg += tr("Sorry, this shouldn't happen.\n");
      msg += tr("The match cannot be started.");
      QMessageBox::critical(this, tr("Assign match to court"), msg);
    }
    ui->tvMatches->updateSelectionAfterDataChange();
    return;
  }
  QMessageBox::information(this, tr("Assign match to court"), tr("Call cancled, match not started."));
}

//----------------------------------------------------------------------------

int ScheduleTabWidget::estimateRemainingTournamentTime()
{
  // we can't do any estimations if don't have courts
  // to play on
  int courtCount = Tournament::getCourtMngr()->getActiveCourtCount();
  if (courtCount <= 0) return -1;

  QDateTime curDateTime = QDateTime::currentDateTimeUtc();
  uint epochNow = curDateTime.toTime_t();

  // calculate the average runtime of all running matches
  int totalRuntime = 0;
  auto runningMatches = Tournament::getMatchMngr()->getCurrentlyRunningMatches();
  for (const Match& ma : runningMatches)
  {
    auto startTime = ma.getStartTime();
    if (startTime.isNull()) continue;

    uint startTimeSecs = startTime.toTime_t();

    // calculate the time the match is already running
    int runtime = epochNow - startTimeSecs;

    // sum everything up
    totalRuntime += runtime;
  }

  // calculate the overall remaining playing time for all unfinished matches
  int nTotal;
  int nFinished;
  int nRunning;
  tie(nTotal, nFinished, nRunning) = Tournament::getMatchMngr()->getMatchStats();
  int avgMatchDuration = getAverageMatchDuration();
  int totalRemainingPlayingTime = (nTotal - nFinished) * avgMatchDuration;

  // subtract the time already "consumed" by the running matches
  totalRemainingPlayingTime -= totalRuntime;

  // distribute the remaining time evenly over all courts
  return (totalRemainingPlayingTime / courtCount);
}

//----------------------------------------------------------------------------

int ScheduleTabWidget::getAverageMatchDuration()
{
  return (totalDurationCnt > 0) ? totalDuration / totalDurationCnt : INITIAL_AVG_MATCH_DURATION__SECS;
}

//----------------------------------------------------------------------------

void ScheduleTabWidget::updateProgressBar()
{
  if (tnmt == nullptr)
  {
    ui->pbRemainingMatches->setFormat(QString());
    ui->pbRemainingMatches->setValue(0);
    return;
  }

  QString txt = rawProgressBarString;
  int timeRemain = estimateRemainingTournamentTime();
  if (timeRemain < 0)
  {
    txt = txt.arg(tr("<unknown>"));
  } else {
    int hours = timeRemain / 3600;
    int minutes = (timeRemain % 3600) / 60;
    QString s = "%1:%2";
    s = s.arg(hours);
    s = s.arg(minutes, 2, 10, QLatin1Char('0'));
    txt = txt.arg(s);
  }
  ui->pbRemainingMatches->setFormat(txt);
}

//----------------------------------------------------------------------------

void ScheduleTabWidget::initProgressBarFromDatabase()
{
  // calculate the total match duration of all finished matches
  totalDuration = 0;
  totalDurationCnt = 0;
  for (const Match& ma : Tournament::getMatchMngr()->getFinishedMatches())
  {
    int duration = ma.getMatchDuration();
    if (duration < 0) continue;
    totalDuration += duration;
    ++totalDurationCnt;
  }

  // update the progress bar using dummy match parameters
  onMatchStatusChanged(-1, -1, STAT_MA_FINISHED, STAT_MA_FINISHED);
}

//----------------------------------------------------------------------------

void ScheduleTabWidget::onTournamentOpened(Tournament* _tnmt)
{
  tnmt = _tnmt;
  // connect signals from the Tournament and MatchMngr with my slots
  connect(_tnmt, &Tournament::tournamentClosed, this, &ScheduleTabWidget::onTournamentClosed);
  connect(Tournament::getMatchMngr(), SIGNAL(roundCompleted(int,int)), this, SLOT(onRoundCompleted(int,int)));
  connect(Tournament::getMatchMngr(), SIGNAL(matchStatusChanged(int,int,OBJ_STATE,OBJ_STATE)), this, SLOT(onMatchStatusChanged(int,int,OBJ_STATE,OBJ_STATE)));
  connect(Tournament::getCatMngr(), SIGNAL(categoryStatusChanged(Category,OBJ_STATE,OBJ_STATE)), this, SLOT(onCatStatusChanged()));

  initProgressBarFromDatabase();
}

//----------------------------------------------------------------------------

void ScheduleTabWidget::onMatchStatusChanged(int matchId, int matchSeqNum, OBJ_STATE fromState, OBJ_STATE toState)
{
  // get updated match status counters
  int nTotal;
  int nFinished;
  int nRunning;
  tie(nTotal, nFinished, nRunning) = Tournament::getMatchMngr()->getMatchStats();
  int percComplete = (nTotal > 0) ? (nFinished * 100) / nTotal : 0;

  // update the match duration counter, if applicable
  if ((toState == STAT_MA_FINISHED) && (matchId > 0))
  {
    auto ma = Tournament::getMatchMngr()->getMatch(matchId);
    assert(ma != nullptr);
    int duration = ma->getMatchDuration();

    if (duration >= 0)
    {
      totalDuration += duration;
      ++totalDurationCnt;
    }
  }

  // put everything into a status string
  rawProgressBarString = tr("%1 matches in total, %2 finished (%3 %), %4 running");
  rawProgressBarString = rawProgressBarString.arg(nTotal).arg(nFinished).arg(percComplete).arg(nRunning);
  rawProgressBarString += "   " + tr("Avg. match duration: %1 min., est. remaining tournament time: %2 min.");
  rawProgressBarString = rawProgressBarString.arg(getAverageMatchDuration() / 60);

  // update the progress bar
  ui->pbRemainingMatches->setValue(percComplete);
  updateProgressBar();
}

//----------------------------------------------------------------------------

void ScheduleTabWidget::onCatStatusChanged()
{
  // update the progress bar using dummy match parameters
  onMatchStatusChanged(-1, -1, STAT_MA_FINISHED, STAT_MA_FINISHED);
}

//----------------------------------------------------------------------------

void ScheduleTabWidget::onTournamentClosed()
{
  // reset the progress bar
  ui->pbRemainingMatches->setValue(0);
  ui->pbRemainingMatches->setFormat(QString());

  // disconnect from all signals, because
  // the sending objects don't exist anymore
  disconnect(Tournament::getMatchMngr(), SIGNAL(roundCompleted(int,int)), this, SLOT(onRoundCompleted(int,int)));
  disconnect(Tournament::getMatchMngr(), SIGNAL(matchStatusChanged(int,int,OBJ_STATE,OBJ_STATE)), this, SLOT(onMatchStatusChanged(int,int,OBJ_STATE,OBJ_STATE)));
  disconnect(Tournament::getCatMngr(), SIGNAL(categoryStatusChanged(Category,OBJ_STATE,OBJ_STATE)), this, SLOT(onCatStatusChanged()));
  disconnect(tnmt, &Tournament::tournamentClosed, this, &ScheduleTabWidget::onTournamentClosed);
  tnmt = nullptr;
}

//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------

