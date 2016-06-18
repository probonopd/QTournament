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

#ifndef COURTMNGR_H
#define	COURTMNGR_H

#include <memory>

#include <QList>
#include <QObject>

#include "DbTab.h"

#include "TournamentDB.h"
#include "Court.h"
#include "TournamentDataDefs.h"
#include "TournamentErrorCodes.h"
#include "TournamentDatabaseObjectManager.h"


namespace QTournament
{

  class CourtMngr : public QObject, TournamentDatabaseObjectManager
  {
    Q_OBJECT
    
  public:
    CourtMngr (TournamentDB* _db);
    unique_ptr<Court> createNewCourt (const int courtNum, const QString& _name, ERR *err);
    bool hasCourt (const int courtNum);
    int getHighestUnusedCourtNumber() const;
    unique_ptr<Court> getCourt(const int courtNum);
    vector<Court> getAllCourts();
    ERR renameCourt (Court& c, const QString& _newName);
    unique_ptr<Court> getCourtBySeqNum(int seqNum);
    bool hasCourtById(int id);
    unique_ptr<Court> getCourtById(int id);
    int getActiveCourtCount();

    unique_ptr<Court> getNextUnusedCourt(bool includeManual=false) const;
    unique_ptr<Court> autoSelectNextUnusedCourt(ERR* err, bool includeManual=false) const;

    bool acquireCourt(const Court& co);
    bool releaseCourt(const Court& co);

    ERR disableCourt(const Court& co);
    ERR enableCourt(const Court& co);

    ERR deleteCourt(const Court& co);

  private:

  signals:
  };
}

#endif	/* COURTMNGR_H */

