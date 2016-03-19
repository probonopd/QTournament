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

#include <QHash>

#include "SwissLadderCategory.h"
#include "KO_Config.h"
#include "Tournament.h"
#include "CatRoundStatus.h"
#include "RankingEntry.h"
#include "RankingMngr.h"
#include "assert.h"
#include "BracketGenerator.h"


#include <QDebug>

using namespace SqliteOverlay;

namespace QTournament
{

  SwissLadderCategory::SwissLadderCategory(TournamentDB* db, int rowId)
  : Category(db, rowId)
  {
  }

//----------------------------------------------------------------------------

  SwissLadderCategory::SwissLadderCategory(TournamentDB* db, SqliteOverlay::TabRow row)
  : Category(db, row)
  {
  }

//----------------------------------------------------------------------------

  bool SwissLadderCategory::genMatchesForNextRound() const
  {
    // determine the ranking after the last finished round
    int lastRound = getRoundStatus().getFinishedRoundsCount();
    if (lastRound < 0)
    {
      lastRound = 0;
    }

    PlayerPairList rankedPairs;
    QList<int> rankedPairs_Int;
    auto tnmt = Tournament::getActiveTournament();
    if (lastRound == 0)
    {
      rankedPairs = tnmt->getCatMngr()->getSeeding(*this);
    } else {
      RankingMngr* rm = tnmt->getRankingMngr();
      auto rll = rm->getSortedRanking(*this, lastRound);
      assert(rll.size() == 1);
      auto ranking = rll.at(0);
      for (RankingEntry re : ranking)
      {
        PlayerPair pp = *(re.getPlayerPair());
        rankedPairs.push_back(pp);
      }
    }
    for (PlayerPair pp : rankedPairs)
    {
      rankedPairs_Int.push_back(pp.getPairId());
    }
    int pairCount = rankedPairs.size();
    assert(pairCount == getPlayerPairs().size());

    // store a list of ALL matches that have been played
    // in the past in this category. Keep the list in
    // memory for fast and easy access later on. We'll use
    // this list later to avoid playing the same match twice
    // in different rounds.
    //
    // While we're walking through all matches, count
    // the number of matches for each player. We need this
    // value to figure out who is going to have the next
    // bye in case of an off number of players
    MatchMngr* mm = tnmt->getMatchMngr();
    QStringList pastMatches;
    QHash<int, int> pairId2matchCount;
    for (int id : rankedPairs_Int)
    {
      pairId2matchCount.insert(id, 0);
    }
    for (MatchGroup mg : mm->getMatchGroupsForCat(*this))
    {
      for (Match ma : mg.getMatches())
      {
        // ignore unfinished matches. However, there
        // shouldn't be any when this function is called
        if (ma.getState() != STAT_MA_FINISHED)
        {
          continue;
        }

        // store the match as CSV-strings of the
        // playerpair combinations
        int pp1Id = ma.getPlayerPair1().getPairId();
        int pp2Id = ma.getPlayerPair2().getPairId();
        QString matchString1 = QString("%1,%2").arg(pp1Id).arg(pp2Id);
        QString matchString2 = QString("%1,%2").arg(pp2Id).arg(pp1Id);
        pastMatches.append(matchString1);
        pastMatches.append(matchString2);

        // increment the match counter for each player pair
        int oldCount = pairId2matchCount.take(pp1Id);
        pairId2matchCount.insert(pp1Id, ++oldCount);
        oldCount = pairId2matchCount.take(pp2Id);
        pairId2matchCount.insert(pp2Id, ++oldCount);
      }
    }

    // if necessary, determine the player with the next bye. This
    // is basically the lowest ranked player unless this player
    // already had a bye in a preious round.
    // In this case, it's the second-to-lowest ranked and so on
    if ((pairCount % 2) != 0)
    {
      int byeIndex = pairCount - 1;   // start with the lowest ranked player
      while (true)
      {
        int byePairId = rankedPairs_Int.at(byeIndex);
        int matchCount = pairId2matchCount.value(byePairId);
        if (matchCount == lastRound)  // values identical means: no bye so far
        {
          break;
        }
        --byeIndex;
      }

      // remove the found player pair from all lists so that it's
      // not part of all further algorithms
      rankedPairs.erase(rankedPairs.begin() + byeIndex);
      rankedPairs_Int.erase(rankedPairs_Int.begin() + byeIndex);
      --pairCount;
    }

    // a helper function that determines the next
    // index of an unused player pair
    QList<int> usedPairs;
    auto findNextUnusedPairIndex = [&](int minIndex)
    {
      int result = minIndex;
      while (minIndex < pairCount)
      {
        int pairId = rankedPairs_Int.at(result);
        if (!(usedPairs.contains(pairId)))
        {
          return result;
        }
        ++result;
      }
      return -1;
    };

    // start building new player combinations. Follow the approach
    // "first against second", "third against fourth", etc. but avoid
    // playing the same match twice.
    //
    // the algorithm uses two indices: one for the first player, the
    // other one for the second player.
    QStringList newMatches;
    int firstIndex = 0;
    int secondIndex = 1;
    while ((pairCount - usedPairs.size()) > 1)   // one pair may be unsed in categories with an odd number of participants
    {
      // determine the smallest two indexes of unused players
      if (firstIndex < 0)
      {
        // find the next two unused players
        firstIndex = findNextUnusedPairIndex(0);
        assert(firstIndex >= 0);
        secondIndex = findNextUnusedPairIndex(firstIndex + 1);
        assert(secondIndex > firstIndex);
      }

      // keep the first pair fix while searching for
      // a second pair that results in a unique match
      while ((secondIndex > 0) && (secondIndex < pairCount))
      {
        // create a CSV-string with the player pair IDs making up the match
        QString newMatchString = "%1,%2";
        newMatchString = newMatchString.arg(rankedPairs_Int.at(firstIndex));
        newMatchString = newMatchString.arg(rankedPairs_Int.at(secondIndex));

        // check if this match has been played before
        if (pastMatches.contains(newMatchString))
        {
          secondIndex = findNextUnusedPairIndex(++secondIndex);  // try next combination
          continue;
        }

        // no, the match has not been played
        newMatches.append(newMatchString);

        // mark the involved players as "used"
        usedPairs.append(rankedPairs_Int.at(firstIndex));
        usedPairs.append(rankedPairs_Int.at(secondIndex));

        // set firstIndex to -1 to indicate "match found"
        firstIndex = -1;
        break;
      }

      while ((firstIndex > 0) && (secondIndex < 0))
      {
        // oh well, we are in trouble... we couldn't find
        // a match combination that hasn't been played before
        //
        // this means, we have to go back one match and find
        // a new partner for that previous match. This frees
        // up a formerly used player pair and hopefully results
        // in a better player pair combination
        assert(newMatches.size() > 0);  // we MUST have at least one match at this point

        QString lastMatch = newMatches.takeLast();
        int pp1Id = lastMatch.split(",").at(0).toInt();
        int pp2Id = lastMatch.split(",").at(1).toInt();

        // remove the two player pairs from the list of used players
        usedPairs.removeAll(pp1Id);
        usedPairs.removeAll(pp2Id);

        // find the corresponding indices for the two pairs
        firstIndex = rankedPairs_Int.indexOf(pp1Id);
        secondIndex = rankedPairs_Int.indexOf(pp2Id);

        // continue with the next index beyond the previously
        // determined partner pair (read: skip the previously
        // found solution)
        secondIndex = findNextUnusedPairIndex(++secondIndex);

        // if this search for a new "secondIndex" does not
        // yield any result, we continue in our while loop
        // and remove the next match from the list of solutions
      }
    }

    // if we ever reach this point, newMatches contains a list of
    // strings that define the matches for the next round
    //
    // let's fill the already prepared, "empty" matches with the
    // match information from newMatches
    ERR e;
    auto mg = mm->getMatchGroup(*this, lastRound+1, GROUP_NUM__ITERATION, &e);
    assert(mg != nullptr);
    assert(e == OK);
    assert(mg->getMatches().size() == newMatches.size());
    int cnt = 0;
    for (Match ma : mg->getMatches())
    {
      QString matchString = newMatches.at(cnt);
      int pp1Id = matchString.split(",").at(0).toInt();
      int pp2Id = matchString.split(",").at(1).toInt();
      PlayerPair pp1 = tnmt->getPlayerMngr()->getPlayerPair(pp1Id);
      PlayerPair pp2 = tnmt->getPlayerMngr()->getPlayerPair(pp2Id);

      e = mm->setPlayerPairsForMatch(ma, pp1, pp2);
      assert(e == OK);
      ++cnt;
    }

    return true;
  }

//----------------------------------------------------------------------------

