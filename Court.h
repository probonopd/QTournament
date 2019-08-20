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

#ifndef COURT_H
#define	COURT_H

#include <memory>

#include <SqliteOverlay/TabRow.h>
#include <SqliteOverlay/GenericObjectManager.h>

#include "TournamentDatabaseObject.h"
#include "TournamentDB.h"

#include "TournamentErrorCodes.h"

namespace QTournament
{
  class Match;

  class Court : public TournamentDatabaseObject
  {

    friend class CourtMngr;
    friend class GenericObjectManager;
    friend class SqliteOverlay::GenericObjectManager<TournamentDB>;

  public:
    QString getName(int maxLen = 0) const;
    Error rename(const QString& newName);
    int getNumber() const;
    bool isManualAssignmentOnly() const;
    void setManualAssignment(bool isManual);
    std::optional<Match> getMatch() const;

  private:
    Court (const TournamentDB& db, int rowId);
    Court (const TournamentDB& _db, const SqliteOverlay::TabRow& _row);
  };
  using CourtList = std::vector<Court>;

}
#endif	/* COURT_H */

