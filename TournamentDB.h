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

#ifndef TOURNAMENTDB_H
#define	TOURNAMENTDB_H

#include <tuple>

#include "SqliteDatabase.h"

#include "TournamentDataDefs.h"
#include "TournamentErrorCodes.h"

namespace QTournament
{

  class TournamentDB : public SqliteOverlay::SqliteDatabase
  {
    friend class SqliteOverlay::SqliteDatabase;

  public:
    static unique_ptr<TournamentDB> createNew(const QString& fName, const TournamentSettings& cfg, ERR* err=nullptr);
    static unique_ptr<TournamentDB> openExisting(const QString& fName, ERR* err=nullptr);

    virtual void populateTables();
    virtual void populateViews();
    void createIndices();

    tuple<int, int> getVersion();

    bool isCompatibleDatabaseVersion();

    bool needsConversion();
    bool convertToLatestDatabaseVersion();

  private:
    TournamentDB(string fName, bool createNew);
  };
}

#endif	/* TOURNAMENTDB_H */

