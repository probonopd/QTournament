/* 
 * File:   TeamMngr.h
 * Author: nyoknvk1
 *
 * Created on 18. Februar 2014, 14:04
 */

#ifndef PLAYERMNGR_H
#define	PLAYERMNGR_H

#include <functional>
#include <memory>

#include <QList>
#include <QObject>

#include "TournamentDB.h"
#include "Team.h"
#include "Player.h"
#include "PlayerPair.h"
#include "TournamentDataDefs.h"
#include "TournamentErrorCodes.h"
#include "DbTab.h"
#include "GenericObjectManager.h"
#include "Match.h"


using namespace dbOverlay;

namespace QTournament
{

  class PlayerMngr : public QObject, GenericObjectManager
  {
    Q_OBJECT
    
  public:
    PlayerMngr (TournamentDB* _db);
    ERR createNewPlayer (const QString& firstName, const QString& lastName, SEX sex, const QString& teamName);
    bool hasPlayer (const QString& firstName, const QString& lastName);
    Player getPlayer(const QString& firstName, const QString& lastName);
    QList<Player> getAllPlayers();
    ERR renamePlayer (Player& p, const QString& newFirst, const QString& newLast);
    unique_ptr<Player> getPlayerBySeqNum(int seqNum);
    bool hasPlayer(int id);
    Player getPlayer(int id);
    unique_ptr<Player> getPlayer_up(int id) const;
    PlayerPair getPlayerPair(int id);

    ERR canAcquirePlayerPairsForMatch(const Match& ma);
    ERR acquirePlayerPairsForMatch(const Match& ma);
    ERR releasePlayerPairsAfterMatch(const Match& ma);

    PlayerList determineActualPlayersForMatch(const Match& ma) const;

    ERR setWaitForRegistration(const Player& p, bool waitForPlayerRegistration) const;

    static std::function<bool (Player&, Player&)> getPlayerSortFunction_byName();

    ERR canDeletePlayer(const Player& p) const;
    ERR deletePlayer(const Player& p) const;

    int getTotalPlayerCount() const;

  private:
    DbTab playerTab;

  signals:
    void beginCreatePlayer ();
    void endCreatePlayer (int newPlayerSeqNum);
    void playerRenamed (const Player& p);
    void playerStatusChanged(int playerId, int playerSeqNum, OBJ_STATE fromState, OBJ_STATE toState) const;
    void beginDeletePlayer(int playerSeqNum) const;
    void endDeletePlayer() const;
  };
}

#endif	/* TEAMMNGR_H */

