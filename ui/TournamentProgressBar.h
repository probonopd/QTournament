#ifndef TOURNAMENTPROGRESSBAR_H
#define TOURNAMENTPROGRESSBAR_H

#include <memory>
#include <QProgressBar>
#include <QTimer>

#include "TournamentDB.h"


class TournamentProgressBar : public QProgressBar
{
  Q_OBJECT

public:
  TournamentProgressBar(QWidget* parentWidget = nullptr);
  void setDatabase(QTournament::TournamentDB* _db);

public slots:
  void updateProgressBar();
  void onMatchTimePredictionChanged(int newAvgMatchDuration, time_t newLastMatchFinish);

private:
  static constexpr int ObjState::POLL_TIMER_INTERVAL__MS = 1000;  // update once a second
  QTournament::TournamentDB* db;
  QString rawStatusString;
  time_t lastMatchFinishTime__UTC;
  int avgMatchDuration__secs;
  std::unique_ptr<QTimer> statPollTimer;
};

#endif // TOURNAMENTPROGRESSBAR_H
