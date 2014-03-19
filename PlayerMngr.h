/* 
 * File:   TeamMngr.h
 * Author: nyoknvk1
 *
 * Created on 18. Februar 2014, 14:04
 */

#ifndef PLAYERMNGR_H
#define	PLAYERMNGR_H

#include "TournamentDB.h"
#include "Team.h"
#include "Player.h"
#include "TournamentDataDefs.h"
#include "TournamentErrorCodes.h"
#include "DbTab.h"
#include "GenericObjectManager.h"

#include <QList>
#include <QObject>

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
    Player getPlayerBySeqNum(int seqNum);

  private:
    DbTab playerTab;
    
  signals:
    void newPlayerCreated(const Player& newPlayer);
  };
}

#endif	/* TEAMMNGR_H */

