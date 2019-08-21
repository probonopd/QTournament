/*
 *    This is QTournament, a badminton tournament management program.
 *    Copyright (C) 2014 - 2017  Volker Knollmann
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

#include <stdexcept>
#include <algorithm>

#include <SqliteOverlay/Transaction.h>
#include <QDebug>

#include "RankingMngr.h"
#include "CatRoundStatus.h"
#include "RankingEntry.h"
#include "Match.h"
#include "Score.h"
#include "MatchMngr.h"

using namespace SqliteOverlay;

namespace QTournament
{

  RankingMngr::RankingMngr(const TournamentDB& _db)
  : TournamentDatabaseObjectManager(_db, TabMatchSystem)
  {
  }

//----------------------------------------------------------------------------

  RankingEntryList RankingMngr::createUnsortedRankingEntriesForLastRound(const Category &cat, Error *err, const PlayerPairList& _ppList, bool reset)
  {
    // determine the round we should create the entries for
    CatRoundStatus crs = cat.getRoundStatus();
    int lastRound = crs.getFinishedRoundsCount();
    if (lastRound < 1)
    {
      if (err != nullptr) *err = Error::RoundNotFinished;
      return RankingEntryList();
    }

    // establish the list of player pairs for that the ranking shall
    // be created; it's either ALL pairs or a caller-provided list
    PlayerPairList ppList;
    if (_ppList.empty())
    {
      ppList = cat.getPlayerPairs();
    } else {
      ppList = _ppList;
    }

    // make sure that all matches for these player pairs have
    // valid results. We need to check this in a separate loop
    // to avoid that the ranking entries have already been
    // halfway written to the database when we encounter an invalid
    // match
    MatchMngr mm{db};
    for (PlayerPair pp : ppList)
    {
      auto ma = mm.getMatchForPlayerPairAndRound(pp, lastRound);
      if (ma.has_value())
      {
        Error e;
        auto score = ma->getScore(&e);
        if (e != Error::OK)
        {
          if (err != nullptr) *err = e;
          return RankingEntryList();
        }
        if (!score.has_value())
        {
          if (err != nullptr) *err = Error::NoMatchResultSet;
          return RankingEntryList();
        }
      }
    }

    //
    // Okay, we can be pretty sure that the rest of this method
    // succeeds and that we leave the database in a consistent state
    //

    // iterate over the player pair list and create entries
    // based on match result and, possibly, previous ranking entries
    RankingEntryList result;
    for (PlayerPair pp : ppList)
    {
      // prepare the values for the new entry
      int wonMatches = 0;
      int drawMatches = 0;
      int lostMatches = 0;
      int wonGames = 0;
      int lostGames = 0;
      int wonPoints = 0;
      int lostPoints = 0;

      // get match results, if the player played in this round
      // (maybe the player had a bye; in this case we skip this section)
      auto ma = mm.getMatchForPlayerPairAndRound(pp, lastRound);
      if (ma.has_value())
      {
        // determine whether our pp is player 1 or player 2
        int pp1Id = ma->getPlayerPair1().getPairId();
        int playerNum = (pp1Id == pp.getPairId()) ? 1 : 2;

        // create column values for match data
        Error e;
        auto score = ma->getScore(&e);
        assert(score.has_value());
        wonMatches = (score->getWinner() == playerNum) ? 1 : 0;
        lostMatches = (score->getLoser() == playerNum) ? 1 : 0;
        drawMatches = (score->getWinner() == 0) ? 1 : 0;

        // create column values for game data
        std::tuple<int, int> gameSum = score->getGameSum();
        int gamesTotal = std::get<0>(gameSum) + std::get<1>(gameSum);
        wonGames = (playerNum == 1) ? std::get<0>(gameSum) : std::get<1>(gameSum);
        lostGames = gamesTotal - wonGames;

        // create column values for point data
        std::tuple<int, int> scoreSum = score->getScoreSum();
        wonPoints = (playerNum == 1) ? std::get<0>(scoreSum) : std::get<1>(scoreSum);
        lostPoints = score->getPointsSum() - wonPoints;
      }

      // add values from previous round, if required
      std::optional<RankingEntry> prevEntry{};
      if (!reset)
      {
        // the next call may return nullptr, but this is fine.
        // it just means that there is no old entry to build upon
        // and thus we start with the scoring at zero
        prevEntry = getRankingEntry(pp, lastRound-1);
      }

      if (prevEntry.has_value())
      {
        std::tuple<int, int, int, int> maStat = prevEntry->getMatchStats();
        wonMatches += std::get<0>(maStat);
        drawMatches += std::get<1>(maStat);
        lostMatches += std::get<2>(maStat);

        std::tuple<int, int, int> gameStat = prevEntry->getGameStats();
        wonGames += std::get<0>(gameStat);
        lostGames += std::get<1>(gameStat);

        std::tuple<int, int> pointStat = prevEntry->getPointStats();
        wonPoints += std::get<0>(pointStat);
        lostPoints += std::get<1>(pointStat);
      }

      // determine the match group number for this entry
      int grpNum;
      if (ma.has_value())
      {
        // easiest and most likely case
        grpNum = ma->getMatchGroup().getGroupNumber();
      } else {
        // hmmmm, we need to determine the group number of a
        // playerPair that hasn't played in this round

        // case 1:
        // we are in some sort of round-robin round with
        // multiple match groups. In this case, the group
        // number can be derived directly from the PlayerPair
        if (mm.getMatchGroupsForCat(cat, lastRound).size() > 1)
        {
          grpNum = pp.getPairsGroupNum();
        } else {

          // case 2:
          // we are either in a round-robin phase with only
          // one group or in a KO-round or similar. So we have
          // only one match group. Thus, we can derive the
          // group number from the match group
          grpNum = mm.getMatchGroupsForCat(cat, lastRound).at(0).getGroupNumber();
        }
      }

      // prep the complete data set for the entry,
      // but leave the "rank" column empty
      ColumnValueClause cvc;
      cvc.addCol(RA_MatchesWon, wonMatches);
      cvc.addCol(RA_MatchesDraw, drawMatches);
      cvc.addCol(RA_MatchesLost, lostMatches);
      cvc.addCol(RA_GamesWon, wonGames);
      cvc.addCol(RA_GamesLost, lostGames);
      cvc.addCol(RA_PointsWon, wonPoints);
      cvc.addCol(RA_PointsLost, lostPoints);
      cvc.addCol(RA_PairRef, pp.getPairId());
      cvc.addCol(RA_Round, lastRound);
      cvc.addCol(RA_CatRef, cat.getId());  // eases searching, but is redundant information
      cvc.addCol(RA_GrpNum, grpNum); // eases searching, but is redundant information

      // create the new entry and add an instance
      // of the entry to the result list
      int newId = tab.insertRow(cvc);
      result.push_back(RankingEntry(db, newId));
    }

    if (err != nullptr) *err = Error::OK;
    return result;
  }

//----------------------------------------------------------------------------

  std::optional<RankingEntry> RankingMngr::getRankingEntry(const PlayerPair &pp, int round) const
  {
    WhereClause wc;
    wc.addCol(RA_CatRef, pp.getCategory(db)->getId());
    wc.addCol(RA_PairRef, pp.getPairId());
    wc.addCol(RA_Round, round);

    return getSingleObjectByWhereClause<RankingEntry>(wc);
  }

//----------------------------------------------------------------------------

  RankingEntryListList RankingMngr::getSortedRanking(const Category &cat, int round) const
  {
    // make sure we have ranking entries
    WhereClause wc;
    wc.addCol(RA_CatRef, cat.getId());
    wc.addCol(RA_Round, round);
    RankingEntryList rel = getObjectsByWhereClause<RankingEntry>(wc);
    if (rel.empty())
    {
      return RankingEntryListList();
    }

    RankingEntryListList result;

    // derive a list of all match groups we need to
    // retrieve
    //
    // Note: the list of applicable match groups cannot
    // be derived from tnmt->getMatchMngr()->getMatchGroupsForCat(cat, lastRound)
    // because this doesn't fetch group numbers of those
    // round-robin groups that haven't played in this round.
    // This effect occurs if we have different group sizes in the category
    // (i.e., Group 1 has already finished, Group 2 still has to play one round)
    QList<int> applicableMatchGroupNumbers;
    for (RankingEntry re : rel)
    {
      int grpNum = re.getGroupNumber();
      if (!(applicableMatchGroupNumbers.contains(grpNum)))
      {
        applicableMatchGroupNumbers.append(grpNum);
      }
    }
    std::sort(applicableMatchGroupNumbers.begin(), applicableMatchGroupNumbers.end());

    // get separate lists for every match group.
    //
    // In non-round-robin matches, this does no harm because
    // there is only one (artificial) match group in those cases
    for (int grpNum : applicableMatchGroupNumbers)
    {
      WhereClause wc;
      wc.addCol(RA_CatRef, cat.getId());
      wc.addCol(RA_Round, round);
      wc.addCol(RA_GrpNum, grpNum);
      wc.setOrderColumn_Asc(RA_GrpNum);
      wc.setOrderColumn_Asc(RA_Rank);

      result.push_back(getObjectsByWhereClause<RankingEntry>(wc));
    }

    return result;
  }

//----------------------------------------------------------------------------

  int RankingMngr::getHighestRoundWithRankingEntryForPlayerPair(const Category& cat, const PlayerPair& pp) const
  {
    WhereClause wc;
    wc.addCol(RA_CatRef, cat.getId());
    wc.addCol(RA_PairRef, pp.getPairId());
    wc.setOrderColumn_Desc(RA_Round);

    auto re = getSingleObjectByWhereClause<RankingEntry>(wc);
    if (!re.has_value()) return -1;

    return re->getRound();
  }

  //----------------------------------------------------------------------------

  Error RankingMngr::updateRankingsAfterMatchResultChange(const Match& ma, const MatchScore& oldScore, bool skipSorting) const
  {
    if (ma.is_NOT_InState(ObjState::MA_Finished)) return Error::WrongState;

    Category cat = ma.getCategory();
    int catId = cat.getId();
    int firstRoundToModify = ma.getMatchGroup().getRound();

    // determine the score differences (delta) for each affected player pair
    MatchScore newScore = *(ma.getScore());  // is guaranteed to be != nullptr
    std::tuple<int, int, int> deltaMatches_P1{0,0,0};  // to be added to PlayerPair1
    std::tuple<int, int, int> deltaMatches_P2{0,0,0};  // to be added to PlayerPair2

    int oldWinner = oldScore.getWinner();
    int newWinner = newScore.getWinner();
    if ((oldWinner == 0) && (newWinner == 1))
    {
      deltaMatches_P1 = std::tuple<int, int, int>{1, 0, -1};
      deltaMatches_P2 = std::tuple<int, int, int>{0, 1, -1};
    }
    if ((oldWinner == 0) && (newWinner == 2))
    {
      deltaMatches_P1 = std::tuple<int, int, int>{0, 1, -1};
      deltaMatches_P2 = std::tuple<int, int, int>{1, 0, -1};
    }
    if ((oldWinner == 1) && (newWinner == 0))
    {
      deltaMatches_P1 = std::tuple<int, int, int>{-1, 0, 1};
      deltaMatches_P2 = std::tuple<int, int, int>{0, -1, 1};
    }
    if ((oldWinner == 2) && (newWinner == 0))
    {
      deltaMatches_P1 = std::tuple<int, int, int>{0, -1, 1};
      deltaMatches_P2 = std::tuple<int, int, int>{-1, 0, 1};
    }
    if ((oldWinner == 1) && (newWinner == 2))
    {
      deltaMatches_P1 = std::tuple<int, int, int>{-1, 1, 0};
      deltaMatches_P2 = std::tuple<int, int, int>{1, -1, 0};
    }
    if ((oldWinner == 2) && (newWinner == 1))
    {
      deltaMatches_P1 = std::tuple<int, int, int>{1, -1, 0};
      deltaMatches_P2 = std::tuple<int, int, int>{-1, 1, 0};
    }

    std::tuple<int, int> gameSumOld = oldScore.getGameSum();
    std::tuple<int, int> gameSumNew = newScore.getGameSum();
    int gamesTotalOld = std::get<0>(gameSumOld) + std::get<1>(gameSumOld);
    int gamesTotalNew = std::get<0>(gameSumNew) + std::get<1>(gameSumNew);

    int deltaWonGamesP1 = -std::get<0>(gameSumOld) + std::get<0>(gameSumNew);
    int deltaLostGamesP1 = -(gamesTotalOld - std::get<0>(gameSumOld)) + (gamesTotalNew - std::get<0>(gameSumNew));
    int deltaWonGamesP2 = -std::get<1>(gameSumOld) + std::get<1>(gameSumNew);
    int deltaLostGamesP2 = -(gamesTotalOld - std::get<1>(gameSumOld)) + (gamesTotalNew - std::get<1>(gameSumNew));
    std::tuple<int, int> deltaGames_P1{deltaWonGamesP1, deltaLostGamesP1};  // to be added to PlayerPair1
    std::tuple<int, int> deltaGames_P2{deltaWonGamesP2, deltaLostGamesP2};  // to be added to PlayerPair2

    std::tuple<int, int> scoreSumOld = oldScore.getScoreSum();
    std::tuple<int, int> scoreSumNew = newScore.getScoreSum();
    int oldWonPoints_P1 = std::get<0>(scoreSumOld);
    int newWonPoints_P1 = std::get<0>(scoreSumNew);
    int deltaWonPoints_P1 = newWonPoints_P1 - oldWonPoints_P1;
    int oldLostPoints_P1 = oldScore.getPointsSum() - oldWonPoints_P1;
    int newLostPoints_P1 = newScore.getPointsSum() - newWonPoints_P1;
    int deltaLostPoints_P1 = newLostPoints_P1 - oldLostPoints_P1;
    std::tuple<int, int> deltaPoints_P1{deltaWonPoints_P1, deltaLostPoints_P1};
    std::tuple<int, int> deltaPoints_P2{deltaLostPoints_P1, deltaWonPoints_P1};

    // determine who actually is P1 and P2
    int pp1Id = ma.getPlayerPair1().getPairId();
    int pp2Id = ma.getPlayerPair2().getPairId();

    // find the first entry to modify

    // derive the group number of the affected ranking entries
    //
    // we may only modify subsequent entries with the same
    // group number as the initial number. Thus, we prevent
    // a modification of e.g. the ranking entries in a KO-phase
    // after we started modifications in the round robin phase.
    //
    // we get the group number from the first entry of the
    // first player pair to be modified
    WhereClause w;
    w.addCol(RA_CatRef, catId);
    w.addCol(RA_PairRef, pp1Id);
    w.addCol(RA_Round, firstRoundToModify);
    auto re = getSingleObjectByWhereClause<RankingEntry>(w);
    if (!re.has_value()) return Error::OK;  // no ranking entries yet
    int grpNum = re->getGroupNumber();

    //
    // a helper function that does the actual modification
    //
    auto doMod = [&](int pairId, const std::tuple<int, int, int>& matchDelta,
                     const std::tuple<int, int>& gamesDelta, const std::tuple<int, int>& pointsDelta)
    {
      // let's build a where clause that captures all entries
      // to modified
      w.clear();
      w.addCol(RA_CatRef, catId);
      w.addCol(RA_PairRef, pairId);
      w.addCol(RA_Round, ">=", firstRoundToModify);
      if (grpNum > 0)
      {
        w.addCol(RA_GrpNum, grpNum);   // a dedicated group number (1, 2, 3...)
      } else {
        w.addCol(RA_GrpNum, "<", 0);   // a functional number (iteration, quarter finals, ...)
      }

      auto rowsList = tab.getRowsByWhereClause(w);
      for (const auto& r : rowsList)
      {
        std::vector<std::tuple <std::string, int>> colDelta = {
          {RA_MatchesWon, std::get<0>(matchDelta)},
          {RA_MatchesLost, std::get<1>(matchDelta)},
          {RA_MatchesDraw, std::get<2>(matchDelta)},
          {RA_GamesWon, std::get<0>(gamesDelta)},
          {RA_GamesLost, std::get<1>(gamesDelta)},
          {RA_PointsWon, std::get<0>(pointsDelta)},
          {RA_PointsLost, std::get<1>(pointsDelta)},
        };

        for (auto [colName, deltaVal] : colDelta)
        {
          int oldVal = r.getInt(colName);
          r.update(colName, oldVal + deltaVal);
        }
      }
    };
    //------------------------- end of helper func -------------------

    try
    {
      // start a new transaction to make sure that
      // the database remains consistent in case something goes wrong
      auto trans = db.get().startTransaction(DefaultTransactionType);

      // modify the ranking entries
      doMod(pp1Id, deltaMatches_P1, deltaGames_P1, deltaPoints_P1);
      doMod(pp2Id, deltaMatches_P2, deltaGames_P2, deltaPoints_P2);

      // now we have to re-sort the entries, round by round
      // UNLESS the caller decided to skip the sorting.
      //
      // skipping the sorting (and thus assigning ranks) is only
      // usefull in bracket matches where ranks are not derived
      // from points but from bracket logic
      if (!skipSorting)
      {
        auto specializedCat = cat.convertToSpecializedObject();
        auto lessThanFunc = specializedCat->getLessThanFunction();
        int round = firstRoundToModify;
        while (true)
        {
          w.clear();
          w.addCol(RA_CatRef, catId);
          w.addCol(RA_Round, round);
          w.addCol(RA_GrpNum, grpNum);

          // get the ranking entries
          RankingEntryList rankList = getObjectsByWhereClause<RankingEntry>(w);
          if (rankList.empty()) break;   // no more rounds to modify

          // call the standard sorting algorithm
          std::sort(rankList.begin(), rankList.end(), lessThanFunc);

          // write the sort results back to the database
          int rank = 1;
          for (RankingEntry re : rankList)
          {
            re.rowRef().update(RA_Rank, rank);
            ++rank;
          }

          ++round;
        }
      }
    }
    catch (SqliteOverlay::BusyException&)
    {
      return Error::DatabaseError;
    }
    catch (SqliteOverlay::GenericSqliteException&)
    {
      return Error::DatabaseError;
    }
    catch (...)
    {
      throw;
    }

    return Error::OK;
  }

  //----------------------------------------------------------------------------

  std::string RankingMngr::getSyncString(const std::vector<int>& rows) const
  {
    std::vector<Sloppy::estring> cols = {"id", RA_Round, RA_PairRef, RA_CatRef, RA_GrpNum, RA_GamesWon, RA_GamesLost,
                          RA_MatchesWon, RA_MatchesLost, RA_MatchesDraw, RA_PointsWon, RA_PointsLost, RA_Rank};

    return db.get().getSyncStringForTable(TabMatchSystem, cols, rows);
  }

//----------------------------------------------------------------------------

  RankingEntryListList RankingMngr::sortRankingEntriesForLastRound(const Category& cat, Error* err) const
  {
    // determine the round we should create the entries for
    CatRoundStatus crs = cat.getRoundStatus();
    int lastRound = crs.getFinishedRoundsCount();
    if (lastRound < 1)
    {
      if (err != nullptr) *err = Error::RoundNotFinished;
      return RankingEntryListList();
    }

    // make sure we have (unsorted) ranking entries
    WhereClause wc;
    wc.addCol(RA_CatRef, cat.getId());
    wc.addCol(RA_Round, lastRound);
    RankingEntryList rel = getObjectsByWhereClause<RankingEntry>(wc);
    if (rel.empty())
    {
      if (err != nullptr) *err = Error::MissingRankingEntries;
      return RankingEntryListList();
    }

    // derive a list of all match groups we need to
    // sort
    //
    // Note: the list of applicable match groups cannot
    // be derived from tnmt->getMatchMngr()->getMatchGroupsForCat(cat, lastRound)
    // because this doesn't fetch group numbers of those
    // round-robin groups that haven't played in this round.
    // This effect occurs if we have different group sizes in the category
    // (i.e., Group 1 has already finished, Group 2 still has to play one round)
    QList<int> applicableMatchGroupNumbers;
    for (RankingEntry re : rel)
    {
      int grpNum = re.getGroupNumber();
      if (!(applicableMatchGroupNumbers.contains(grpNum)))
      {
        applicableMatchGroupNumbers.append(grpNum);
      }
    }

    // get the category-specific comparison function
    auto specializedCat = cat.convertToSpecializedObject();
    auto lessThanFunc = specializedCat->getLessThanFunction();

    // prepare the result object
    RankingEntryListList result;

    // apply separate sorting for every match group.
    //
    // In non-round-robin matches, this does no harm because
    // there is only one (artificial) match group in those cases
    for (int grpNum : applicableMatchGroupNumbers)
    {
      WhereClause wcWithGroupNum = wc;
      wcWithGroupNum.addCol(RA_GrpNum, grpNum);

      // get the ranking entries
      RankingEntryList rankList = getObjectsByWhereClause<RankingEntry>(wcWithGroupNum);

      // call the standard sorting algorithm
      std::sort(rankList.begin(), rankList.end(), lessThanFunc);

      // write the sort results back to the database
      int rank = 1;
      for (RankingEntry re : rankList)
      {
        re.rowRef().update(RA_Rank, rank);
        ++rank;
      }

      // add the sorted group list to the result
      result.push_back(rankList);
    }

    if (err != nullptr) *err = Error::OK;
    return result;
  }

//----------------------------------------------------------------------------

  Error RankingMngr::forceRank(const RankingEntry& re, int rank) const
  {
    if (rank < 1) return Error::InvalidRank;

    re.rowRef().update(RA_Rank, rank);

    return Error::OK;
  }

  //----------------------------------------------------------------------------

  Error RankingMngr::clearRank(const RankingEntry& re) const
  {
    re.rowRef().updateToNull(RA_Rank);

    return Error::OK;
  }

//----------------------------------------------------------------------------

  void RankingMngr::fillRankGaps(const Category& cat, int round, int maxRank)
  {
    int catId = cat.getId();

    // a little helper function for creating a dummy ranking entry
    auto insertRankingEntry = [&](int g, int r) {
      ColumnValueClause cvc;
      cvc.addCol(RA_MatchesWon, -1);
      cvc.addCol(RA_MatchesDraw, -1);
      cvc.addCol(RA_MatchesLost, -1);
      cvc.addCol(RA_GamesWon, -1);
      cvc.addCol(RA_GamesLost, -1);
      cvc.addCol(RA_PointsWon, -1);
      cvc.addCol(RA_PointsLost, -1);
      cvc.addNullCol(RA_PairRef);
      cvc.addCol(RA_Round, round);
      cvc.addCol(RA_CatRef, catId);
      cvc.addCol(RA_GrpNum, g);
      cvc.addCol(RA_Rank, r);
      tab.insertRow(cvc);
    };

    RankingEntryListList rll = getSortedRanking(cat, round);
    if (rll.empty()) return;

    // for each match group, go through the
    // list of ranking entries and fill gaps
    // or append entries up to maxRank
    for (RankingEntryList rl : rll)
    {
      int lastSeenRank = 0;
      int grpNum = rl.at(0).getGroupNumber();

      for (RankingEntry re : rl)
      {
        int curRank = re.getRank();
        if (curRank == RankingEntry::NoRankAssigned) continue;

        // fill gaps, e.g., insert a dummy rank 4 and 5 between existing,
        // "real" entries 3 and 6
        while (curRank > (lastSeenRank+1))
        {
          ++lastSeenRank;
          insertRankingEntry(grpNum, lastSeenRank);
        }
        lastSeenRank = curRank;
      }

      // append entries up to maxRank
      while (lastSeenRank < maxRank)
      {
        ++lastSeenRank;
        insertRankingEntry(grpNum, lastSeenRank);
      }
    }
  }

//----------------------------------------------------------------------------

  std::optional<RankingEntry> RankingMngr::getRankingEntry(const Category& cat, int round, int grpNum, int rank) const
  {
    WhereClause wc;
    wc.addCol(RA_CatRef, cat.getId());
    wc.addCol(RA_Round, round);
    wc.addCol(RA_GrpNum, grpNum);
    wc.addCol(RA_Rank, rank);

    return getSingleObjectByWhereClause<RankingEntry>(wc);
  }

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------


}
