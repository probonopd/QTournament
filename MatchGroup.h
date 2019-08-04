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

#ifndef MATCHGROUP_H
#define	MATCHGROUP_H

#include <vector>

#include <SqliteOverlay/TabRow.h>
#include <SqliteOverlay/DbTab.h>
#include <SqliteOverlay/GenericObjectManager.h>

#include "TournamentDatabaseObject.h"
#include "TournamentDB.h"
#include "TournamentErrorCodes.h"
#include "Category.h"
#include "Match.h"

namespace QTournament
{

  typedef vector<Match> MatchList;

  class MatchGroup : public TournamentDatabaseObject
  {
    friend class MatchMngr;
    friend class Match;
    friend class SqliteOverlay::GenericObjectManager<TournamentDB>;
    friend class TournamentDatabaseObjectManager;
    
  public:
    Category getCategory() const;
    int getGroupNumber() const;
    int getRound() const;
    MatchList getMatches() const;
    int getMatchCount() const;
    int getStageSequenceNumber() const;
    bool hasMatchesInState(OBJ_STATE stat) const;
    bool hasMatches__NOT__InState(OBJ_STATE stat) const;

  private:
    DbTab* matchTab;
    MatchGroup(TournamentDB* db, int rowId);
    MatchGroup(TournamentDB* db, SqliteOverlay::TabRow row);
  } ;

}
#endif	/* MATCHGROUP_H */

