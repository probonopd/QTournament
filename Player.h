/* 
 * File:   Team.h
 * Author: volker
 *
 * Created on March 2, 2014, 6:13 PM
 */

#ifndef PLAYER_H
#define	PLAYER_H

#include "GenericDatabaseObject.h"
#include "TournamentDB.h"
#include "TabRow.h"
#include "TournamentErrorCodes.h"
#include "Team.h"

namespace QTournament
{

  class Player : public GenericDatabaseObject
  {
    friend class PlayerMngr;
    
  public:
    QString getDisplayName(int maxLen = 0) const;
    QString getFirstName() const;
    QString getLastName() const;
    ERR rename(const QString& newFirst, const QString& newLast);
    SEX getSex() const;
    Team getTeam() const;

  private:
    Player (TournamentDB* db, int rowId);
    Player (TournamentDB* db, dbOverlay::TabRow row);
  };

}
#endif	/* TEAM_H */

