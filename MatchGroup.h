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

#ifndef MATCHGROUP_H
#define	MATCHGROUP_H

#include "GenericDatabaseObject.h"
#include "TournamentDB.h"
#include "TabRow.h"
#include "TournamentErrorCodes.h"
#include "Category.h"
#include "Match.h"
#include "DbTab.h"

#include <QList>

using namespace dbOverlay;

namespace QTournament
{

  typedef QList<Match> MatchList;

  class MatchGroup : public GenericDatabaseObject
  {
    friend class MatchMngr;
    friend class Match;
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
    MatchGroup(TournamentDB* db, int rowId);
    MatchGroup(TournamentDB* db, dbOverlay::TabRow row);
    DbTab matchTab;
  } ;

}
#endif	/* MATCHGROUP_H */

