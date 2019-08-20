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
#include "SwissLadderGenerator.h"

using namespace std;
using namespace SqliteOverlay;

namespace QTournament
{

  SwissLadderCategory::SwissLadderCategory(const TournamentDB& _db, int rowId)
  : Category(_db, rowId)
  {
  }

//----------------------------------------------------------------------------

  SwissLadderCategory::SwissLadderCategory(const TournamentDB& _db, const TabRow _row)
  : Category(_db, _row)
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

    // create the input data for the SwissLadderGenerator:
    // a list of ranked player pairs and a list of all
    // matches played so far
    PlayerPairList rankedPairs;
    std::vector<int> rankedPairs_Int;
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

    MatchMngr mm{db};
    std::vector<std::tuple<int,int>> pastMatches;
    for (MatchGroup mg : mm.getMatchGroupsForCat(*this))
    {
      for (Match ma : mg.getMatches())
      {
        // ignore unfinished matches. However, there
        // shouldn't be any when this function is called
        if (ma.getState() != ObjState::MA_FINISHED)
        {
          continue;
        }

        // store the match as CSV-strings of the
        // playerpair combinations
        int pp1Id = ma.getPlayerPair1().getPairId();
        int pp2Id = ma.getPlayerPair2().getPairId();
        pastMatches.push_back(make_tuple(pp1Id, pp2Id));
      }
    }

    // instantiate the SwissLadderGenerator and let it
    // generate the next set of matches
    SwissLadderGenerator slg{rankedPairs_Int, pastMatches};
    std::vector<std::tuple<int, int>> nextMatches;
    int errCode = slg.getNextMatches(nextMatches);

    // if we encountered a deadlock, remove all prepared future
    // matches and match groups and then we're done
    if (errCode == SwissLadderGenerator::DEADLOCK)
    {
      handleDeadlock();   // no result checking here for now
      return true;
    }

    if (errCode != SwissLadderGenerator::SOLUTION_FOUND)
    {
      return false;
    }

    // if we ever reach this point, newMatches contains a list of
    // tuples that define the matches for the next round
    //
    // let's fill the already prepared, "empty" matches with the
    // match information from newMatches
    ERR e;
    auto mg = mm.getMatchGroup(*this, lastRound+1, GROUP_NUM__ITERATION, &e);
    assert(mg.has_value());
    assert(e == ERR::OK);
    assert(mg->getMatches().size() == nextMatches.size());
    int cnt = 0;
    PlayerMngr pm{db};
    for (Match ma : mg->getMatches())
    {
      std::tuple<int, int> matchDef = nextMatches.at(cnt);
      int pp1Id = get<0>(matchDef);
      int pp2Id = get<1>(matchDef);
      PlayerPair pp1 = pm.getPlayerPair(pp1Id);
      PlayerPair pp2 = pm.getPlayerPair(pp2Id);

      e = mm.setPlayerPairsForMatch(ma, pp1, pp2);
      assert(e == ERR::OK);
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
      if (err != ERR::OK) return err;   // shouldn't happen
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
      if (mg.getState() != ObjState::MG_FINISHED)
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

    return ERR::OK;
  }


//----------------------------------------------------------------------------

  ERR SwissLadderCategory::canFreezeConfig()
  {
    if (getState() != ObjState::CAT_CONFIG)
    {
      return ERR::CONFIG_ALREADY_FROZEN;
    }
    
    // make sure there no unpaired players in singles or doubles
    if ((getMatchType() != SINGLES) && (hasUnpairedPlayers()))
    {
      return ERR::UNPAIRED_PLAYERS;
    }
    
    // make sure we have at least three players
    PlayerPairList pp = getPlayerPairs();
    if (pp.size() < 3)
    {
      return ERR::INVALID_PLAYER_COUNT;
    }
    
    return ERR::OK;
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

  ERR SwissLadderCategory::prepareFirstRound()
  {
    if (getState() != ObjState::CAT_IDLE) return ERR::WRONG_STATE;

    MatchMngr mm{db};

    // make sure we have not been called before; to this end, just
    // check that there have no matches been created for us so far
    auto allGrp = mm.getMatchGroupsForCat(*this);
    // do not return an error here, because obviously we have been
    // called successfully before and we only want to avoid
    // double initialization
    if (allGrp.size() != 0) return ERR::OK;

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
      assert(mg.has_value());
      assert(e == ERR::OK);

      for (int m=0; m < nMatchesPerRound; ++m)
      {
        auto ma = mm.createMatch(*mg, &e);
        assert(ma.has_value());
        assert(e == ERR::OK);
      }

      mm.closeMatchGroup(*mg);
    }

    // Fill the first round of matches based on the initial seeding
    genMatchesForNextRound();

    return ERR::OK;
  }

//----------------------------------------------------------------------------

