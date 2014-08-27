/* 
 * File:   Category.cpp
 * Author: volker
 * 
 * Created on March 2, 2014, 6:13 PM
 */

#include "Category.h"
#include "TournamentDataDefs.h"
#include "TournamentDB.h"
#include "TournamentErrorCodes.h"
#include "CatMngr.h"
#include "Tournament.h"
#include "RoundRobinCategory.h"

#include <stdexcept>

namespace QTournament
{

  Category::Category(TournamentDB* db, int rowId)
  :GenericDatabaseObject(db, TAB_CATEGORY, rowId)
  {
  }

//----------------------------------------------------------------------------

  Category::Category(TournamentDB* db, dbOverlay::TabRow row)
  :GenericDatabaseObject(db, row)
  {
  }

//----------------------------------------------------------------------------

  QString Category::getName() const
  {
    return row[GENERIC_NAME_FIELD_NAME].toString();
  }

//----------------------------------------------------------------------------

  ERR Category::rename(const QString& nn)
  {
    return Tournament::getCatMngr()->renameCategory(*this, nn);
  }

//----------------------------------------------------------------------------

  MATCH_SYSTEM Category::getMatchSystem() const
  {
    int sysInt = row[CAT_SYS].toInt();
    
    return static_cast<MATCH_SYSTEM>(sysInt);
  }

//----------------------------------------------------------------------------

  MATCH_TYPE Category::getMatchType() const
  {
    int typeInt = row[CAT_MATCH_TYPE].toInt();
    
    return static_cast<MATCH_TYPE>(typeInt);
  }

//----------------------------------------------------------------------------

  SEX Category::getSex() const
  {
    int sexInt = row[CAT_SEX].toInt();
    
    return static_cast<SEX>(sexInt);
  }

//----------------------------------------------------------------------------

  ERR Category::setMatchSystem(MATCH_SYSTEM s)
  {
    return Tournament::getCatMngr()->setMatchSystem(*this, s);
  }

//----------------------------------------------------------------------------

  ERR Category::setMatchType(MATCH_TYPE t)
  {
    return Tournament::getCatMngr()->setMatchType(*this, t);
  }

//----------------------------------------------------------------------------

  ERR Category::setSex(SEX s)
  {
    return Tournament::getCatMngr()->setSex(*this, s);
  }

//----------------------------------------------------------------------------

  bool Category::canAddPlayers() const
  {
    // For now, you can only add players to a category
    // when it's still in configuration mode
    return (getState() == STAT_CAT_CONFIG);
    
    // TODO: make more sophisticated tests depending e. g. on
    // the match system. For instance, if we have random
    // matches, players should be addable after every round
  }

//----------------------------------------------------------------------------

  CAT_ADD_STATE Category::getAddState(const SEX s) const
  {
    if (!(canAddPlayers()))
    {
      return CAT_CLOSED;
    }
    
    // a "mixed" category can take any player
    if (getMatchType() == MIXED)
    {
      return CAN_JOIN;
    }
    
    // ok, so we're either in singles or doubles. if the sex
    // is set to DONT_CARE, then also any player will fit
    if (getSex() == DONT_CARE)
    {
      return CAN_JOIN;
    }
    
    // in all other cases, the category's sex has to
    // match the player's sex
    return (s == getSex()) ? CAN_JOIN : WRONG_SEX;
  }

//----------------------------------------------------------------------------

  CAT_ADD_STATE Category::getAddState(const Player& p) const
  {
    if (hasPlayer(p))
    {
      return ALREADY_MEMBER;
    }
    
    return getAddState(p.getSex());
  }

//----------------------------------------------------------------------------

  ERR Category::addPlayer(const Player& p)
  {
    return Tournament::getCatMngr()->addPlayerToCategory(p, *this);
  }

//----------------------------------------------------------------------------

  bool Category::hasPlayer(const Player& p) const
  {
    QVariantList qvl;
    qvl << P2C_PLAYER_REF << p.getId();
    qvl << P2C_CAT_REF << getId();
    
    return (db->getTab(TAB_P2C).getMatchCountForColumnValue(qvl) > 0);
  }

//----------------------------------------------------------------------------

  bool Category::canRemovePlayer(const Player& p) const
  {
    // For now, you can only delete players from a category
    // when it's still in configuration mode
    if (getState() != STAT_CAT_CONFIG) return false;
    
    // check whether the player is paired with another player
    if (isPaired(p))
    {
      Player partner = getPartner(p);
      if (canSplitPlayers(p, partner) != OK)
      {
	return false;
      }
    }
    
    // TODO: make more sophisticated tests depending e. g. on
    // the match system. For instance, if we have random
    // matches, players should be removable after every round
    //
    // Also, we should be able to remove only players that were not
    // yet involved/scheduled for any matches.
    
    return true;
  }

//----------------------------------------------------------------------------

