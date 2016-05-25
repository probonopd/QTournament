/*
 *    This is QTournament, a badminton tournament management program.
 *    Copyright (C) 2014 - 2016  Volker Knollmann
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
#include "TabRow.h"
#include "TournamentErrorCodes.h"

namespace QTournament
{

  class Team : public TournamentDatabaseObject
  {
    friend class TeamMngr;
    friend class SqliteOverlay::GenericObjectManager<TournamentDB>;
    friend class TournamentDatabaseObjectManager;

  public:
    QString getName(int maxLen=0) const;
    ERR rename(const QString& newName);

  private:
    Team (TournamentDB* db, int rowId);
    Team (TournamentDB* db, SqliteOverlay::TabRow row);
  };

}
#endif	/* TEAM_H */