  ERR SwissLadderCategory::canFreezeConfig()
  {
    if (getState() != STAT_CAT_CONFIG)
    {
      return CONFIG_ALREADY_FROZEN;
    }
    
    // make sure there no unpaired players in singles or doubles
    if ((getMatchType() != SINGLES) && (hasUnpairedPlayers()))
    {
      return UNPAIRED_PLAYERS;
    }
    
    // make sure we have at least three players
    PlayerPairList pp = getPlayerPairs();
    if (pp.size() < 3)
    {
      return INVALID_PLAYER_COUNT;
    }
    
    return OK;
  }

//----------------------------------------------------------------------------

  bool SwissLadderCategory::needsInitialRanking()
  {
    return true;
  }

//----------------------------------------------------------------------------

  bool SwissLadderCategory::needsGroupInitialization()
  {
    return false;
  }

//----------------------------------------------------------------------------

  ERR SwissLadderCategory::prepareFirstRound(ProgressQueue *progressNotificationQueue)
  {
    if (getState() != STAT_CAT_IDLE) return WRONG_STATE;

    auto tnmt = Tournament::getActiveTournament();
    auto mm = tnmt->getMatchMngr();

    // make sure we have not been called before; to this end, just
    // check that there have no matches been created for us so far
    auto allGrp = mm->getMatchGroupsForCat(*this);
    // do not return an error here, because obviously we have been
    // called successfully before and we only want to avoid
    // double initialization
    if (allGrp.size() != 0) return OK;

    // alright, this is a virgin category.
    // Assume that we play all possible rounds (very much like a
    // round-robin) and generate all match groups along with
    // placeholder matches
    int nRounds = calcTotalRoundsCount();
    int nPairs = getPlayerPairs().size();
    int nMatchesPerRound = nPairs / 2;   // this always rounds down

    for (int r=1; r <= nRounds; ++r)
    {
      ERR e;
      auto mg = mm->createMatchGroup(*this, r, GROUP_NUM__ITERATION, &e);
      assert(mg != nullptr);
      assert(e == OK);

      for (int m=0; m < nMatchesPerRound; ++m)
      {
        auto ma = mm->createMatch(*mg, &e);
        assert(ma != nullptr);
        assert(e == OK);
      }

      mm->closeMatchGroup(*mg);
    }

    // Fill the first round of matches based on the initial seeding
    genMatchesForNextRound();

    return OK;
  }

//----------------------------------------------------------------------------

