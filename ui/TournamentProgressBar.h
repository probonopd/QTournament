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
  void setDatabase(const QTournament::TournamentDB* _db);

public slots:
  void updateProgressBar();
  void onMatchTimePredictionChanged(int newAvgMatchDuration, time_t newLastMatchFinish);

private:
  static constexpr int PollTimerIntervall_ms = 1000;  // update once a second
  const QTournament::TournamentDB* db{nullptr};
  QString rawStatusString;
  time_t lastMatchFinishTime__UTC;
  int avgMatchDuration__secs;
  std::unique_ptr<QTimer> statPollTimer;
};

#endif // TOURNAMENTPROGRESSBAR_H
