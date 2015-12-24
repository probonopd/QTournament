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

#ifndef GENERICDATABASEOBJECT_H
#define	GENERICDATABASEOBJECT_H

#include <QString>

#include "SqliteOverlay/TabRow.h"
#include "SqliteOverlay/GenericDatabaseObject.h"
#include "TournamentDB.h"

namespace QTournament
{

  class GenericDatabaseObject : public SqliteOverlay::GenericDatabaseObject
  {
  public:
    GenericDatabaseObject (SqliteOverlay::SqliteDatabase* _db, const QString& _tabName, int _id);
    GenericDatabaseObject (SqliteOverlay::SqliteDatabase* _db, SqliteOverlay::TabRow _row);
    
    OBJ_STATE getState() const;
    void setState(OBJ_STATE newState) const;
    int getSeqNum() const;

  };
}

#endif	/* GENERICDATABASEOBJECT_H */

