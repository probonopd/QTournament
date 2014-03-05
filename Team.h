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

namespace QTournament
{

  class Team : public GenericDatabaseObject
  {
    friend class TeamMngr;
    
  public:
    QString getName();

  private:
    Team (TournamentDB* db, int rowId);
    Team (dbOverlay::TabRow row);
  };

}
#endif	/* TEAM_H */

