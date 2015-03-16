/* 
 * File:   TeamMngr.cpp
 * Author: nyoknvk1
 * 
 * Created on 18. Februar 2014, 14:04
 */

#include <stdexcept>

#include "RankingMngr.h"
#include "Tournament.h"
#include "CatRoundStatus.h"
#include "RankingEntry.h"
#include "Match.h"
#include "Score.h"

using namespace dbOverlay;

namespace QTournament
{

  RankingMngr::RankingMngr(TournamentDB* _db)
  : GenericObjectManager(_db), rankTab((*db)[TAB_RANKING])
  {
  }

//----------------------------------------------------------------------------

  RankingEntryList RankingMngr::createUnsortedRankingEntriesForLastRound(const Category &cat, ERR *err, PlayerPairList _ppList, bool reset)
  {
    // determine the round we should create the entries for
    CatRoundStatus crs = cat.getRoundStatus();
    int lastRound = crs.getFinishedRoundsCount();
    if (lastRound < 1)
    {
      if (err != nullptr) *err = ROUND_NOT_FINISHED;
      return RankingEntryList();
    }

    // establish the list of player pairs for that the ranking shall
    // be created; it's either ALL pairs or a caller-provided list
    PlayerPairList ppList;
    if (_ppList.isEmpty())
    {
      ppList = cat.getPlayerPairs();
    } else {
      ppList = _ppList;
    }

    // iterate over the player pair list and create entries
    // based on match result and, possibly, previous ranking entries
    RankingEntryList result;
    for (PlayerPair pp : ppList)
    {
      unique_ptr<Match> ma = Tournament::getMatchMngr()->getMatchForPlayerPairAndRound(pp, lastRound);

      // prepare the values for the new entry
      int wonMatches = 0;
      int drawMatches = 0;
      int lostMatches = 0;
      int wonGames = 0;
      int lostGames = 0;
      int wonPoints = 0;
      int lostPoints = 0;
      if (ma != nullptr)
      {
        // determine whether our pp is player 1 or player 2
        int pp1Id = ma->getPlayerPair1().getPairId();
        int playerNum = (pp1Id == pp.getPairId()) ? 1 : 2;

        // create column values for match data
        unique_ptr<MatchScore> score = ma->getScore();
        wonMatches = (score->getWinner() == playerNum) ? 1 : 0;
        lostMatches = (score->getLoser() == playerNum) ? 1 : 0;
        drawMatches = (score->getWinner() == 0) ? 1 : 0;

        // create column values for game data
        tuple<int, int> gameSum = score->getGameSum();
        int gamesTotal = get<0>(gameSum) + get<1>(gameSum);
        wonGames = (playerNum == 1) ? get<0>(gameSum) : get<1>(gameSum);
        lostGames = gamesTotal - wonGames;

        // create column values for point data
        tuple<int, int> scoreSum = score->getScoreSum();
        int pointsTotal = get<0>(scoreSum) + get<1>(scoreSum);
        wonPoints = (playerNum == 1) ? get<0>(scoreSum) : get<1>(scoreSum);
        lostPoints = pointsTotal - wonPoints;
      }

      // add values from previous round, if required
      unique_ptr<RankingEntry> prevEntry = nullptr;
      if (!reset)
      {
        // the next call may return nullptr, but this is fine.
        // it just means that there is no old entry to build upon
        // and thus we start with the scoring at zero
        prevEntry = getRankingEntry(cat, pp, lastRound-1);
      }

      if (prevEntry != nullptr)
      {
        tuple<int, int, int, int> maStat = prevEntry->getMatchStats();
        wonMatches += get<0>(maStat);
        drawMatches += get<1>(maStat);
        lostMatches += get<2>(maStat);

        tuple<int, int, int> gameStat = prevEntry->getGameStats();
        wonGames += get<0>(gameStat);
        lostGames += get<1>(gameStat);

        tuple<int, int> pointStat = prevEntry->getPointStats();
        wonPoints += get<0>(pointStat);
        lostPoints += get<1>(pointStat);
      }

      // prep the complete data set for the entry,
      // but leave the "rank" column empty
      QVariantList qvl;
      qvl << RA_MATCHES_WON << wonMatches;
      qvl << RA_MATCHES_DRAW << drawMatches;
      qvl << RA_MATCHES_LOST << lostMatches;
      qvl << RA_GAMES_WON << wonGames;
      qvl << RA_GAMES_LOST << lostGames;
      qvl << RA_POINTS_WON << wonPoints;
      qvl << RA_POINTS_LOST << lostPoints;
      qvl << RA_PAIR_REF << pp.getPairId();
      qvl << RA_ROUND << lastRound;
      qvl << RA_CAT_REF << cat.getId();

      // create the new entry and add an instance
      // of the entry to the result list
      int newId = rankTab.insertRow(qvl);
      result.append(RankingEntry(db, newId));
    }

    if (err != nullptr) *err = OK;
    return result;
  }

//----------------------------------------------------------------------------

  unique_ptr<RankingEntry> RankingMngr::getRankingEntry(const Category &cat, const PlayerPair &pp, int round) const
  {
    QVariantList qvl;
    qvl << RA_CAT_REF << cat.getId();
    qvl << RA_PAIR_REF << pp.getPairId();
    qvl << RA_ROUND << round;

    return getSingleObjectByColumnValue<RankingEntry>(rankTab, qvl);
  }

//----------------------------------------------------------------------------

  RankingEntryList RankingMngr::getSortedRanking(const Category &cat, int round) const
  {
    QString where = RA_CAT_REF + " = " + QString::number(cat.getId());
    where += " AND " + RA_ROUND + " = " + QString::number(round);
    where += " ORDER BY " + RA_RANK + " ASC";

    return getObjectsByWhereClause<RankingEntry>(rankTab, where);
  }

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------


}
