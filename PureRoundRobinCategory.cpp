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

#include "PureRoundRobinCategory.h"
#include "KO_Config.h"
#include "CatRoundStatus.h"
#include "RankingEntry.h"
#include "RankingMngr.h"
#include "assert.h"
#include "BracketGenerator.h"
#include "MatchMngr.h"

#include <QDebug>

using namespace SqliteOverlay;

namespace QTournament
{

  PureRoundRobinCategory::PureRoundRobinCategory(const TournamentDB& _db, int rowId)
  : Category(_db, rowId)
  {
  }

//----------------------------------------------------------------------------

  PureRoundRobinCategory::PureRoundRobinCategory(const TournamentDB& _db, const TabRow& _row)
  : Category(_db, _row)
  {
  }

  //----------------------------------------------------------------------------

  int PureRoundRobinCategory::getRoundCountPerIteration() const
  {
    ObjState stat = getState();
    if ((stat == ObjState::CAT_Config) || (stat == ObjState::CAT_Frozen))
    {
      return -1;   // category not yet fully configured; can't calc rounds
    }

    PlayerPairList allPairs = getPlayerPairs();
    int nPairs = allPairs.size();

    return ((nPairs % 2) == 0) ? (nPairs-1) : nPairs;
  }

  //----------------------------------------------------------------------------

  int PureRoundRobinCategory::getIterationCount() const
  {
    return getParameter_int(CatParameter::RoundRobinIterations);  // simple wrapper function
  }

  //----------------------------------------------------------------------------

  std::optional<PureRoundRobinCategory> PureRoundRobinCategory::getFromGenericCat(const Category& cat)
  {
    MatchSystem msys = cat.getMatchSystem();

    return (msys == MatchSystem::RoundRobin) ? PureRoundRobinCategory{cat.db, cat.row} : std::optional<PureRoundRobinCategory>{};
  }

  //----------------------------------------------------------------------------

  ModMatchResult PureRoundRobinCategory::canModifyMatchResult(const Match& ma) const
  {
    // the match has to be in FINISHED state
    if (ma.is_NOT_InState(ObjState::MA_Finished)) return ModMatchResult::NotPossible;

    // if this match does not belong to us, we're not responsible
    if (ma.getCategory().getMatchSystem() != MatchSystem::RoundRobin) return ModMatchResult::NotPossible;

    // in round robins, we can modify the results of all matches
    // at any time
    return ModMatchResult::WinnerLoser;
  }

  //----------------------------------------------------------------------------

  ModMatchResult PureRoundRobinCategory::modifyMatchResult(const Match& ma, const MatchScore& newScore) const
  {
    ModMatchResult mmr = canModifyMatchResult(ma);
    if (mmr != ModMatchResult::WinnerLoser) return mmr;

    //
    // if we can modify the score, we can also change winner/loser
    // information. Thus, we can set any score that's presented to us
    // without further checks
    //

    // store the old match result
    MatchScore oldScore = *(ma.getScore());  // is guaranteed to be != nullptr

    // update the match
    MatchMngr mm{db};
    Error e = mm.updateMatchScore(ma, newScore, true);
    if (e != Error::OK) return ModMatchResult::NotPossible;

    // update all affected ranking entries
    RankingMngr rm{db};
    e = rm.updateRankingsAfterMatchResultChange(ma, oldScore);
    return (e == Error::OK) ? ModMatchResult::ModDone : ModMatchResult::NotPossible;
  }

//----------------------------------------------------------------------------

  Error PureRoundRobinCategory::canFreezeConfig()
  {
    if (is_NOT_InState(ObjState::CAT_Config))
    {
      return Error::ConfigAlreadyFrozen;
    }
    
    // make sure there no unpaired players in singles or doubles
    if ((getMatchType() != MatchType::Singles) && (hasUnpairedPlayers()))
    {
      return Error::UnpairedPlayers;
    }
    
    // make sure we have at least three players
    PlayerPairList pp = getPlayerPairs();
    if (pp.size() < 3)
    {
      return Error::InvalidPlayerCount;
    }
    
    return Error::OK;
  }

//----------------------------------------------------------------------------

