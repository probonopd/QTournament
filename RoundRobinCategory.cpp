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

#include <QDebug>

#include "RoundRobinCategory.h"
#include "KO_Config.h"
#include "CatRoundStatus.h"
#include "RankingEntry.h"
#include "RankingMngr.h"
#include "assert.h"
#include "HelperFunc.h"
#include "MatchMngr.h"
#include "CatMngr.h"
#include "BackendAPI.h"

using namespace SqliteOverlay;

namespace QTournament
{

  RoundRobinCategory::RoundRobinCategory(const TournamentDB& _db, int rowId)
  : Category(_db, rowId)
  {
  }

//----------------------------------------------------------------------------

  RoundRobinCategory::RoundRobinCategory(const TournamentDB& _db, const TabRow& _row)
  : Category(_db, _row)
  {
  }

//----------------------------------------------------------------------------

  Error RoundRobinCategory::canFreezeConfig()
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

    // make sure we have a valid group configuration
    KO_Config cfg = KO_Config(getParameter_string(CatParameter::GroupConfig));
    if (!(cfg.isValid(pp.size())))
    {
      return Error::InvalidKoConfig;
    }
    
    return Error::OK;
  }

//----------------------------------------------------------------------------

  bool RoundRobinCategory::needsInitialRanking()
  {
    return false;
  }

//----------------------------------------------------------------------------

  bool RoundRobinCategory::needsGroupInitialization()
  {
    return true;
  }

//----------------------------------------------------------------------------

  Error RoundRobinCategory::prepareFirstRound()
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

    // alright, this is a virgin category. Generate group matches
    // for each group
    KO_Config cfg = KO_Config(getParameter_string(CatParameter::GroupConfig));
    for (int grpIndex = 0; grpIndex < cfg.getNumGroups(); ++grpIndex)
    {
      PlayerPairList grpMembers = getPlayerPairs(grpIndex+1);
      Error e = generateGroupMatches(grpMembers, grpIndex+1, 1);
      if (e != Error::OK) return e;
    }

    return Error::OK;
  }

//----------------------------------------------------------------------------

  int RoundRobinCategory::calcTotalRoundsCount() const
  {
    ObjState stat = getState();
    if ((stat == ObjState::CAT_Config) || (stat == ObjState::CAT_Frozen))
    {
      return -1;   // category not yet fully configured; can't calc rounds
    }

    // the following call must succeed, since we made it past the
    // configuration point
    KO_Config cfg = KO_Config(getParameter_string(CatParameter::GroupConfig));

    // the number of rounds is
    // (number of group rounds) + (number of KO rounds)
    int groupRounds = cfg.getNumRounds();

    KO_Start startLevel = cfg.getStartLevel();
    int eliminationRounds = 1;  // finals
    if (startLevel != KO_Start::Final) ++eliminationRounds; // semi-finals for all, except we dive straight into finals
    if ((startLevel == KO_Start::Quarter) || (startLevel == KO_Start::L16)) ++eliminationRounds;  // QF and last 16
    if (startLevel == KO_Start::L16) ++eliminationRounds;  // round of last 16

    return groupRounds + eliminationRounds;
  }

