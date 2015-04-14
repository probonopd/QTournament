#ifndef CATROUNDSTATUS_H
#define CATROUNDSTATUS_H

#include "Tournament.h"
#include "Category.h"
#include "TournamentDB.h"

namespace QTournament
{

class CatRoundStatus
{
public:

  static constexpr int TOTAL_ROUNDS_COUNT_UNKNOWN = -1;
  static constexpr int NO_CURRENTLY_RUNNING_ROUND = -1;
  static constexpr int NO_ROUNDS_FINISHED_YET = -1;
  static constexpr int MULTIPLE_ROUNDS_RUNNING = -2;

  CatRoundStatus(TournamentDB* _db, const Category& _cat);
  ~CatRoundStatus();

  int getFinishedRoundsCount() const;
  int getCurrentlyRunningRoundNumber() const;
  int getTotalRoundsCount() const;
  QList<int> getCurrentlyRunningRoundNumbers() const;
  int getHighestGeneratedMatchRound() const;

  tuple<int, int, int> getMatchCountForCurrentRound() const;

private:
  TournamentDB* db;
  Category cat;
};

}

#endif // CATROUNDSTATUS_H