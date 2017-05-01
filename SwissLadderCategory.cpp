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

#include <QHash>
#include <QDebug>

#include "SwissLadderCategory.h"
#include "KO_Config.h"
#include "CatRoundStatus.h"
#include "RankingEntry.h"
#include "RankingMngr.h"
#include "assert.h"
#include "BracketGenerator.h"
#include "CatMngr.h"
#include "PlayerMngr.h"
#include "CentralSignalEmitter.h"

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
    CatMngr cm{db};
    if (lastRound == 0)
    {
      rankedPairs = cm.getSeeding(*this);
    } else {
      RankingMngr rm{db};
      auto rll = rm.getSortedRanking(*this, lastRound);
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

    // do we need a deadlock prevention check?
    //
    // This check is necessary when generating the matches
    // for the round "nPairs - 3" in order to prevent
    // deadlock AFTER playing that round
    int maxRounds = ((pairCount % 2) == 0) ? pairCount - 1 : pairCount;
    int nextRound = lastRound + 1;
    bool needsDeadlockPrevention = (nextRound == (maxRounds - 3));

    // store a list of ALL matches that have been played
    // in the past in this category. Keep the list in
    // memory for fast and easy access later on. We'll use
    // this list later to avoid playing the same match twice
    // in different rounds.
    //
    // While we're walking through all matches, count
    // the number of matches for each player. We need this
    // value to figure out who is going to have the next
    // bye in case of an odd number of players
    MatchMngr mm{db};
    QStringList pastMatches;
    QHash<int, int> pairId2matchCount;
    for (int id : rankedPairs_Int)
    {
      pairId2matchCount.insert(id, 0);
    }
    for (MatchGroup mg : mm.getMatchGroupsForCat(*this))
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
    // already had a bye in a previous round.
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
    auto findNextUnusedPairRank = [&](int minRank)
    {
      int rank = minRank;
      while (rank < pairCount)
      {
        int pairId = rankedPairs_Int.at(rank);
        if (!(usedPairs.contains(pairId)))
        {
          return rank;
        }
        ++rank;
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
    int pair1Rank = 0;
    int pair2Rank = 1;
    bool isDeadlock = false;
    while ((pairCount - usedPairs.size()) > 1)   // one pair may be unsed in categories with an odd number of participants
    {
      // determine the smallest two indexes of unused players
      if (pair1Rank < 0)
      {
        // find the next two unused players
        pair1Rank = findNextUnusedPairRank(0);
        assert(pair1Rank >= 0);
        pair2Rank = findNextUnusedPairRank(pair1Rank + 1);
        assert(pair2Rank > pair1Rank);
      }

      // keep the first pair fix while searching for
      // a second pair that results in a unique match
      while ((pair2Rank > 0) && (pair2Rank < pairCount))
      {
        // create a CSV-string with the player pair IDs making up the match
        QString newMatchString = "%1,%2";
        newMatchString = newMatchString.arg(rankedPairs_Int.at(pair1Rank));
        newMatchString = newMatchString.arg(rankedPairs_Int.at(pair2Rank));

        // check if this match has been played before
        if (pastMatches.contains(newMatchString))
        {
          pair2Rank = findNextUnusedPairRank(++pair2Rank);  // try next combination
          continue;
        }

        // no, the match has not been played
        newMatches.append(newMatchString);

        // if we've found the last match for the next round, we
        // need to make sure that the selected combination of matches
        // does not lead to a deadlock AFTER playing the next round
        //
        // we only need to perform this check before the third to last round
        if (needsDeadlockPrevention && (newMatches.size() == (pairCount % 2)))
        {
          if (!(deadlockPreventionCheck(pastMatches, newMatches)))
          {
            newMatches.pop_back();
            continue;
          }
        }

        // mark the involved players as "used"
        usedPairs.append(rankedPairs_Int.at(pair1Rank));
        usedPairs.append(rankedPairs_Int.at(pair2Rank));

        // set firstIndex to -1 to indicate "match found"
        pair1Rank = -1;
        break;
      }

      // if we weren't able to find a partner for the player pair
      // ranked at position one, we've exhausted all possible
      // match combinations without success.
      //
      // In this case, our match combinations in previous
      // rounds were so unlucky that we can't establish another
      // round without repeating an already played match.
      //
      // This ONLY happens after round "nPairs - 3" (1-based counting)
      if ((pair1Rank == 0) && (pair2Rank < 0))
      {
        isDeadlock = true;
        break;
      }

      while ((pair1Rank > 0) && (pair2Rank < 0))
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
        pair1Rank = rankedPairs_Int.indexOf(pp1Id);
        pair2Rank = rankedPairs_Int.indexOf(pp2Id);

        // continue with the next index beyond the previously
        // determined partner pair (read: skip the previously
        // found solution)
        pair2Rank = findNextUnusedPairRank(++pair2Rank);

        // if this search for a new "secondIndex" does not
        // yield any result, we continue in our while loop
        // and remove the next match from the list of solutions
      }
    }

    // if we encountered a deadlock, remove all prepared future
    // matches and match groups and then we're done
    if (isDeadlock)
    {
      handleDeadlock();   // no result checking here for now
      return true;
    }

    // if we ever reach this point, newMatches contains a list of
    // strings that define the matches for the next round
    //
    // let's fill the already prepared, "empty" matches with the
    // match information from newMatches
    ERR e;
    auto mg = mm.getMatchGroup(*this, lastRound+1, GROUP_NUM__ITERATION, &e);
    assert(mg != nullptr);
    assert(e == OK);
    assert(mg->getMatches().size() == newMatches.size());
    int cnt = 0;
    for (Match ma : mg->getMatches())
    {
      QString matchString = newMatches.at(cnt);
      int pp1Id = matchString.split(",").at(0).toInt();
      int pp2Id = matchString.split(",").at(1).toInt();
      PlayerMngr pm{db};
      PlayerPair pp1 = pm.getPlayerPair(pp1Id);
      PlayerPair pp2 = pm.getPlayerPair(pp2Id);

      e = mm.setPlayerPairsForMatch(ma, pp1, pp2);
      assert(e == OK);
      ++cnt;
    }

    return true;
  }

  //----------------------------------------------------------------------------

  ERR SwissLadderCategory::handleDeadlock() const
  {
    // genMatchesForNextRound() has found that there NO POSSIBLE combination
    // of matches for the next round without repeating an already played
    // match. ==> Deadlock.
    //
    // As a consequence, we have to terminate the category here.
    //
    // We delete all matches and match groups for the remaining
    // rounds which implicitly shifts the category to COMPLETED after
    // we're done here

    int catId = getId();

    // step 1: un-stage all staged match groups of this category
    MatchMngr mm{db};
    auto stagedMatchGroups = mm.getStagedMatchGroupsOrderedBySequence();
    for (const MatchGroup& mg : stagedMatchGroups)
    {
      if (mg.getCategory().getId() != catId) continue;

      ERR err = mm.unstageMatchGroup(mg);
      if (err != OK) return err;   // shouldn't happen
    }

    // step 2: tell everyone that something baaaad is about to happen
    CentralSignalEmitter* cse = CentralSignalEmitter::getInstance();
    cse->beginResetAllModels();

    //
    // now the actual deletion starts
    //

    // deletion 1: bracket vis data, because it has only outgoing refs
    //
    // not necessary here because Swiss Ladder does not have any bracket vis data

    // deletion 2: ranking data, because it has only outgoing refs
    //
    // not necessary here because we only delete matches for future rounds
    // that do not yet have any ranking data assigned

    // deletion 3a: matches of future match groups (rounds > last finished round; equivalent to: groups that are not yet FINISHED)
    // deletion 3b: match groups for these future matches
    for (const MatchGroup& mg : mm.getMatchGroupsForCat(*this))
    {
      if (mg.getState() != STAT_MG_FINISHED)
      {
        mm.deleteMatchGroupAndMatch(mg);
      }
    }

    //
    // deletion completed
    //

    // refresh all models and the reports tab
    cse->endResetAllModels();

    // update the category status to FINISHED
    CatMngr cm{db};
    cm.updateCatStatusFromMatchStatus(*this);

    return OK;
  }

  //----------------------------------------------------------------------------

  bool SwissLadderCategory::deadlockPreventionCheck(const QStringList& pastMatches, const QStringList& nextMatches) const
  {
    // check whether the selection of next matches causes
    // a deadlock after playing those played matches in the next
    // round


    // Algorithm:
    //
    // Step 1: determine all matches for this category (means: all player pair combinations)
    // Step 2: subtract what has been played in the previous rounds (pastMatches)
    // Step 3: subtract what is to be played in the next round (nextMatches)
    // Step 4: check if the remaining matches allow for at least one more round

    //
    // Step 1: determine all matches
    //
    PlayerPairList ppList = getPlayerPairs();
    QStringList allMatches;
    for (int idxFirst = 0; idxFirst < (ppList.size() - 1); ++idxFirst)
    {
      int idFirst = ppList.at(idxFirst).getPairId();

      for (int idxSecond = idxFirst + 1; idxSecond < ppList.size(); ++idxSecond)
      {
        int idSecond = ppList.at(idxSecond).getPairId();

        QString m = "%1,%2";
        m = m.arg(idFirst);
        m = m.arg(idSecond);
        allMatches.push_back(m);
      }
    }

    QStringList remainingMatches = allMatches;

    //
    // Step 2: subtract already played matches
    //
    // Note: in pastMatches, every match occurs twice: as "a,b" and "b,a"
    for (const QString& m : pastMatches)
    {
      if (remainingMatches.contains(m)) remainingMatches.removeAll(m);
    }

    //
    // Step 3: subtract next matches
    //
    // Note: to ensure to catch all applicable matches, we convert all entries
    // in nextMatches from "a,b" to "b,a" and subtract the latter form as well
    for (const QString& m : nextMatches)
    {
      QString pp1Id = m.split(",").at(0);
      QString pp2Id = m.split(",").at(1);
      QString swapped = pp2Id + "," + pp1Id;

      if (remainingMatches.contains(m)) remainingMatches.removeAll(m);
      if (remainingMatches.contains(swapped)) remainingMatches.removeAll(swapped);
    }

    //
    // Step 4: check if we can create at least one more round from the
    // remaining matches
    //

    // FIX: to be implemented!!

    // dummy return value
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

    MatchMngr mm{db};

    // make sure we have not been called before; to this end, just
    // check that there have no matches been created for us so far
    auto allGrp = mm.getMatchGroupsForCat(*this);
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
      auto mg = mm.createMatchGroup(*this, r, GROUP_NUM__ITERATION, &e);
      assert(mg != nullptr);
      assert(e == OK);

      for (int m=0; m < nMatchesPerRound; ++m)
      {
        auto ma = mm.createMatch(*mg, &e);
        assert(ma != nullptr);
        assert(e == OK);
      }

      mm.closeMatchGroup(*mg);
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

    // primary choice: determine the number of rounds
    // from the number of created match groups. This approach
    // also covers the case that we have to reduce the number
    // of possible rounds and thus the number of match groups
    // in case we've encountered a deadlock after the third
    // to last round
    MatchMngr mm{db};
    auto mgList = mm.getMatchGroupsForCat(*this);
    if (mgList.size() > 0) return mgList.size();

    // there might be cases in early phases of a category,
    // when this function is called but match groups have not
    // yet been created. In this case, we determine the number
    // of rounds from the number of player pairs in this category

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
    RankingMngr rm{db};
    ERR err;
    PlayerPairList allPairs = getPlayerPairs();

    rm.createUnsortedRankingEntriesForLastRound(*this, &err, allPairs);
    if (err != OK) return err;  // shouldn't happen
    rm.sortRankingEntriesForLastRound(*this, &err);
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
