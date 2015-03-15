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

int CatRoundStatus::getCurrentlyRunningRoundNumber() const
{
  // filter through all currently running matches
  // and search for a match that belongs to this
  // category
  for (Match ma : Tournament::getMatchMngr()->getCurrentlyRunningMatches())
  {
    if (ma.getCategory() == cat)
    {
      return ma.getMatchGroup().getRound();
    }
  }

  // in some cases, a round is running but no match
  // of this round is currently being played
  //
  // read: SOME matches in this round are finished,
  // some are not and none is currently running
  //
  // good news: we only need to check finishedRounds+1 for this
  // ugly border case
  int finishedRounds = getFinishedRoundsCount();
  if (finishedRounds == NO_ROUNDS_FINISHED_YET) finishedRounds = 0;
  for (MatchGroup mg : Tournament::getMatchMngr()->getMatchGroupsForCat(cat, finishedRounds+1))
  {
    for (Match ma : mg.getMatches())
    {
      if (ma.getState() == STAT_MA_FINISHED) return finishedRounds+1;
    }
  }

  return NO_CURRENTLY_RUNNING_ROUND;  // any other case
}

//----------------------------------------------------------------------------

int CatRoundStatus::getFinishedRoundsCount() const
{
  MatchMngr* mm = Tournament::getMatchMngr();

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
  int curRound = getCurrentlyRunningRoundNumber();
  if (curRound == NO_CURRENTLY_RUNNING_ROUND)
  {
    return make_tuple(curRound, curRound, curRound);
  }

  int unfinishedMatchCount = 0;
  int runningMatchCount = 0;
  int totalMatchCount = 0;
  MatchGroupList matchGroupsInThisRound = Tournament::getMatchMngr()->getMatchGroupsForCat(cat, curRound);
  for (MatchGroup mg : matchGroupsInThisRound)
  {
    for (Match ma : mg.getMatches())
    {
      if (ma.getState() != STAT_MA_FINISHED) ++unfinishedMatchCount;
      if (ma.getState() == STAT_MA_RUNNING) ++runningMatchCount;
      ++totalMatchCount;
    }
  }

  // total, unfinished, running
  return make_tuple(totalMatchCount, unfinishedMatchCount, runningMatchCount);
}

//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------

}
