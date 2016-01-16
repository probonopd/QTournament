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

#ifndef COURT_H
#define	COURT_H

#include <memory>

#include "SqliteOverlay/TabRow.h"
#include "SqliteOverlay/GenericObjectManager.h"

#include "GenericDatabaseObject.h"
#include "TournamentDB.h"

#include "TournamentErrorCodes.h"

namespace QTournament
{
  class Match;

  class Court : public GenericDatabaseObject
  {

    friend class CourtMngr;
    friend class GenericObjectManager;
    friend class SqliteOverlay::GenericObjectManager<TournamentDB>;

  public:
    QString getName(int maxLen = 0) const;
    ERR rename(const QString& newName);
    int getNumber() const;
    bool isManualAssignmentOnly() const;
    void setManualAssignment(bool isManual);
    unique_ptr<Match> getMatch() const;

  private:
    Court (SqliteOverlay::SqliteDatabase* db, int rowId);
    Court (SqliteOverlay::SqliteDatabase* db, const SqliteOverlay::TabRow& row);
  };
  typedef vector<Court> CourtList;

}
#endif	/* COURT_H */

