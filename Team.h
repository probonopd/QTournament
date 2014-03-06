/* 
 * File:   Team.h
 * Author: volker
 *
 * Created on March 2, 2014, 6:13 PM
 */

#ifndef TEAM_H
#define	TEAM_H

#include "GenericDatabaseObject.h"
#include "TournamentDB.h"
#include "TabRow.h"
#include "TournamentErrorCodes.h"

namespace QTournament
{

  class Team : public GenericDatabaseObject
  {
    friend class TeamMngr;
    
  public:
    QString getName();
    ERR rename(const QString& newName);

  private:
    Team (TournamentDB* db, int rowId);
    Team (TournamentDB* db, dbOverlay::TabRow row);
  };

}
#endif	/* TEAM_H */

