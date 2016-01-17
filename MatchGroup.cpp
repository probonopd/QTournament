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

#include "MatchGroup.h"
#include "Tournament.h"
#include "DbTab.h"

namespace QTournament
{

  MatchGroup::MatchGroup(TournamentDB* db, int rowId)
  :TournamentDatabaseObject(db, TAB_MATCH_GROUP, rowId), matchTab(db->getTab(TAB_MATCH))
  {
  }

//----------------------------------------------------------------------------

  MatchGroup::MatchGroup(TournamentDB* db, SqliteOverlay::TabRow row)
  :TournamentDatabaseObject(db, row), matchTab(db->getTab(TAB_MATCH))
  {
  }

//----------------------------------------------------------------------------

  Category MatchGroup::getCategory() const
  {
    int catId = row.getInt(MG_CAT_REF);
    auto tnmt = Tournament::getActiveTournament();
    return tnmt->getCatMngr()->getCategoryById(catId);
  }

//----------------------------------------------------------------------------

  int MatchGroup::getGroupNumber() const
  {
    return row.getInt(MG_GRP_NUM);
  }

//----------------------------------------------------------------------------

  int MatchGroup::getRound() const
  {
    return row.getInt(MG_ROUND);
  }  

//----------------------------------------------------------------------------

  MatchList MatchGroup::getMatches() const
  {
    auto tnmt = Tournament::getActiveTournament();
    return tnmt->getMatchMngr()->getMatchesForMatchGroup(*this);
  }

//----------------------------------------------------------------------------

  int MatchGroup::getMatchCount() const
  {
    return matchTab->getMatchCountForColumnValue(MA_GRP_REF, getId());
  }

//----------------------------------------------------------------------------

  int MatchGroup::getStageSequenceNumber() const
  {
    auto result = row.getInt2(MG_STAGE_SEQ_NUM);
    if (result->isNull())
    {
      return -1;  // group not staged
    }

    return result->get();
  }

//----------------------------------------------------------------------------

  bool MatchGroup::hasMatchesInState(OBJ_STATE stat) const
  {
    // for performance reasons, we issue a single SQL-statement here
    // instead of looping through all matches in the group
    WhereClause wc;
    wc.addIntCol(MA_GRP_REF, getId());
    wc.addIntCol(GENERIC_STATE_FIELD_NAME, static_cast<int>(stat));
    return (matchTab->getMatchCountForWhereClause(wc) > 0);
  }

//----------------------------------------------------------------------------

  bool MatchGroup::hasMatches__NOT__InState(OBJ_STATE stat) const
  {
    // for performance reasons, we issue a single SQL-statement here
    // instead of looping through all matches in the group
    QString where = "%1 = %2 AND %3 != %4";
    where = where.arg(MA_GRP_REF).arg(getId());
    where = where.arg(GENERIC_NAME_FIELD_NAME).arg(static_cast<int>(stat));

    return (matchTab->getMatchCountForWhereClause(where.toUtf8().constData()) > 0);
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