//----------------------------------------------------------------------------

  // this return a function that should return true if "a" goes before "b" when sorting. Read:
  // return a function that return true true if the score of "a" is better than "b"
  std::function<bool (RankingEntry& a, RankingEntry& b)> RoundRobinCategory::getLessThanFunction()
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

  Error RoundRobinCategory::onRoundCompleted(int round)
  {
    // determine the number of group rounds.
    //
    // The following call must succeed, since we made it past the
    // configuration point
    KO_Config cfg = KO_Config(getParameter_string(CatParameter::GroupConfig));
    int groupRounds = cfg.getNumRounds();

    RankingMngr rm{db};
    Error err;

    // if we are still in group rounds, simply calculate the
    // new ranking
    if (round <= groupRounds)
    {
      // we always want to create ranking entries for all player pairs,
      // even if some players or complete groups haven't played in this
      // round at all (happens with different group sizes in one category)
      rm.createUnsortedRankingEntriesForLastRound(*this, &err, getPlayerPairs());
      if (err != Error::OK) return err;  // shouldn't happen
      rm.sortRankingEntriesForLastRound(*this, &err);
      if (err != Error::OK) return err;  // shouldn't happen
    }

    // if this was the last round in group rounds,
    // we need to wait for user input (seeding)
    // before we can enter the KO rounds
    if (round == groupRounds)
    {
      CatMngr cm{db};
      cm.switchCatToWaitForSeeding(*this);
    }

    // Actions for KO rounds
    if (round > groupRounds)
    {
      // create ranking entries for everyone who played.
      // this is only to get the accumulated values for the finalists right
      PlayerPairList ppList;
      ppList = this->getRemainingPlayersAfterRound(round - 1, &err);
      if (err != Error::OK) return err;
      rm.createUnsortedRankingEntriesForLastRound(*this, &err, ppList);
      if (err != Error::OK) return err;

      // there's nothing to do for us except after the last round.
      // after the last roound, we have to create final ranking entries
      CatRoundStatus crs = getRoundStatus();
      int lastFinishedRound = crs.getFinishedRoundsCount();
      if (lastFinishedRound != calcTotalRoundsCount())
      {
        return Error::OK;
      }

      // set the ranks for the winner / losers of the finals
      MatchMngr mm{db};
      for (MatchGroup mg : mm.getMatchGroupsForCat(*this, lastFinishedRound))
      {
        for (Match ma : mg.getMatches())
        {
          auto winner = ma.getWinner();
          assert(winner.has_value());
          auto re = rm.getRankingEntry(*winner, lastFinishedRound);
          assert(re.has_value());
          int winnerRank = ma.getWinnerRank();
          assert(winnerRank > 0);
          rm.forceRank(*re, winnerRank);

          auto loser = ma.getLoser();
          assert(loser.has_value());
          re = rm.getRankingEntry(*loser, lastFinishedRound);
          assert(re.has_value());
          int loserRank = ma.getLoserRank();
          assert(loserRank > 0);
          rm.forceRank(*re, loserRank);
        }
      }
    }
    return Error::OK;
  }

//----------------------------------------------------------------------------

  PlayerPairList RoundRobinCategory::getRemainingPlayersAfterRound(int round, Error* err) const
  {
    // we can only determine remaining players after completed rounds
    CatRoundStatus crs = getRoundStatus();
    if (round > crs.getFinishedRoundsCount())
    {
      if (err != nullptr) *err = Error::InvalidRound;
      return PlayerPairList();
    }

    // the following call must succeed since we finished at least one round
    KO_Config cfg = KO_Config(getParameter_string(CatParameter::GroupConfig));
    int numGroupRounds = cfg.getNumRounds();

    // three cases for the list of remaining players:
    //   1) before the last round robing round (==> all players)
    //   2) after the last RR round and before the first KO round (==> qualified players)
    //   3) in KO rounds (==> survivors)

    if (round < numGroupRounds)
    {
      if (err != nullptr) *err = Error::OK;
      return getPlayerPairs();
    }

    if (round == numGroupRounds)
    {
      if (err != nullptr) *err = Error::OK;
      return getQualifiedPlayersAfterRoundRobin_sorted();
    }

    if (round > numGroupRounds)
    {
      //
      // TODO: this is redundant code. See EliminationCategory
      //

      // get the list for the previous round
      PlayerPairList result;
      Error e;
      result = this->getRemainingPlayersAfterRound(round-1, &e);
      if (e != Error::OK)
      {
        if (err != nullptr) *err = Error::InvalidRound;
        return PlayerPairList();
      }

      // get the match losers of this round
      // and remove them from the list of the previous round
      //
      // exception: losers in semi-finals will continue in the
      // match for 3rd place
      if (round == (calcTotalRoundsCount() - 1))  // semi-finals
      {
        if (err != nullptr) *err = Error::OK;
        return result;
      }
      MatchMngr mm{db};
      for (MatchGroup mg : mm.getMatchGroupsForCat(*this, round))
      {
        for (Match ma : mg.getMatches())
        {
          auto loser = ma.getLoser();
          if (!loser.has_value()) continue;   // shouldn't happen
          Sloppy::eraseAllOccurencesFromVector<PlayerPair>(result, *loser);
        }
      }

      if (err != nullptr) *err = Error::OK;
      return result;
    }

    // we should never reach this point
    if (err != nullptr) *err = Error::InvalidRound;
    return PlayerPairList();
  }