  ERR Category::removePlayer(const Player& p)
  {
    return Tournament::getCatMngr()->removePlayerFromCategory(p, *this);
  }


//----------------------------------------------------------------------------

  QVariant Category::getParameter(CAT_PARAMETER p) const
  {
    switch (p) {
      
    case ALLOW_DRAW:
      return row[CAT_ACCEPT_DRAW];
      
    case WIN_SCORE:
      return row[CAT_WIN_SCORE];
      
    case DRAW_SCORE:
      return row[CAT_DRAW_SCORE];
    
    case GROUP_CONFIG:
      return row[CAT_GROUP_CONFIG];
    /*  
    case :
      return row[];
      
    case :
      return row[];
      
    case :
      return row[];
    */ 
    default:
      return QVariant();
    }
  }

//----------------------------------------------------------------------------

  bool Category::setParameter(CAT_PARAMETER p, const QVariant& v)
  {
    return Tournament::getCatMngr()->setCatParameter(*this, p, v);
  }

//----------------------------------------------------------------------------

  int Category::getParameter_int(CAT_PARAMETER p) const
  {
    return getParameter(p).toInt();
  }

//----------------------------------------------------------------------------

  bool Category::getParameter_bool(CAT_PARAMETER p) const
  {
    return getParameter(p).toBool();
  }
  
//----------------------------------------------------------------------------

  QString Category::getParameter_string(CAT_PARAMETER p) const
  {
    return getParameter(p).toString();
  }

//----------------------------------------------------------------------------

  QList<PlayerPair> Category::getPlayerPairs() const
  {
    QList<PlayerPair> result;
    PlayerMngr* pmngr = Tournament::getPlayerMngr();
    
    // get all players assigned to this category
    QList<Player> singlePlayers = getAllPlayersInCategory();
    
    // filter out the players that are paired
    DbTab::CachingRowIterator it = db->getTab(TAB_PAIRS).getRowsByColumnValue(PAIRS_CAT_REF, getId());
    while (!(it.isEnd()))
    {
      int id1 = (*it)[PAIRS_PLAYER1_REF].toInt();
      int id2 = (*it)[PAIRS_PLAYER2_REF].toInt();
      Player p1 = pmngr->getPlayer(id1);
      Player p2 = pmngr->getPlayer(id2);
      
      result.append(PlayerPair(p1, p2, (*it).getId()));
      singlePlayers.removeAll(p1);
      singlePlayers.removeAll(p2);
      
      ++it;
    }
    
    // create special entries for un-paired players
    for (int i=0; i < singlePlayers.count(); i++)
    {
      result.append(PlayerPair(singlePlayers.at(i)));
    }
    
    return result;
  }

//----------------------------------------------------------------------------

  QList<Player> Category::getAllPlayersInCategory() const
  {
    QList<Player> result;
    PlayerMngr* pmngr = Tournament::getPlayerMngr();
    
    DbTab::CachingRowIterator it = db->getTab(TAB_P2C).getRowsByColumnValue(P2C_CAT_REF, getId());
    while (!(it.isEnd()))
    {
      result.append(pmngr->getPlayer((*it)[P2C_PLAYER_REF].toInt()));
      ++it;
    }
    
    return result;
  }

//----------------------------------------------------------------------------

  bool Category::isPaired(const Player& p) const
  {
    if (!(hasPlayer(p)))
    {
      return false;
    }
    
    // manually construct a where-clause for an OR-query
    QString whereClause = "(" + PAIRS_PLAYER1_REF + " = ? OR " + PAIRS_PLAYER2_REF + " = ?) ";
    whereClause += "AND (" + PAIRS_CAT_REF + " = ?)";
    QVariantList qvl;
    qvl << p.getId();
    qvl << p.getId();
    qvl << getId();
    
    // see if we have a row that matches the query
    DbTab pairTab = db->getTab(TAB_PAIRS);
    return (pairTab.getMatchCountForWhereClause(whereClause, qvl) != 0);
  }

//----------------------------------------------------------------------------

