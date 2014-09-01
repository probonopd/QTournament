/* 
 * File:   PlayerPair.h
 * Author: volker
 *
 * Created on March 29, 2014, 7:22 PM
 */

#ifndef PLAYERPAIR_H
#define	PLAYERPAIR_H

#include "Player.h"

namespace QTournament
{

  class PlayerPair
  {
  public:
    PlayerPair(const Player& p1, const Player& p2, int _pairId);
    PlayerPair(const Player& p1, int _pairId);
    PlayerPair(const Player& p1);
    bool hasPlayer2() const;
    Player getPlayer1();
    Player getPlayer2();
    QString getDisplayName(int maxLen = 0);
    int getPairId();

  private:
    int id1;
    int id2;
    int pairId;
    void sortPlayers();
  } ;

}
#endif	/* PLAYERPAIR_H */

