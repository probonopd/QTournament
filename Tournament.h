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

using namespace SqliteOverlay;

namespace QTournament
{
  class CategoryTableModel;

  class Tournament : public QObject
  {
    Q_OBJECT
    
  public:
    // Static functions for creating and opening a tournament
    // and for setting / getting the current global tournament instance
    static unique_ptr<Tournament> createNew(const QString& fName, const TournamentSettings& cfg, ERR* err=nullptr);
    static unique_ptr<Tournament> openExisting(const QString& fName, ERR* err=nullptr);
    static void setActiveTournament(Tournament* newTnmt);
    static bool hasActiveTournament();
    static Tournament* getActiveTournament();

    // getters for the various managers and models
    // for this tournament
    TeamMngr* getTeamMngr();
    CatMngr* getCatMngr();
    PlayerMngr* getPlayerMngr();
    TeamListModel* getTeamListModel();
    PlayerTableModel* getPlayerTableModel();
    CategoryTableModel* getCategoryTableModel();
    MatchMngr* getMatchMngr();
    MatchGroupTableModel* getMatchGroupTableModel();
    MatchTableModel* getMatchTableModel();
    CourtTableModel* getCourtTableModel();
    CourtMngr* getCourtMngr();
    RankingMngr* getRankingMngr();
    ReportFactory* getReportFactory();
    TournamentDB* getDatabaseHandle();

    void close();
    ~Tournament();

  private:
    static Tournament* activeTournament;

    Tournament (unique_ptr<TournamentDB> dbHandle);
    unique_ptr<TournamentDB> db;

    unique_ptr<TeamMngr> tm;
    unique_ptr<CatMngr> cm;
    unique_ptr<PlayerMngr> pm;
    unique_ptr<TeamListModel> tlm;
    unique_ptr<PlayerTableModel> ptm;
    unique_ptr<CategoryTableModel> ctm;
    unique_ptr<MatchMngr> mm;
    unique_ptr<MatchGroupTableModel> mgm;
    unique_ptr<MatchTableModel> mam;
    unique_ptr<CourtTableModel> courtMod;
    unique_ptr<CourtMngr> com;
    unique_ptr<RankingMngr> rm;
    unique_ptr<ReportFactory> repFab;
    
    void initManagers ();
    void initModels();

  signals:
    void tournamentClosed();

  };

}

#endif	/* TOURNAMENT_H */