  ERR Category::canPairPlayers(const Player& p1, const Player& p2) const
  {
    // we can only create pairs while being in config mode
    if (getState() != STAT_CAT_CONFIG)
    {
      return CATEGORY_NOT_CONFIGURALE_ANYMORE;
    }
    
    // in singles, we don't need pairs. The same is true if we're using
    // the match system "random matches with random partners".
    MATCH_TYPE mt = getMatchType();
    if (mt == SINGLES)
    {
      return NO_CATEGORY_FOR_PAIRING;
    }
    // TODO: check for "random" with "random partners"
    
    
    // make sure that both players are actually listed in this category
    if ((!(hasPlayer(p1))) || (!(hasPlayer(p2))))
    {
      return PLAYER_NOT_IN_CATEGORY;
    }
    
    // make sure that both players are not yet paired in this category
    if ((isPaired(p1)) || (isPaired(p2)))
    {
      return PLAYER_ALREADY_PAIRED;
    }
    
    // make sure that the players are not identical
    if (p1 == p2)
    {
      return PLAYERS_IDENTICAL;
    }
    
    // if this is a mixed category, make sure the sex is right
    if ((mt == MIXED) && (getSex() != DONT_CARE))
    {
      if (p1.getSex() == p2.getSex())
      {
	return INVALID_SEX;
      }
    }
    
    // if this is a doubles category, make sure the sex is right
    if ((mt == DOUBLES) && (getSex() != DONT_CARE))
    {
      SEX catSex = getSex();
      if ((p1.getSex() != catSex) || (p2.getSex() != catSex))
      {
	return INVALID_SEX;
      }
    }
    
    return OK;
  }

//----------------------------------------------------------------------------

  ERR Category::canSplitPlayers(const Player& p1, const Player& p2) const
  {
    // we can only split pairs while being in config mode
    if (getState() != STAT_CAT_CONFIG)
    {
      return CATEGORY_NOT_CONFIGURALE_ANYMORE;
    }
    
    // check if the two players are paired for this category
    QVariantList qvl;
    qvl << PAIRS_CAT_REF << getId();
    qvl << PAIRS_PLAYER1_REF << p1.getId();
    qvl << PAIRS_PLAYER2_REF << p2.getId();
    DbTab pairsTab = db->getTab(TAB_PAIRS);
    if (pairsTab.getMatchCountForColumnValue(qvl) != 0)
    {
      return OK;
    }
    
    // swap player 1 and player 2 and make a new query
    qvl.clear();
    qvl << PAIRS_CAT_REF << getId();
    qvl << PAIRS_PLAYER1_REF << p2.getId();
    qvl << PAIRS_PLAYER2_REF << p1.getId();
    if (pairsTab.getMatchCountForColumnValue(qvl) != 0)
    {
      return OK;
    }
    
    return PLAYERS_NOT_A_PAIR;
  }

//----------------------------------------------------------------------------

  Player Category::getPartner(const Player& p) const
  {
    if (!(hasPlayer(p)))
    {
      throw std::invalid_argument("Player not in category");
    }
    
    // manually construct a where-clause for an OR-query
    QString whereClause = "(" + PAIRS_PLAYER1_REF + " = ? OR " + PAIRS_PLAYER2_REF + " = ?) ";
    whereClause += "AND (" + PAIRS_CAT_REF + " = ?)";
    QVariantList qvl;
    qvl << p.getId();
    qvl << p.getId();
    qvl << getId();
    
    // see if we have a row that matches the query
    int partnerId = -1;
    DbTab pairTab = db->getTab(TAB_PAIRS);
    try
    {
      TabRow r = pairTab.getSingleRowByWhereClause(whereClause, qvl);
      partnerId = (r[PAIRS_PLAYER1_REF].toInt() == p.getId()) ? r[PAIRS_PLAYER2_REF].toInt() : r[PAIRS_PLAYER1_REF].toInt();
    } catch (exception e) {
      throw std::invalid_argument("Player doesn't have a partner");
    }
    
    return Tournament::getPlayerMngr()->getPlayer(partnerId);
  }

//----------------------------------------------------------------------------

  bool Category::hasUnpairedPlayers() const
  {
    QList<PlayerPair> pp = getPlayerPairs();
    for (int i=0; i < pp.count(); i++)
    {
      if (!(pp.at(i).hasPlayer2())) return true;
    }
    
    return false;
  }

//----------------------------------------------------------------------------

  Category* Category::convertToSpecializedObject()
  {
    // return an instance of a suitable, specialized category-child
    MATCH_SYSTEM sys = getMatchSystem();

    if (sys == GROUPS_WITH_KO) {
      return new RoundRobinCategory(db, row);
    }

    // THIS IS JUST A HOT FIX UNTIL WE HAVE
    // SPECIALIZED CLASSED FOR ALL MATCH SYSTEMS!!!
    //
    // Returning an object of the base class instead of the derived class
    // will end up in exceptions and abnormal program termination
    return new Category(db, row);
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