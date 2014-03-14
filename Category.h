/* 
 * File:   Category.h
 * Author: volker
 *
 * Created on March 2, 2014, 6:13 PM
 */

#ifndef CATEGORY_H
#define	CATEGORY_H

#include "GenericDatabaseObject.h"
#include "TournamentDB.h"
#include "TabRow.h"
#include "Player.h"
#include "TournamentErrorCodes.h"

namespace QTournament
{

  class Category : public GenericDatabaseObject
  {
    friend class CatMngr;
    
  public:
    QString getName() const;
    ERR rename(const QString& newName);
    MATCH_TYPE getMatchType() const;
    ERR setMatchType(MATCH_TYPE t);
    MATCH_SYSTEM getMatchSystem() const;
    ERR setMatchSystem(MATCH_SYSTEM s);
    SEX getSex() const;
    ERR setSex(SEX s);
    bool canAddPlayers() const;
    bool isPlayerSuitable(const Player& p) const;
    bool hasPlayer(const Player& p) const;
    ERR addPlayer(const Player& p);
    bool canRemovePlayer(const Player& p) const;
    ERR removePlayer(const Player& p);

  private:
    Category (TournamentDB* db, int rowId);
    Category (TournamentDB* db, dbOverlay::TabRow row);
  };

}
#endif	/* TEAM_H */

