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

#ifndef TOURNAMENT_H
#define	TOURNAMENT_H

#include <memory>

#include <QString>
#include <QObject>

#include "TournamentDataDefs.h"
#include "TournamentDB.h"
#include "KeyValueTab.h"
#include "TeamMngr.h"
#include "CatMngr.h"
#include "PlayerMngr.h"
#include "models/TeamListModel.h"
#include "models/PlayerTableModel.h"
#include "models/CatTableModel.h"
#include "MatchMngr.h"
#include "models/MatchGroupTabModel.h"
#include "models/MatchTabModel.h"
#include "models/CourtTabModel.h"
#include "CourtMngr.h"
#include "RankingMngr.h"
#include "reports/ReportFactory.h"

using namespace dbOverlay;

namespace QTournament
{
  class Tournament : public QObject
  {
    Q_OBJECT
    
  public:
    Tournament (const QString& fName, const TournamentSettings& cfg);
    Tournament (const QString& fName);
    static TeamMngr* getTeamMngr();
    static CatMngr* getCatMngr();
    static PlayerMngr* getPlayerMngr();
    static TeamListModel* getTeamListModel();
    static PlayerTableModel* getPlayerTableModel();
    static CategoryTableModel* getCategoryTableModel();
    static MatchMngr* getMatchMngr();
    static MatchGroupTableModel* getMatchGroupTableModel();
    static MatchTableModel* getMatchTableModel();
    static CourtTableModel* getCourtTableModel();
    static CourtMngr* getCourtMngr();
    static RankingMngr* getRankingMngr();
    static ReportFactory* getReportFactory();
    static TournamentDB* getDatabaseHandle();
    void close();
    ~Tournament();

  private:
    static TournamentDB* db;
    static TeamMngr* tm;
    static CatMngr* cm;
    static PlayerMngr* pm;
    static TeamListModel* tlm;
    static PlayerTableModel *ptm;
    static CategoryTableModel *ctm;
    static MatchMngr* mm;
    static MatchGroupTableModel* mgm;
    static MatchTableModel* mam;
    static CourtTableModel* courtMod;
    static CourtMngr* com;
    static RankingMngr* rm;
    static ReportFactory* repFab;
    
    void initManagers ();
    void initModels();

  signals:
    void tournamentClosed();

  };

}

#endif	/* TOURNAMENT_H */

