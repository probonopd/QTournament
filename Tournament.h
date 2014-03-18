/* 
 * File:   Tournament.h
 * Author: nyoknvk1
 *
 * Created on 18. Februar 2014, 12:31
 */

#ifndef TOURNAMENT_H
#define	TOURNAMENT_H

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
    void close();
    ~Tournament();

  private:
    TournamentDB db;
    static TeamMngr* tm;
    static CatMngr* cm;
    static PlayerMngr* pm;
    static TeamListModel* tlm;
    static PlayerTableModel *ptm;
    
    void initManagers ();
    void initModels();

  signals:
    void tournamentClosed();

  };

}

#endif	/* TOURNAMENT_H */

