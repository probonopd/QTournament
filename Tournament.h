/* 
 * File:   Tournament.h
 * Author: nyoknvk1
 *
 * Created on 18. Februar 2014, 12:31
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
    void close();
    ~Tournament();

  private:
    TournamentDB* db;
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