//----------------------------------------------------------------------------

  PlayerPairList RoundRobinCategory::getPlayerPairsForIntermediateSeeding() const
  {
    if (is_NOT_InState(ObjState::CAT_WaitForIntermediateSeeding))
    {
      return PlayerPairList();
    }

    return getQualifiedPlayersAfterRoundRobin_sorted();
  }

//----------------------------------------------------------------------------

  Error RoundRobinCategory::resolveIntermediateSeeding(const PlayerPairList& seed) const
  {
    if (is_NOT_InState(ObjState::CAT_WaitForIntermediateSeeding))
    {
      return Error::CategoryNeedsNoSeeding;
    }

    // make sure that the required player pairs and the
    // provided player pairs are identical
    PlayerPairList controlList = getPlayerPairsForIntermediateSeeding();
    for (PlayerPair pp : seed)
    {
      if (std::find(controlList.begin(), controlList.end(), pp) == controlList.end())
      {
        return Error::InvalidSeedingList;
      }
      Sloppy::eraseAllOccurencesFromVector<PlayerPair>(controlList, pp);
    }
    if (!(controlList.empty()))
    {
      return Error::InvalidSeedingList;
    }

    // okay, the list is valid. Now lets generate single-KO matches
    // for the second phase of the tournament
    KO_Config cfg = KO_Config(getParameter_string(CatParameter::GroupConfig));
    int numGroupRounds = cfg.getNumRounds();
    return API::Internal::generateBracketMatches(*this, SvgBracketMatchSys::SingleElim, seed, numGroupRounds+1);
  }

//----------------------------------------------------------------------------

  PlayerPairList RoundRobinCategory::getQualifiedPlayersAfterRoundRobin_sorted() const
  {
    // have we finished the round robin phase?
    CatRoundStatus crs = getRoundStatus();
    KO_Config cfg = KO_Config(getParameter_string(CatParameter::GroupConfig));
    int numGroupRounds = cfg.getNumRounds();
    if (crs.getFinishedRoundsCount() < numGroupRounds)
    {
      return PlayerPairList();  // nope, we're not there yet
    }

    // get the ranking after the last round-robin round
    RankingMngr rm{db};
    RankingEntryListList rll = rm.getSortedRanking(*this, numGroupRounds);
    assert(rll.size() == cfg.getNumGroups());

    PlayerPairList result;
    for (RankingEntryList rl : rll)
    {
      // this should be guaranteed by the minimum group size of three
      assert(rl.size() >= 2);

      // the first in each group is always qualified
      auto qualifiedPP = rl.at(0).getPlayerPair();
      assert(qualifiedPP.has_value());
      result.insert(result.begin(), *qualifiedPP);

      // maybe the second qualifies as well
      if (cfg.getSecondSurvives())
      {
        qualifiedPP = rl.at(1).getPlayerPair();
        assert(qualifiedPP.has_value());
        result.push_back(*qualifiedPP);
      }
    }

    // the list entries for the first-ranked players are in reverse order, e.g.:
    // the first of group 8 is at index 0, the first of group 7 is at index 1, ...
    //
    // let's fix that (for cosmetic reasons)
    //std::reverse(result.begin(), result.begin() + cfg.getNumGroups() + 1);  // +1 because the last element is not included in the reversing operation
    auto firstItem = std::begin(result);
    auto lastItem = firstItem;
    std::advance(lastItem, cfg.getNumGroups());
    std::reverse(firstItem, lastItem);

    return result;
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


}
