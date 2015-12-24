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

#ifndef GENERICOBJECTMANAGER_H
#define	GENERICOBJECTMANAGER_H

#include <QString>

#include "SqliteOverlay/GenericObjectManager.h"
#include "SqliteOverlay/DbTab.h"

#include "TournamentDB.h"

namespace QTournament
{
  class GenericObjectManager : public SqliteOverlay::GenericObjectManager
  {
  public:
    GenericObjectManager (TournamentDB* _db, SqliteOverlay::DbTab* _tab);
    GenericObjectManager (TournamentDB* _db, const QString& tabName);
    inline TournamentDB* getTournamentDatabaseHandle() { return tdb; }
    
  protected:
    void fixSeqNumberAfterInsert(const QString& tabName) const;
    void fixSeqNumberAfterDelete(const QString& tabName, int deletedSeqNum) const;

    TournamentDB* tdb;
  };

}

#endif	/* GENERICOBJECTMANAGER_H */