  int SwissLadderCategory::calcTotalRoundsCount() const
  {
    OBJ_STATE stat = getState();
    if ((stat == STAT_CAT_CONFIG) || (stat == STAT_CAT_FROZEN))
    {
      return -1;   // category not yet fully configured; can't calc rounds
    }

    PlayerPairList allPairs = getPlayerPairs();
    int nPairs = allPairs.size();

    // the max number of rounds is equivalent to a full round-robin
    return ((nPairs % 2) == 0) ? (nPairs-1) : nPairs;
  }

//----------------------------------------------------------------------------

  // this return a function that should return true if "a" goes before "b" when sorting. Read:
  // return a function that return true true if the score of "a" is better than "b"
  std::function<bool (RankingEntry& a, RankingEntry& b)> SwissLadderCategory::getLessThanFunction()
  {
    return [](RankingEntry& a, RankingEntry& b) {
      // first criterion: delta between won and lost matches
      tuple<int, int, int, int> matchStatsA = a.getMatchStats();
      tuple<int, int, int, int> matchStatsB = b.getMatchStats();
      int deltaA = get<0>(matchStatsA) - get<2>(matchStatsA);
      int deltaB = get<0>(matchStatsB) - get<2>(matchStatsB);
      if (deltaA > deltaB) return true;
      if (deltaA < deltaB) return false;

      // second criteria: delta between won and lost games
      tuple<int, int, int> gameStatsA = a.getGameStats();
      tuple<int, int, int> gameStatsB = b.getGameStats();
      deltaA = get<0>(gameStatsA) - get<1>(gameStatsA);
      deltaB = get<0>(gameStatsB) - get<1>(gameStatsB);
      if (deltaA > deltaB) return true;
      if (deltaA < deltaB) return false;

      // second criteria: delta between won and lost points
      tuple<int, int> pointStatsA = a.getPointStats();
      tuple<int, int> pointStatsB = b.getPointStats();
      deltaA = get<0>(pointStatsA) - get<1>(pointStatsA);
      deltaB = get<0>(pointStatsB) - get<1>(pointStatsB);
      if (deltaA > deltaB) return true;
      if (deltaA < deltaB) return false;

      // TODO: add a direct comparison as additional criteria?

      // Default: "a" is not strictly better than "b", so we return false
      return false;
    };
  }

//----------------------------------------------------------------------------

  ERR SwissLadderCategory::onRoundCompleted(int round)
  {
    auto tnmt = Tournament::getActiveTournament();
    RankingMngr* rm = tnmt->getRankingMngr();
    ERR err;
    PlayerPairList allPairs = getPlayerPairs();

    rm->createUnsortedRankingEntriesForLastRound(*this, &err, allPairs);
    if (err != OK) return err;  // shouldn't happen
    rm->sortRankingEntriesForLastRound(*this, &err);
    if (err != OK) return err;  // shouldn't happen

    if (round != calcTotalRoundsCount())
    {
      genMatchesForNextRound();
    }

    return OK;
  }

//----------------------------------------------------------------------------

  PlayerPairList SwissLadderCategory::getRemainingPlayersAfterRound(int round, ERR* err) const
  {
    // No knock-outs, never
    if (err != nullptr) *err = OK;
    return getPlayerPairs();
  }

//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


}
