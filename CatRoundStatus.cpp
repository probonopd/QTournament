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

#include <exception>

#include "CatRoundStatus.h"
#include "MatchMngr.h"

namespace QTournament
{

CatRoundStatus::CatRoundStatus(TournamentDB* _db, const Category& _cat)
  :db(nullptr), cat(_cat)
{
  if (_db == nullptr)
  {
    throw std::invalid_argument("Rceived nullptr for database handle");
  }

  db = _db;
  cat = _cat;
}

//----------------------------------------------------------------------------

CatRoundStatus::~CatRoundStatus()
{
}

//----------------------------------------------------------------------------

QList<int> CatRoundStatus::getCurrentlyRunningRoundNumbers() const
{
  QList<int> result;

  int lastFinishedRound = getFinishedRoundsCount();
  int minRoundRunning = -1;
  int maxRoundRunning = -1;

  // the initial round that could be in state RUNNING
  int roundToCheck = (lastFinishedRound < 0) ? 1 : lastFinishedRound+1;

  // the last round that can possibly in state RUNNING
  auto tnmt = Tournament::getActiveTournament();
  int lastRoundToCheck = tnmt->getMatchMngr()->getHighestUsedRoundNumberInCategory(cat);

  // loop through all applicable rounds and check their status
  while (roundToCheck <= lastRoundToCheck)
  {
    bool hasFinishedMatches = cat.hasMatchesInState(STAT_MA_FINISHED, roundToCheck);
    bool hasRunningMatches = cat.hasMatchesInState(STAT_MA_RUNNING, roundToCheck);

    if (hasFinishedMatches || hasRunningMatches)
    {
      if (minRoundRunning < 0) minRoundRunning = roundToCheck;
      if (maxRoundRunning < roundToCheck) maxRoundRunning = roundToCheck;
      result.append(roundToCheck);
    }

    ++roundToCheck;
  }

  return result;
}

//----------------------------------------------------------------------------

int CatRoundStatus::getHighestGeneratedMatchRound() const
{
  auto tnmt = Tournament::getActiveTournament();
  return tnmt->getMatchMngr()->getHighestUsedRoundNumberInCategory(cat);
}

//----------------------------------------------------------------------------

int CatRoundStatus::getCurrentlyRunningRoundNumber() const
{
  QList<int> runningRounds = getCurrentlyRunningRoundNumbers();
  int n = runningRounds.count();

  if (n < 1) return NO_CURRENTLY_RUNNING_ROUND;
  if (n == 1) return runningRounds.at(0);

  return MULTIPLE_ROUNDS_RUNNING;  // can happen in group match rounds
}

//----------------------------------------------------------------------------

int CatRoundStatus::getFinishedRoundsCount() const
{
  auto tnmt = Tournament::getActiveTournament();
  MatchMngr* mm = tnmt->getMatchMngr();

  int roundNum = 1;
  int lastFinishedRound = NO_ROUNDS_FINISHED_YET;

  // go through rounds one by one
  while (true)
  {
    MatchGroupList matchGroupsInThisRound = mm->getMatchGroupsForCat(cat, roundNum);
    // finish searching for rounds when no more groups show up
    // in the search
    if (matchGroupsInThisRound.count() == 0) break;

    bool allGroupsFinished = true;
    for (MatchGroup mg : matchGroupsInThisRound)
    {
      if (mg.getState() != STAT_MG_FINISHED)
      {
        allGroupsFinished = false;
        break;
      }
    }

    if (!allGroupsFinished) break;

    // we only make it to this point if all match groups
    // in this round are finished.
    // so we may safely assume that this round is finished
    lastFinishedRound = roundNum;

    // check next round
    ++roundNum;
  }

  return lastFinishedRound;
}

//----------------------------------------------------------------------------

tuple<int, int, int> CatRoundStatus::getMatchCountForCurrentRound() const
{
  QList<int> runningRounds = getCurrentlyRunningRoundNumbers();

  if (runningRounds.count() == 0)
  {
    int tmp = NO_CURRENTLY_RUNNING_ROUND;
    return make_tuple(tmp, tmp, tmp);
  }

  int unfinishedMatchCount = 0;
  int runningMatchCount = 0;
  int totalMatchCount = 0;

  auto tnmt = Tournament::getActiveTournament();
  auto mm = tnmt->getMatchMngr();
  for (int curRound : runningRounds)
  {
    MatchGroupList matchGroupsInThisRound = mm->getMatchGroupsForCat(cat, curRound);
    for (MatchGroup mg : matchGroupsInThisRound)
    {
      for (Match ma : mg.getMatches())
      {
        if (ma.getState() != STAT_MA_FINISHED) ++unfinishedMatchCount;
        if (ma.getState() == STAT_MA_RUNNING) ++runningMatchCount;
        ++totalMatchCount;
      }
    }
  }

  // total, unfinished, running
  return make_tuple(totalMatchCount, unfinishedMatchCount, runningMatchCount);
}

//----------------------------------------------------------------------------

int CatRoundStatus::getTotalRoundsCount() const
{
  auto specializedCat = cat.convertToSpecializedObject();
  return specializedCat->calcTotalRoundsCount();
}

//----------------------------------------------------------------------------


//----------------------------------------------------------------------------

}
