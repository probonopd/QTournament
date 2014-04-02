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
#include "PlayerPair.h"
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
    bool hasPlayer(const Player& p) const;
    ERR addPlayer(const Player& p);
    bool canRemovePlayer(const Player& p) const;
    ERR removePlayer(const Player& p);
    CAT_ADD_STATE getAddState(const SEX s) const;
    CAT_ADD_STATE getAddState(const Player& p) const;
    QVariant getParameter(CAT_PARAMETER) const;
    int getParameter_int(CAT_PARAMETER) const;
    bool getParameter_bool(CAT_PARAMETER) const;
    bool setParameter(CAT_PARAMETER p, const QVariant& v);
    QList<PlayerPair> getPlayerPairs();
    QList<Player> getAllPlayersInCategory();
    bool isPaired(const Player& p) const;
    ERR canPairPlayers(const Player& p1, const Player& p2) const;
    ERR canSplitPlayers(const Player& p1, const Player& p2) const;
    Player getPartner(const Player& p) const;

  private:
    Category (TournamentDB* db, int rowId);
    Category (TournamentDB* db, dbOverlay::TabRow row);
  };

  // we need this to have a category object in a QHash
  inline uint qHash(const Category& c)
  {
    return c.getId(); // bad style: implicit conversion from int to uint...
  }

}
#endif	/* TEAM_H */

