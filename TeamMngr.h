/* 
 * File:   TeamMngr.h
 * Author: nyoknvk1
 *
 * Created on 18. Februar 2014, 14:04
 */

#ifndef TEAMMNGR_H
#define	TEAMMNGR_H

#include "TournamentDB.h"
#include "Team.h"
#include "TournamentDataDefs.h"
#include "TournamentErrorCodes.h"
#include "DbTab.h"
#include "GenericObjectManager.h"

#include <QList>
#include <QObject>

using namespace dbOverlay;

namespace QTournament
{

  class TeamMngr : public QObject, GenericObjectManager
  {
  Q_OBJECT
  
  public:
    TeamMngr (TournamentDB* _db);
    ERR createNewTeam (const QString& teamName);
    bool hasTeam (const QString& teamName);
    Team getTeam(const QString& name);
    QList<Team> getAllTeams();
    ERR renameTeam (Team& t, const QString& nn);
    Team getTeamBySeqNum(int seqNum);
    Team getTeamById(int id);

  private:
    DbTab teamTab;
    
  signals:
    void newTeamCreated(const Team& newTeam);
  };
}

#endif	/* TEAMMNGR_H */

