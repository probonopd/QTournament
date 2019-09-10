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

#include "BackendAPI.h"

#include "Match.h"
#include "TournamentDataDefs.h"
#include "TournamentDB.h"
#include "TournamentErrorCodes.h"
#include "CatRoundStatus.h"


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
