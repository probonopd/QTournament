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

#ifndef TOURNAMENTDATABASEOBJECTMANAGER_H
#define	TOURNAMENTDATABASEOBJECTMANAGER_H

#include <string>
#include <vector>

#include <QString>

#include <SqliteOverlay/GenericObjectManager.h>

namespace SqliteOverlay
{
  class DbTab;
}

namespace QTournament
{
  class TournamentDB;

  class TournamentDatabaseObjectManager : public SqliteOverlay::GenericObjectManager<TournamentDB>
  {
  public:
    TournamentDatabaseObjectManager (const TournamentDB& _db, const std::string& _tabName)
      : SqliteOverlay::GenericObjectManager<TournamentDB>(_db, _tabName, false) {}

    virtual ~TournamentDatabaseObjectManager() {}

    virtual std::string getSyncString(int rowId = -1) const;
    virtual std::string getSyncString(const std::vector<int>& rows) const { return ""; }

  protected:
    static constexpr int InvalidInitialSequenceNumber = -1;

    void fixSeqNumberAfterInsert() const;
    void fixSeqNumberAfterDelete(int deletedSeqNum) const;
    void fixSeqNumberAfterInsert(const SqliteOverlay::DbTab& otherTab) const;
    void fixSeqNumberAfterDelete(const SqliteOverlay::DbTab& otherTab, int deletedSeqNum) const;
  };

}

#endif	/* TOURNAMENTDATABASEOBJECTMANAGER_H */

