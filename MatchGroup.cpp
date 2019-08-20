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

#include "MatchGroup.h"
#include <SqliteOverlay/DbTab.h>
#include "CatMngr.h"
#include "MatchMngr.h"

namespace QTournament
{

  MatchGroup::MatchGroup(const TournamentDB& _db, int rowId)
    :TournamentDatabaseObject(_db, TabMatch_GROUP, rowId), matchTab{db, TabMatch, false}
  {
  }

//----------------------------------------------------------------------------

  MatchGroup::MatchGroup(const TournamentDB& _db, const SqliteOverlay::TabRow& _row)
  :TournamentDatabaseObject(_db, _row), matchTab{db, TabMatch, false}
  {
  }

//----------------------------------------------------------------------------

  Category MatchGroup::getCategory() const
  {
    int catId = row.getInt(MG_CatRef);
    CatMngr cm{db};
    return cm.getCategoryById(catId);
  }

//----------------------------------------------------------------------------

  int MatchGroup::getGroupNumber() const
  {
    return row.getInt(MG_GrpNum);
  }

//----------------------------------------------------------------------------

  int MatchGroup::getRound() const
  {
    return row.getInt(MG_Round);
  }  

//----------------------------------------------------------------------------

  MatchList MatchGroup::getMatches() const
  {
    MatchMngr mm{db};
    return mm.getMatchesForMatchGroup(*this);
  }

//----------------------------------------------------------------------------

  int MatchGroup::getMatchCount() const
  {
    return matchTab.getMatchCountForColumnValue(MA_GrpRef, getId());
  }

//----------------------------------------------------------------------------

  int MatchGroup::getStageSequenceNumber() const
  {
    auto result = row.getInt2(MG_StageSeqNum);
    return result.value_or(-1);
  }

//----------------------------------------------------------------------------

  bool MatchGroup::hasMatchesInState(ObjState stat) const
  {
    // for performance reasons, we issue a single SQL-statement here
    // instead of looping through all matches in the group
    SqliteOverlay::WhereClause wc;
    wc.addCol(MA_GrpRef, getId());
    wc.addCol(GenericStateFieldName, static_cast<int>(stat));

    return (matchTab.getMatchCountForWhereClause(wc) > 0);
  }

//----------------------------------------------------------------------------

  bool MatchGroup::hasMatches__NOT__InState(ObjState stat) const
  {
    // for performance reasons, we issue a single SQL-statement here
    // instead of looping through all matches in the group
    SqliteOverlay::WhereClause wc;
    wc.addCol(MA_GrpRef, getId());
    wc.addCol(GenericStateFieldName, "!=", static_cast<int>(stat));

    return (matchTab.getMatchCountForWhereClause(wc) > 0);
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

}