  int SwissLadderCategory::calcTotalRoundsCount() const
  {
    ObjState stat = getState();
    if ((stat == ObjState::CAT_CONFIG) || (stat == ObjState::CAT_FROZEN))
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
      std::tuple<int, int, int, int> matchStatsA = a.getMatchStats();
      std::tuple<int, int, int, int> matchStatsB = b.getMatchStats();
      int deltaA = get<0>(matchStatsA) - get<2>(matchStatsA);
      int deltaB = get<0>(matchStatsB) - get<2>(matchStatsB);
      if (deltaA > deltaB) return true;
      if (deltaA < deltaB) return false;

      // second criteria: delta between won and lost games
      std::tuple<int, int, int> gameStatsA = a.getGameStats();
      std::tuple<int, int, int> gameStatsB = b.getGameStats();
      deltaA = get<0>(gameStatsA) - get<1>(gameStatsA);
      deltaB = get<0>(gameStatsB) - get<1>(gameStatsB);
      if (deltaA > deltaB) return true;
      if (deltaA < deltaB) return false;

      // second criteria: delta between won and lost points
      std::tuple<int, int> pointStatsA = a.getPointStats();
      std::tuple<int, int> pointStatsB = b.getPointStats();
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
    if (err != ERR::OK) return err;  // shouldn't happen
    rm.sortRankingEntriesForLastRound(*this, &err);
    if (err != ERR::OK) return err;  // shouldn't happen

    if (round != calcTotalRoundsCount())
    {
      genMatchesForNextRound();
    }

    return ERR::OK;
  }

//----------------------------------------------------------------------------

  PlayerPairList SwissLadderCategory::getRemainingPlayersAfterRound(int round, ERR* err) const
  {
    // No knock-outs, never
    if (err != nullptr) *err = ERR::OK;
    return getPlayerPairs();
  }

  //----------------------------------------------------------------------------

  ModMatchResult SwissLadderCategory::canModifyMatchResult(const Match& ma) const
  {
    // the match has to be in FINISHED state
    if (ma.getState() != ObjState::MA_FINISHED) return ModMatchResult::NotPossible;

    // if this match does not belong to us, we're not responsible
    if (ma.getCategory().getMatchSystem() != SWISS_LADDER) return ModMatchResult::NotPossible;

    // in Swiss Ladder, we can modify the results of
    // matches in the currently running round BEFORE the round is finished
    CatRoundStatus crs = getRoundStatus();
    if (crs.getCurrentlyRunningRoundNumber() != ma.getMatchGroup().getRound())
    {
      return ModMatchResult::NotPossible;
    }

    // we can change everything
    return ModMatchResult::WinnerLoser;
  }

  //----------------------------------------------------------------------------

  ModMatchResult SwissLadderCategory::modifyMatchResult(const Match& ma, const MatchScore& newScore) const
  {
    ModMatchResult mmr = canModifyMatchResult(ma);

    if ((mmr != ModMatchResult::ScoreOnly) && (mmr != ModMatchResult::WinnerLoser))
    {
      return mmr;
    }

    // IF we can modify the score, we can also change winner/loser
    // information. Thus, we can set any score that's presented to us
    MatchMngr mm{db};
    ERR e = mm.updateMatchScore(ma, newScore, true);

    return (e == ERR::OK) ? ModMatchResult::ModDone : ModMatchResult::NotPossible;
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
