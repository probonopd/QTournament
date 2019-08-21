/*
 *    This is QTournament, a badminton tournament management program.
 *    Copyright (C) 2014 - 2017  Volker Knollmann
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

#include <stdexcept>
#include <assert.h>

#include "PlayerPair.h"
#include "PlayerMngr.h"
#include "CatMngr.h"

using namespace SqliteOverlay;

namespace QTournament {

  // ctor for a player pair with a single player only and without database entry
  PlayerPair::PlayerPair(const Player& p1)
    : id1(p1.getId()), id2(-1), pairId(-1), db(p1.getDatabaseHandle())
  {
  }

//----------------------------------------------------------------------------

  // ctor for a player pair with two players and an optional database entry (_pairId may be -1)
  PlayerPair::PlayerPair(const Player& p1, const Player& p2, int _pairId)
    : id1(p1.getId()), id2(p2.getId()), pairId(_pairId), db(p1.getDatabaseHandle())
  {
    sortPlayers();
  }

//----------------------------------------------------------------------------

  // ctor for a player pair with a single player only and with database entry
  PlayerPair::PlayerPair(const Player& p1, int _pairId)
    : id1(p1.getId()), id2(-1), pairId(_pairId), db(p1.getDatabaseHandle())
  {
  }

//----------------------------------------------------------------------------

  // ctor for a PlayerPair constructed from a row in TabPairs
  PlayerPair::PlayerPair(const TournamentDB& _db, const TabRow& row)
    :id1(row.getInt(Pairs_Player1Ref)), id2(-1), pairId(row.id()), db(_db)
  {
    auto _id2 = row.getInt2(Pairs_Player2Ref);
    if (_id2.has_value())
    {
      id2 = *_id2;
      sortPlayers();
    }
  }

//----------------------------------------------------------------------------

  // ctor for a PlayerPair constructed from a row in TabPairs identified by its ID
  PlayerPair::PlayerPair(const TournamentDB& _db, int ppId)
    :db(_db)
  {
    TabRow row{db, TabPairs, ppId};

    pairId = ppId;
    id1 = row.getInt(Pairs_Player1Ref);
    id2 = -1;

    auto _id2 = row.getInt2(Pairs_Player2Ref);
    if (_id2.has_value())
    {
      id2 = *_id2;
      sortPlayers();
    }
  }

//----------------------------------------------------------------------------

  void PlayerPair::sortPlayers()
  {
    PlayerMngr pm{db};

    // if we have two players, sort the man first
    if (id2 > 0)
    {
      Player p1 = pm.getPlayer(id1);
      Player p2 = pm.getPlayer(id2);
      if ((p2.getSex() == Sex::M) && (p1.getSex() == Sex::F))
      {
        id1 = p2.getId();
        id2 = p1.getId();
      }
    }
    
  }

//----------------------------------------------------------------------------

  bool PlayerPair::hasPlayer2() const
  {
    return (id2 > 0);
  }

//----------------------------------------------------------------------------

  Player PlayerPair::getPlayer1() const
  {
    PlayerMngr pm{db};
    return pm.getPlayer(id1);
  }

//----------------------------------------------------------------------------

  Player PlayerPair::getPlayer2() const
  {
    PlayerMngr pm{db};
    return pm.getPlayer(id2);
  }

//----------------------------------------------------------------------------

  QString PlayerPair::getDisplayName(int maxLen, bool unregisteredPlayersInBrackets) const
  {
    if (maxLen < 0)
    {
      maxLen = 0;
    }
    
    if ((maxLen > 0) && (maxLen < 9))
    {
      throw std::invalid_argument("Max len for display name too short!");
    }
    
    // prepare strings for the player names and for the result
    QString result;

    Player p1 = getPlayer1();
    ObjState p1Stat = p1.getState();

    QString p2Name;
    if (hasPlayer2())
    {
      // reserve space for " / " if we have two players
      maxLen = (maxLen == 0) ? 0 : maxLen - 3;
      
      // and cut the max len in half
      if ((maxLen % 2) != 0)
      {
        maxLen -= 1;
      }
      maxLen = maxLen / 2;

      Player p2 = getPlayer2();
      p2Name = p2.getDisplayName(maxLen);
      ObjState p2Stat = p2.getState();

      result = "%2 / %1";
      if (unregisteredPlayersInBrackets)
      {
        if ((p1Stat == ObjState::PL_WaitForRegistration) && (p2Stat == ObjState::PL_WaitForRegistration))
        {
          result = "(%2 / %1)";
        } else if (p1Stat == ObjState::PL_WaitForRegistration)
        {
          result = "(%2) / %1";
        } else if (p2Stat == ObjState::PL_WaitForRegistration)
        {
          result = "%2 / (%1)";
        }
      }
      result = result.arg(p2Name);
    } else {
      result = (unregisteredPlayersInBrackets && (p1Stat == ObjState::PL_WaitForRegistration)) ? "(%1)" : "%1";
    }

    result = result.arg(p1.getDisplayName(maxLen));

    return result;
  }

//----------------------------------------------------------------------------

  QString PlayerPair::getDisplayName_FirstNameFirst() const
  {
    QString result = getPlayer1().getDisplayName_FirstNameFirst();
    if (hasPlayer2())
    {
      result += " / ";
      result += getPlayer2().getDisplayName_FirstNameFirst();
    }

    return result;
  }

//----------------------------------------------------------------------------

  int PlayerPair::getPairId() const
  {
    return pairId;
  }

//----------------------------------------------------------------------------
  
  QString PlayerPair::getDisplayName_Team(int maxLen) const
  {
    if (maxLen < 0)
    {
      maxLen = 0;
    }
    
    if ((maxLen > 0) && (maxLen < 9))
    {
      throw std::invalid_argument("Max len for display name too short!");
    }
    
    Team t1 = getPlayer1().getTeam();
    if (hasPlayer2())
    {
      // if we have different teams, return "team 1 / team 2", else return just "team"
      Team t2 = getPlayer2().getTeam();
      if (t1 != t2)
      {
        // reserve space for " / "
        maxLen = (maxLen == 0) ? 0 : maxLen - 3;

        // determine the full length of both team names
        QString name1 = t1.getName();
        QString name2 = t2.getName();

        // if there is no length limit, return both full names
        if (maxLen == 0)
        {
          return name1 + " / " + name2;
        }

        // if both fit in the specified length ==> okay!
        int len1 = name1.length();
        int len2 = name2.length();
        if ((len1 + len2) <= maxLen)
        {
          return name1 + " / " + name2;
        }

        // calculate by how much we exceed the max length
        // add distribute the delta over both names
        int diff = (len1 + len2) - maxLen;
        if ((diff % 2) != 0)
        {
          ++diff;
        }
        diff = diff / 2;

        // shorten both names by "diff" characters
        name1 = name1.left(len1 - diff);
        name2 = name2.left(len2 - diff);

        return name1 + " / " + name2;
      }
    }
    
    return t1.getName(maxLen);
  }

//----------------------------------------------------------------------------

  // this serves only as a hot fix until this class will be re-factored to inherit GenericDatabaseObject
  std::optional<Category> PlayerPair::getCategory(const TournamentDB& db) const
  {
    if (pairId <= 0) return {};

    TabRow pairRow{db, TabPairs, pairId};
    CatMngr cm{db};
    return cm.getCategoryById(pairRow.getInt(Pairs_CatRef));
  }

//----------------------------------------------------------------------------

  // this serves only as a hot fix until this class will be re-factored to inherit GenericDatabaseObject
  //
  // for debugging and unit testing only
  bool PlayerPair::isConsistent() const
  {
    if (pairId > 0)
    {
      TabRow pairRow{db, TabPairs, pairId};
      if (pairRow.getInt(Pairs_Player1Ref) != id1) return false;

      auto p2Id = pairRow.getInt2(Pairs_Player2Ref);
      if (!p2Id.has_value() && (id2 > 0)) return false;
      if ((p2Id.has_value()) && (id2 != *p2Id)) return false;
    }

    return true;
  }

//----------------------------------------------------------------------------

  // this serves only as a hot fix until this class will be re-factored to inherit GenericDatabaseObject
  int PlayerPair::getPairsGroupNum() const
  {
    if (pairId <= 0)
    {
      throw std::runtime_error("Queried PlayerPair does not yet exist in the database");
    }

    TabRow pairRow{db, TabPairs, pairId};
    return pairRow.getInt(Pairs_GrpNum);
  }

//----------------------------------------------------------------------------

  bool PlayerPair::areAllPlayersIdle() const
  {
    auto p = getPlayer1();
    if (p.is_NOT_InState(ObjState::PL_Idle)) return false;
    if (hasPlayer2())
    {
      p = getPlayer2();
      if (p.is_NOT_InState(ObjState::PL_Idle)) return false;
    }
    return true;
  }

//----------------------------------------------------------------------------

  QString PlayerPair::getCallName(const QString& sepString) const
  {
    auto p = getPlayer1();
    QString result = p.getFirstName() + " " + p.getLastName();
    if (hasPlayer2())
    {
      p = getPlayer2();
      result += "    " + sepString + "    ";
      result += p.getFirstName() + " " + p.getLastName();
    }

    return result;
  }

//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


}
