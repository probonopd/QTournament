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

#ifndef TEAM_H
#define	TEAM_H

#include "TournamentDatabaseObject.h"
#include "TournamentDatabaseObjectManager.h"
#include "TournamentDB.h"
#include <SqliteOverlay/TabRow.h>
#include "TournamentErrorCodes.h"

namespace QTournament
{

  class Team : public TournamentDatabaseObject
  {
  public:
    QString getName(int maxLen=0) const;
    Error rename(const QString& newName);
    int getMemberCount() const;
    int getUnregisteredMemberCount() const;

    Team (const TournamentDB& _db, int rowId);
    Team (const TournamentDB& _db, const SqliteOverlay::TabRow& _row);
  };

}
#endif	/* TEAM_H */

