/*
 *    This is QTournament, a badminton tournament management program.
 *    Copyright (C) 2014 - 2016  Volker Knollmann
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PLAYERPAIR_H
#define	PLAYERPAIR_H

#include <memory>
#include <vector>

#include "Player.h"
#include "TournamentDB.h"

namespace QTournament
{

  class PlayerPair
  {
  public:
    // constructors
    PlayerPair(const Player& p1, const Player& p2, int _pairId);
    PlayerPair(const Player& p1, int _pairId);
    PlayerPair(const Player& p1);
    PlayerPair(TournamentDB* _db, const SqliteOverlay::TabRow& row);
    PlayerPair(TournamentDB* _db, int ppId);

    // boolean queries
    bool hasPlayer2() const;
    bool isConsistent() const; // for debugging only
    bool areAllPlayersIdle() const;

    // getters
    Player getPlayer1() const;
    Player getPlayer2() const;
    QString getDisplayName(int maxLen = 0, bool unregisteredPlayersInBrackets=false) const;
    QString getDisplayName_FirstNameFirst() const;
    QString getDisplayName_Team(int maxLen = 0) const;
    QString getCallName(const QString &sepString) const;
    int getPairId() const;
    unique_ptr<Category> getCategory(TournamentDB* db) const;  // only as a hot-fix
    int getPairsGroupNum() const;  // only as a hot-fix

    inline bool operator == (const PlayerPair& other) const {
      // case 1: we have player pairs with a database ID
      if ((pairId > 0) && (other.pairId > 0))
      {
        return (pairId == other.pairId);
      }

      // case 2: we have player pairs "in memory" without
      // database entry
      return ((id1 == other.id1) && (id2 == other.id2));
    }
    inline bool operator != (const PlayerPair& other) const {
      return (!(this->operator ==(other)));
    }

  private:
    int id1;
    int id2;
    int pairId;
    TournamentDB* db;
    void sortPlayers();
  } ;
  
  typedef vector<PlayerPair> PlayerPairList;
  typedef unique_ptr<PlayerPair> upPlayerPair;

}
#endif	/* PLAYERPAIR_H */