  bool PureRoundRobinCategory::needsInitialRanking()
  {
    return false;
  }

//----------------------------------------------------------------------------

  bool PureRoundRobinCategory::needsGroupInitialization()
  {
    return false;
  }

//----------------------------------------------------------------------------

  Error PureRoundRobinCategory::prepareFirstRound()
  {
    if (is_NOT_InState(ObjState::CAT_Idle)) return Error::WrongState;

    MatchMngr mm{db};

    // make sure we have not been called before; to this end, just
    // check that there have no matches been created for us so far
    auto allGrp = mm.getMatchGroupsForCat(*this);
    // do not return an error here, because obviously we have been
    // called successfully before and we only want to avoid
    // double initialization
    if (allGrp.size() != 0) return Error::OK;

    // alright, this is a virgin category. Generate
    // all round robin matches at once
    PlayerPairList allPairs = getPlayerPairs();
    int iterationCount = getIterationCount();
    int roundsPerIteration = getRoundCountPerIteration();
    Error e;
    for (int i=0; i < iterationCount; ++i)
    {
      int firstRoundNum = (i * roundsPerIteration) + 1;
      e = generateGroupMatches(allPairs, GroupNum_Iteration, firstRoundNum);
      if (e != Error::OK) return e;
    }
    return Error::OK;
  }

//----------------------------------------------------------------------------

  int PureRoundRobinCategory::calcTotalRoundsCount() const
  {
    int roundsPerIteration = getRoundCountPerIteration();
    if (roundsPerIteration < 1) return -1;  // category not yet configured

    // done
    return getIterationCount() * roundsPerIteration;
  }

//----------------------------------------------------------------------------

  // this return a function that should return true if "a" goes before "b" when sorting. Read:
  // return a function that return true true if the score of "a" is better than "b"
  std::function<bool (RankingEntry& a, RankingEntry& b)> PureRoundRobinCategory::getLessThanFunction()
  {
    return [](RankingEntry& a, RankingEntry& b) {
      // first criterion: delta between won and lost matches
      std::tuple<int, int, int, int> matchStatsA = a.getMatchStats();
      std::tuple<int, int, int, int> matchStatsB = b.getMatchStats();
      int deltaA = std::get<0>(matchStatsA) - std::get<2>(matchStatsA);
      int deltaB = std::get<0>(matchStatsB) - std::get<2>(matchStatsB);
      if (deltaA > deltaB) return true;
      if (deltaA < deltaB) return false;

      // second criteria: delta between won and lost games
      std::tuple<int, int, int> gameStatsA = a.getGameStats();
      std::tuple<int, int, int> gameStatsB = b.getGameStats();
      deltaA = std::get<0>(gameStatsA) - std::get<1>(gameStatsA);
      deltaB = std::get<0>(gameStatsB) - std::get<1>(gameStatsB);
      if (deltaA > deltaB) return true;
      if (deltaA < deltaB) return false;

      // second criteria: delta between won and lost points
      std::tuple<int, int> pointStatsA = a.getPointStats();
      std::tuple<int, int> pointStatsB = b.getPointStats();
      deltaA = std::get<0>(pointStatsA) - std::get<1>(pointStatsA);
      deltaB = std::get<0>(pointStatsB) - std::get<1>(pointStatsB);
      if (deltaA > deltaB) return true;
      if (deltaA < deltaB) return false;

      // TODO: add a direct comparison as additional criteria?

      // Default: "a" is not strictly better than "b", so we return false
      return false;
    };
  }

//----------------------------------------------------------------------------

  Error PureRoundRobinCategory::onRoundCompleted(int round)
  {
    RankingMngr rm{db};
    Error err;

    rm.createUnsortedRankingEntriesForLastRound(*this, &err);
    if (err != Error::OK) return err;  // shouldn't happen
    rm.sortRankingEntriesForLastRound(*this, &err);
    if (err != Error::OK) return err;  // shouldn't happen

    return Error::OK;
  }

//----------------------------------------------------------------------------

  PlayerPairList PureRoundRobinCategory::getRemainingPlayersAfterRound(int round, Error* err) const
  {
    // No knock-outs, never
    if (err != nullptr) *err = Error::OK;
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
