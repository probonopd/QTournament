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
