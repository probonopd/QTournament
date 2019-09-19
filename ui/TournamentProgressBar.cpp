#include <ctime>

#include <QDateTime>

#include "TournamentProgressBar.h"
#include "MatchMngr.h"
#include "CentralSignalEmitter.h"

using namespace QTournament;

TournamentProgressBar::TournamentProgressBar(QWidget* parentWidget)
  :QProgressBar(parentWidget)
{
  // prep the raw status string
  rawStatusString = tr("%1 matches in total, %2 scheduled, %3 running, %4 finished (%5 %) ; ");
  rawStatusString += tr("avg. match duration: %6 min. ; last scheduled match finished at %7 (%8)");

  // set the progressbar range to 0...100
  setMinimum(0);
  setMaximum(100);

  // start with an empty database
  setDatabase(nullptr);

  // set up a timer that polls the match stats once a second
  // and updates the status bar accordingly
  statPollTimer = std::make_unique<QTimer>();
  connect(statPollTimer.get(), SIGNAL(timeout()), this, SLOT(updateProgressBar()));
  statPollTimer->start(PollTimerIntervall_ms);

  // connect to match time prediction updates and match count updates
  CentralSignalEmitter* cse = CentralSignalEmitter::getInstance();
  connect(cse, SIGNAL(matchTimePredictionChanged(int,time_t)), this, SLOT(onMatchTimePredictionChanged(int,time_t)));
}

//----------------------------------------------------------------------------

void TournamentProgressBar::setDatabase(const TournamentDB* _db)
{
  db = _db;

  // reset all timers; updates will be triggered automatically
  // via the CentralSignalEmitter
  avgMatchDuration__secs = -1;
  lastMatchFinishTime__UTC = 0;

  // clear all content if we have no database open
  if (db == nullptr)
  {
    // clear all content
    setFormat("");
    setValue(0);
    return;
  }

  // in all other cases update the status string
  updateProgressBar();
}

//----------------------------------------------------------------------------

void TournamentProgressBar::updateProgressBar()
{
  if (db == nullptr) return;

  // get updated match status counters
  MatchMngr mm{*db};
  auto [nTotal, nScheduled, nRunning, nFinished] = mm.getMatchStats();

  // calculate the completion percent
  int percComplete = (nTotal > 0) ? (nFinished * 100) / nTotal : 0;

  // create the status string
  QString txt = rawStatusString;
  txt = txt.arg(nTotal).arg(nScheduled).arg(nRunning).arg(nFinished);
  txt = txt.arg(percComplete);
  if (avgMatchDuration__secs > 0)
  {
    txt = txt.arg(avgMatchDuration__secs / 60);
  } else {
    txt = txt.arg("??");
  }
  if (lastMatchFinishTime__UTC > 0)
  {
    QDateTime finish = QDateTime::fromTime_t(lastMatchFinishTime__UTC);
    txt = txt.arg(finish.toString("HH:mm"));

    time_t now = time(nullptr);
    int remainSecs = lastMatchFinishTime__UTC - now;

    int hours = remainSecs / 3600;
    int minutes = (remainSecs - hours * 3600) / 60;

    QString s = tr("%1 h %2 min.");
    s = s.arg(hours).arg(minutes, 2, 10, QLatin1Char('0'));
    txt = txt.arg(s);
  } else {
    txt = txt.arg("??").arg("??");
  }

  // set string and percentage value
  setFormat(txt);
  setValue(percComplete);
}

//----------------------------------------------------------------------------

void TournamentProgressBar::onMatchTimePredictionChanged(int newAvgMatchDuration, time_t newLastMatchFinish)
{
  avgMatchDuration__secs = newAvgMatchDuration;
  lastMatchFinishTime__UTC = newLastMatchFinish;
  updateProgressBar();
}
