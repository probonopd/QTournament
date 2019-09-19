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

#ifndef GENERICDATABASEOBJECT_H
#define	GENERICDATABASEOBJECT_H

#include <QString>

#include <SqliteOverlay/TabRow.h>
#include <SqliteOverlay/GenericDatabaseObject.h>
#include "TournamentDB.h"

namespace QTournament
{

  class TournamentDatabaseObject : public SqliteOverlay::GenericDatabaseObject<TournamentDB>
  {
  public:
    TournamentDatabaseObject (const TournamentDB& _db, const QString& _tabName, int _id)
      : SqliteOverlay::GenericDatabaseObject<TournamentDB>(_db, _tabName.toUtf8().constData(), _id) {}

    TournamentDatabaseObject (const TournamentDB& _db, const SqliteOverlay::TabRow& _row)
      : SqliteOverlay::GenericDatabaseObject<TournamentDB>(_db, _row) {}
    
    ObjState getState() const;
    bool isInState(ObjState cmpState) const { return (getState() == cmpState); }
    bool is_NOT_InState(ObjState cmpState) const { return (getState() != cmpState); }
    void setState(ObjState newState) const;
    int getSeqNum() const;

    const SqliteOverlay::TabRow& rowRef() const { return row; }

  };
}

#endif	/* GENERICDATABASEOBJECT_H */

