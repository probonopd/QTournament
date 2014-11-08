/* 
 * File:   Category.h
 * Author: volker
 *
 * Created on March 2, 2014, 6:13 PM
 */

#ifndef CATEGORY_H
#define	CATEGORY_H

#include <memory>

#include "GenericDatabaseObject.h"
#include "TournamentDB.h"
#include "TabRow.h"
#include "Player.h"
#include "PlayerPair.h"
#include "TournamentErrorCodes.h"
#include "KO_Config.h"

namespace QTournament
{

  class Category : public GenericDatabaseObject
  {
    friend class CatMngr;
    friend class RoundRobinCategory;
    
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
    QString getParameter_string(CAT_PARAMETER) const;
    bool setParameter(CAT_PARAMETER p, const QVariant& v);
    PlayerPairList getPlayerPairs() const;
    QList<Player> getAllPlayersInCategory() const;
    bool isPaired(const Player& p) const;
    ERR canPairPlayers(const Player& p1, const Player& p2) const;
    ERR canSplitPlayers(const Player& p1, const Player& p2) const;
    Player getPartner(const Player& p) const;
    bool hasUnpairedPlayers() const;
    unique_ptr<Category> convertToSpecializedObject() const;
    ERR canApplyGroupAssignment(QList<PlayerPairList> grpCfg);
    ERR canApplyInitialRanking(PlayerPairList seed);

    
    virtual ~Category() {};
    
    //
    // The following methods MUST be overwritten by derived classes for a specific match system
    //
    virtual ERR canFreezeConfig() { throw std::runtime_error("Unimplemented Method: canFreezeConfig"); };
    virtual bool needsInitialRanking() { throw std::runtime_error("Unimplemented Method: needsInitialRanking"); };
    virtual bool needsGroupInitialization() { throw std::runtime_error("Unimplemented Method: needsGroupInitialization"); };
    virtual ERR prepareFirstRound(QList<PlayerPairList> grpCfg, PlayerPairList seed) { throw std::runtime_error("Unimplemented Method: prepareFirstRound"); };

  private:
    Category (TournamentDB* db, int rowId);
    Category (TournamentDB* db, dbOverlay::TabRow row);
    ERR applyGroupAssignment(QList<PlayerPairList> grpCfg);
    ERR applyInitialRanking(PlayerPairList seed);
  };

  // we need this to have a category object in a QHash
  inline uint qHash(const Category& c)
  {
    return c.getId(); // bad style: implicit conversion from int to uint...
  }

}
#endif	/* TEAM_H */

