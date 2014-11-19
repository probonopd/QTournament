/* 
 * File:   PlayerPair.cpp
 * Author: volker
 * 
 * Created on March 29, 2014, 7:22 PM
 */

#include <stdexcept>
#include <assert.h>

#include "PlayerPair.h"
#include "PlayerMngr.h"
#include "Tournament.h"

namespace QTournament {

  PlayerPair::PlayerPair(const Player& p1)
  {
    id1 = p1.getId();
    id2 = -1;
    pairId = -1;
  }

//----------------------------------------------------------------------------

  PlayerPair::PlayerPair(const Player& p1, const Player& p2, int _pairId)
  {
    id1 = p1.getId();
    id2 = p2.getId();
    pairId = _pairId;
    sortPlayers();
  }

//----------------------------------------------------------------------------

  PlayerPair::PlayerPair(const Player& p1, int _pairId)
  {
    id1 = p1.getId();
    id2 = -1;
    pairId = _pairId;
  }

//----------------------------------------------------------------------------

  void PlayerPair::sortPlayers()
  {
    // if we have two players, sort the man first
    if (id2 > 0)
    {
      Player p1 = Tournament::getPlayerMngr()->getPlayer(id1);
      Player p2 = Tournament::getPlayerMngr()->getPlayer(id2);
      if ((p2.getSex() == M) && (p1.getSex() == F))
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
    return Tournament::getPlayerMngr()->getPlayer(id1);
  }

//----------------------------------------------------------------------------

  Player PlayerPair::getPlayer2() const
  {
    return Tournament::getPlayerMngr()->getPlayer(id2);
  }

//----------------------------------------------------------------------------

  QString PlayerPair::getDisplayName(int maxLen) const
  {
    if (maxLen < 0)
    {
      maxLen = 0;
    }
    
    if ((maxLen > 0) && (maxLen < 9))
    {
      throw std::invalid_argument("Max len for display name too short!");
    }
    
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
      
      return getPlayer1().getDisplayName(maxLen) + " / " + getPlayer2().getDisplayName(maxLen);
    }
    
    return getPlayer1().getDisplayName(maxLen);
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
      
      return getPlayer1().getTeam().getName(maxLen) + " / " + getPlayer2().getTeam().getName(maxLen);
    }
    
    return getPlayer1().getTeam().getName(maxLen);
  }

//----------------------------------------------------------------------------

  // this serves only as a hot fix until this class will be re-factored to inherit GenericDatabaseObject
  unique_ptr<Category> PlayerPair::getCategory(TournamentDB* db) const
  {
    assert(db != nullptr);

    if (pairId <= 0) return nullptr;

    TabRow pairRow = (db->getTab(TAB_PAIRS))[pairId];
    Category cat = Tournament::getCatMngr()->getCategoryById(pairRow[PAIRS_CAT_REF].toInt());

    return unique_ptr<Category>(new Category(cat));
  }

//----------------------------------------------------------------------------

  // this serves only as a hot fix until this class will be re-factored to inherit GenericDatabaseObject
  //
  // for debugging and unit testing only
  bool PlayerPair::isConsistent(TournamentDB *db) const
  {
    assert(db != nullptr);

    if (pairId > 0)
    {
      TabRow pairRow = (db->getTab(TAB_PAIRS))[pairId];
      if (pairRow[PAIRS_PLAYER1_REF].toInt() != id1) return false;

      QVariant p2Id = pairRow[PAIRS_PLAYER2_REF];
      if (p2Id.isNull() && (id2 > 0)) return false;
      if (!(p2Id.isNull()))
      {
        if (id2 != p2Id.toInt()) return false;
      }
    }

    return true;
  }

//----------------------------------------------------------------------------

  // this serves only as a hot fix until this class will be re-factored to inherit GenericDatabaseObject
  int PlayerPair::getPairsGroupNum(TournamentDB *db) const
  {
    assert(db != nullptr);

    if (pairId <= 0)
    {
      throw runtime_error("Queried PlayerPair does not yet exist in the database");
    }

    TabRow pairRow = (db->getTab(TAB_PAIRS))[pairId];
    return pairRow[PAIRS_GRP_NUM].toInt();
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


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


}
