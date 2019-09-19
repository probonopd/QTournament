/*
 *    This is QTournament, a badminton tournament management program.
 *    Copyright (C) 2014 - 2019  Volker Knollmann
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

#include <SqliteOverlay/ClausesAndQueries.h>
#include <SqliteOverlay/GenericObjectManager.h>

#include "BackendAPI.h"

#include "Match.h"
#include "TournamentDataDefs.h"
#include "TournamentDB.h"
#include "TournamentErrorCodes.h"
#include "CatRoundStatus.h"
#include "CatMngr.h"
#include "MatchMngr.h"

using namespace SqliteOverlay;

namespace QTournament::API::Qry
{
  std::optional<Match> nextCallableMatch(const TournamentDB& db)
  {
    // find the next available match with the lowest match number
    int reqState = static_cast<int>(ObjState::MA_Ready);
    WhereClause wc;
    wc.addCol(GenericStateFieldName, reqState);
    wc.setOrderColumn_Asc(MA_Num);
    auto matchRow = DbTab{db, TabMatch, false}.get2(wc);
    if (!matchRow)
    {
      return std::optional<Match>{};
    }

    return Match{db, *matchRow};
  }

//----------------------------------------------------------------------------
  bool isBracketRound(const TournamentDB& db, const Category& cat, Round r)
  {
    // check the match system
    const auto msys = cat.getMatchSystem();
    if ((msys != MatchSystem::Bracket) && (msys != MatchSystem::GroupsWithKO))
    {
      return false;
    }

    //
    // check the round validity
    //

    if (r < 1) return false;
    auto special = cat.convertToSpecializedObject();
    int rMax = special->calcTotalRoundsCount();
    if (r > rMax) return false;

    if (msys == MatchSystem::GroupsWithKO)
    {
      KO_Config cfg{cat.getParameter_string(CatParameter::GroupConfig)};
      return (r > cfg.getNumRounds());
    }

    if (msys == MatchSystem::Bracket)
    {
      return true;
    }

    return false;
  }
//----------------------------------------------------------------------------

  std::vector<SimplifiedRanking> getBracketRanks(const Category& cat, const Round& firstRound, const Round& lastRound)
  {

    if (firstRound < 0) return {};
    const auto catState = cat.getState();
    if ((catState == ObjState::CAT_Config) || (catState == ObjState::CAT_Frozen)) return{};

    const auto msys = cat.getMatchSystem();
    if ((msys != MatchSystem::GroupsWithKO) && (msys != MatchSystem::Bracket)) return {};

    // determine the first round of bracket matches and
    // and check the validity of the firstRound argument
    Round firstBracketRound{1};
    if (msys == MatchSystem::GroupsWithKO)
    {
      KO_Config cfg{cat.getParameter_string(CatParameter::GroupConfig)};
      firstBracketRound = Round{cfg.getNumRounds() + 1};
    }
    if (firstRound < (firstBracketRound.get() - 1)) return {};   // "first - 1" to account for "ranks after seeding only"
    CatRoundStatus crs{cat.getDatabaseHandle(), cat};
    const Round lastFinishedRound{crs.getFinishedRoundsCount()};
    if (firstRound > lastFinishedRound) return {};

    // determine the actual last round for which we'll retrieve ranks
    Round actualLastRound{ (lastRound >= firstRound) ? lastRound : firstRound };
    if (actualLastRound > lastFinishedRound) actualLastRound = lastFinishedRound;

    // initialize the tree
    auto allMatches = getSeededBracketMatches(cat);

    std::vector<SimplifiedRanking> result;
    SimplifiedRanking curRoundRanking{Round{-1}};  // dummy initial round number

    // are we supposed to return the ranks after
    // seeding?
    if (firstRound == (firstBracketRound.get() - 1))
    {
      curRoundRanking.round = Round{firstRound.get() - 1};
      curRoundRanking.ranks = extractSortedRanksFromBracket(allMatches);
      result.push_back(curRoundRanking);
    }

    // apply the existing matches round by round
    // in ascending round order
    MatchMngr mm{cat.getDatabaseHandle()};
    for (Round curRound{firstBracketRound}; curRound <= actualLastRound; curRound = Round{curRound.get() + 1})
    {
      // update the row number in the ever-growing ranks list
      curRoundRanking.round = curRound;

      // apply the matches of this round to the bracket
      // and already store winner / loser information that
      // is contained in the match
      const auto mgl = mm.getMatchGroupsForCat(cat, curRound.get());
      if (!mgl.empty())
      {
        std::vector<Match> maList;
        for (const auto& ma : mgl[0].getMatches())
        {
          maList.push_back(ma);
        }

        // append the ranks from the match list
        const auto thisRoundsRanks = extractSortedRanksFromMatchList(maList);
        std::copy(begin(thisRoundsRanks), end(thisRoundsRanks), back_inserter(curRoundRanking.ranks));

        // apply the matches to the bracket
        allMatches.applyMatches(maList);
      }

      // find ranks resulting from assigned / dead branch combinations
      const auto thisRoundsRanks = extractSortedRanksFromBracket(allMatches);
      std::copy(begin(thisRoundsRanks), end(thisRoundsRanks), back_inserter(curRoundRanking.ranks));

      // are we supposed to store the ranking for
      // this round?
      if (curRound >= firstRound)
      {
        auto& rankList = curRoundRanking.ranks;  // just as an abbreviation

        Internal::sortSimplifiedRanking(rankList);

        // ranks resulting from the bracket (not from the finished matches)
        // might now occur more than once in our list ==> remove the duplicates
        auto newEnd = std::unique(begin(rankList), end(rankList),
                                  [](const SimplifiedRankingEntry& re1, const SimplifiedRankingEntry& re2)
        {
          return (re1.rank == re2.rank);
        });
        rankList.erase(newEnd, end(rankList));

        result.push_back(curRoundRanking);
      }


      // DO NOT RESET curRoundRanking because in the next iterations, we just
      // keep on adding new ranks
    }

    return result;
  }


//----------------------------------------------------------------------------

  std::vector<SimplifiedRankingEntry> extractSortedRanksFromMatchList(const MatchList& maList)
  {
    std::vector<SimplifiedRankingEntry> result;

    for (const auto& ma : maList)
    {
      auto r = ma.getWinnerRank();
      auto pp = ma.getWinner();
      if (pp && (r > 0))
      {
        result.push_back(SimplifiedRankingEntry{PlayerPairRefId{pp->getPairId()}, Rank{r}});
      }
      r = ma.getLoserRank();
      pp = ma.getLoser();
      if (pp && (r > 0))
      {
        result.push_back(SimplifiedRankingEntry{PlayerPairRefId{pp->getPairId()}, Rank{r}});
      }
    }

    API::Internal::sortSimplifiedRanking(result);

    return result;
  }


//----------------------------------------------------------------------------

  std::vector<SimplifiedRankingEntry> extractSortedRanksFromBracket(const SvgBracket::BracketMatchDataList& bmdl)
  {
    using BrState = SvgBracket::BracketMatchData::BranchState;

    std::vector<SimplifiedRankingEntry> result;
    for (const auto& ma : bmdl)
    {
      const auto r = ma.winnerRank();

      // skip matches without winner rank
      if (!r) continue;

      const auto br1State = ma.pair1State();
      const auto br2State = ma.pair2State();
      if ((br1State == BrState::Assigned) && (br2State == BrState::Dead))
      {
        const auto pp = ma.assignedPair1();
        result.push_back(SimplifiedRankingEntry{pp, *r});
      }
      if ((br1State == BrState::Dead) && (br2State == BrState::Assigned))
      {
        const auto pp = ma.assignedPair2();
        result.push_back(SimplifiedRankingEntry{pp, *r});
      }
    }

    API::Internal::sortSimplifiedRanking(result);

    return result;
  }


//----------------------------------------------------------------------------

  SvgBracket::BracketMatchDataList getSeededBracketMatches(const Category& cat)
  {
    CatMngr cm{cat.getDatabaseHandle()};
    SvgBracketMatchSys brSys = static_cast<SvgBracketMatchSys>(cat.getParameter_int(CatParameter::BracketMatchSystem));
    const auto seeding = cm.getSeeding(cat);
    if (seeding.empty())
    {
      throw std::runtime_error("getSeededBracketMatches(): no suitable seeding"); // this should never happen if the caller met the preconditions
    }
    auto brDef = SvgBracket::findSvgBracket(brSys, seeding.size());
    if (!brDef)
    {
      throw std::runtime_error("getSeededBracketMatches(): no suitable bracket"); // this should never happen if the caller met the preconditions
    }

    auto allMatches = SvgBracket::convertToBracketMatches(*brDef);

    allMatches.applySeeding(seeding);

    return allMatches;
  }

//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------

}
