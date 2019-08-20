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

#ifndef COURTMNGR_H
#define	COURTMNGR_H

#include <memory>

#include <QList>
#include <QObject>

#include <SqliteOverlay/DbTab.h>

#include "TournamentDB.h"
#include "Court.h"
#include "TournamentDataDefs.h"
#include "TournamentErrorCodes.h"
#include "TournamentDatabaseObjectManager.h"


namespace QTournament
{

  using CourtOrError = ObjectOrError<Court>;

  class CourtMngr : public QObject, public TournamentDatabaseObjectManager
  {
    Q_OBJECT
    
  public:
    CourtMngr (const TournamentDB& _db);
    CourtOrError createNewCourt (const int courtNum, const QString& _name);
    bool hasCourt (const int courtNum);
    int getHighestUnusedCourtNumber() const;
    std::optional<Court> getCourt(const int courtNum);
    std::vector<Court> getAllCourts();
    ERR renameCourt (Court& c, const QString& _newName);
    std::optional<Court> getCourtBySeqNum(int seqNum);
    bool hasCourtById(int id);
    std::optional<Court> getCourtById(int id);
    int getActiveCourtCount();

    std::optional<Court> getNextUnusedCourt(bool includeManual=false) const;
    std::optional<Court> autoSelectNextUnusedCourt(ERR* err, bool includeManual=false) const;

    bool acquireCourt(const Court& co);
    bool releaseCourt(const Court& co);

    ERR disableCourt(const Court& co);
    ERR enableCourt(const Court& co);

    ERR deleteCourt(const Court& co);

    std::string getSyncString(const std::vector<int>& rows) const override;

  private:

  signals:
  };
}

#endif	/* COURTMNGR_H */

