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

#include <SqliteOverlay/ClausesAndQueries.h>
#include <SqliteOverlay/GenericObjectManager.h>

#include "Match.h"
#include "Category.h"
#include "CatRoundStatus.h"
#include "TournamentDataDefs.h"
#include "TournamentDB.h"
#include "TournamentErrorCodes.h"
#include "PlayerPair.h"
#include "MatchMngr.h"

#include "BackendAPI.h"

using namespace SqliteOverlay;

namespace QTournament::API::Internal
{

  Error generateBracketMatches(const Category& cat, SvgBracketMatchSys brMatchSys, const QTournament::PlayerPairList& seeding, int firstRoundNum)
  {
    const auto& db = cat.getDatabaseHandle();

    // check at least that the first round number is valid
    CatRoundStatus crs = cat.getRoundStatus();
    if (firstRoundNum <= crs.getHighestGeneratedMatchRound()) return Error::InvalidRound;

    // retrieve the bracket definition for the selected system
    // and the required number of players
    auto brDef = SvgBracket::findSvgBracket(brMatchSys, seeding.size());
    if (!brDef)
    {
      return Error::InvalidPlayerCount;  // shouldn't happen, should have been checked before
    }

    // generate the bracket data for the player list
    //
    // matches come already sorted by match number
    auto allMatches = SvgBracket::convertToBracketMatches(*brDef);

    // assign the intial seeding to the bracket.
    // this automatically tags all unused matches as either "fast forward" (only
    // one player) or "empty" (no players at all)
    allMatches.applySeeding(seeding);

    // create match groups and matches "from left to right"
    auto trans = db.startTransaction();

    MatchMngr mm{db};
    Round tnmtRound{firstRoundNum - 1};  // counter for the "real", "external" tournament round number; start with "-1" to compensate for the first increment in the loop
    Round bracketRound{1};   // counter for the bracket-internal rounds
    std::optional<MatchGroup> curGroup{};
    std::unordered_map<int, int> bracket2regularMatchNum;
    for (const auto& bmd : allMatches)
    {
      // do we need to close the current match group?
      if (bmd.round() > bracketRound)
      {
        if (curGroup)
        {
          mm.closeMatchGroup(*curGroup);
          curGroup.reset();
        }
        bracketRound = bmd.round();
      }

      // only process "real" matches
      if (bmd.canSkip()) continue;

      // do we need to create a new match group?
      if (!bmd.canSkip() && !curGroup)
      {
        tnmtRound = Round{tnmtRound.get() + 1};

        int grpType = brDef->roundTypes.at(bracketRound.get() - 1);
        auto newGroup = mm.createMatchGroup(cat, tnmtRound.get(), grpType);
        if (!newGroup)
        {
          return newGroup.err();
        }
        curGroup = newGroup;
      }

      // create a new, empty match in this group and map it to the bracket match id
      auto ma = mm.createMatch(*curGroup);
      if (!ma)
      {
        return ma.err();
      }
      mm.setBrackMatchLink(*ma, bmd.matchNum());
      bracket2regularMatchNum[bmd.matchNum().get()] = ma->getId();
      std::cout << "M " << bmd.matchNum().get() << ": created as real match (" << ma->getId() << ")" << std::endl;
    }

    // close the last group
    if (curGroup) mm.closeMatchGroup(*curGroup);

    // copy the data from the BracketMatchDataList to the "real" matches
    for (const auto& bmd : allMatches)
    {
      if (bmd.canSkip()) continue;

      const auto ma = mm.getMatch(bracket2regularMatchNum.at(bmd.matchNum().get())).value();

      // assign symbolic player pairs and ranks first
      //
      // this is important because setSymbolicPlayerForMatch() overrides any assigned
      // real player pair with NULL

      // follow the winner path
      Error err;
      const auto winnerAction = allMatches.traverseForward(bmd, SvgBracket::PairRole::AsWinner);
      if (std::holds_alternative<Rank>(winnerAction))
      {
        err = mm.setRankForWinnerOrLoser(ma, true, std::get<Rank>(winnerAction).get());

        std::cout << "R " << ma.getId() << ": set WR = " << std::get<Rank>(winnerAction).get() << std::endl;
      } else {
        auto& outLink = std::get<SvgBracket::OutgoingBracketLink>(winnerAction);
        int realNextMatchId = bracket2regularMatchNum.at(outLink.dstMatch.get());
        Match nextMatch = mm.getMatch(realNextMatchId).value();
        err = mm.setSymbolicPlayerForMatch(ma, nextMatch, true, outLink.pos);
        std::cout << "Linking R " << ma.getId() << ".W --> R " << nextMatch.getId() << "." << outLink.pos << std::endl;
      }
      if (err != Error::OK) return err;

      // follow the loser path
      const auto loserAction = allMatches.traverseForward(bmd, SvgBracket::PairRole::AsLoser);
      if (std::holds_alternative<Rank>(loserAction))
      {
        err = mm.setRankForWinnerOrLoser(ma, false, std::get<Rank>(loserAction).get());
        std::cout << "R " << ma.getId() << ": set LR = " << std::get<Rank>(loserAction).get() << std::endl;
      } else {
        auto& outLink = std::get<SvgBracket::OutgoingBracketLink>(loserAction);
        int realNextMatchId = bracket2regularMatchNum.at(outLink.dstMatch.get());
        Match nextMatch = mm.getMatch(realNextMatchId).value();
        err = mm.setSymbolicPlayerForMatch(ma, nextMatch, false, outLink.pos);
        std::cout << "Linking R " << ma.getId() << ".L --> R " << nextMatch.getId() << "." << outLink.pos << std::endl;
      }
      if (err != Error::OK) return err;

      // assign real player pair IDs
      if (bmd.assignedPair1().get() != -1)
      {
        PlayerPair pp{db, bmd.assignedPair1().get()};
        auto err = mm.setPlayerPairForMatch(ma, pp, 1);
        std::cout << "R " << ma.getId() << ".1: assigned pair" << std::endl;
        if (err != Error::OK) return err;
      } else {
        // special case, rare: only one player is used and the match does not need to be played,
        // BUT the match contains information about the final rank of the one player
        //
        // FIX ME: still required? Or covered by "traverseForward()"?
        /*if (wr)
        {
          mm.setPlayerToUnused(*ma, 1, wr->get());
        }*/
      }

      if (bmd.assignedPair2().get() != -1)
      {
        PlayerPair pp{db, bmd.assignedPair2().get()};
        auto err = mm.setPlayerPairForMatch(ma, pp, 2);
        std::cout << "R " << ma.getId() << ".2: assigned pair" << std::endl;
        if (err != Error::OK) return err;
      } else {
        // special case, rare: only one player is used and the match does not need to be played,
        // BUT the match contains information about the final rank of the one player
        //
        // FIX ME: still required? Or covered by "traverseForward()"?
        /*if (wr)
        {
          mm.setPlayerToUnused(*ma, 2, wr->get());
        }*/
      }
    }

    // finally store all changes
    trans.commit();

    return Error::OK;
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

}
