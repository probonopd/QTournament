/* 
 * File:   TeamMngr.cpp
 * Author: nyoknvk1
 * 
 * Created on 18. Februar 2014, 14:04
 */

#include "CatMngr.h"
#include "TournamentErrorCodes.h"
#include "TournamentDataDefs.h"
#include <stdexcept>
#include <qt/QtCore/qdebug.h>
#include <qt/QtCore/qjsonarray.h>
#include "HelperFunc.h"
#include "Category.h"
#include "Player.h"
#include "Tournament.h"

using namespace dbOverlay;

namespace QTournament
{

  CatMngr::CatMngr(TournamentDB* _db)
  : GenericObjectManager(_db), catTab((*db)[TAB_CATEGORY])
  {
  }

//----------------------------------------------------------------------------

  ERR CatMngr::createNewCategory(const QString& cn)
  {
    QString catName = cn.trimmed();
    
    if (catName.isEmpty())
    {
      return INVALID_NAME;
    }
    
    if (catName.length() > MAX_NAME_LEN)
    {
      return INVALID_NAME;
    }
    
    if (hasCategory(catName))
    {
      return NAME_EXISTS;
    }
    
    // create a new table row and set some arbitrary default data
    QVariantList qvl;
    qvl << GENERIC_NAME_FIELD_NAME << catName;
    qvl << CAT_ACCEPT_DRAW << false;
    qvl << CAT_SYS << static_cast<int>(GROUPS_WITH_KO);
    qvl << CAT_MATCH_TYPE << static_cast<int>(SINGLES);
    qvl << CAT_SEX << static_cast<int>(M);
    qvl << GENERIC_STATE_FIELD_NAME << static_cast<int>(STAT_CAT_CONFIG);
    qvl << CAT_WIN_SCORE << 2;
    qvl << CAT_DRAW_SCORE << 1;
    
    catTab.insertRow(qvl);
    fixSeqNumberAfterInsert(TAB_CATEGORY);
    
    return OK;
  }

//----------------------------------------------------------------------------

  bool CatMngr::hasCategory(const QString& catName)
  {
    return (catTab.getMatchCountForColumnValue(GENERIC_NAME_FIELD_NAME, catName) > 0);
  }

//----------------------------------------------------------------------------

  /**
   * Returns a database object for a category identified by its name
   *
   * Note: the category must exist, otherwise this method throws an exception!
   *
   * @param name is the name of the category to look up
   *
   * @return a Category instance of that team
   */
  Category CatMngr::getCategory(const QString& name)
  {
    if (!(hasCategory(name)))
    {
      throw std::invalid_argument("The category '" + QString2String(name) + "' does not exist");
    }
    
    TabRow r = catTab.getSingleRowByColumnValue(GENERIC_NAME_FIELD_NAME, name);
    
    return Category(db, r);
  }

//----------------------------------------------------------------------------

  /**
   * Returns a list of all teams
   *
   * @Return QList holding all Teams
   */
  QList<Category> CatMngr::getAllCategories()
  {
    QList<Category> result;
    
    DbTab::CachingRowIterator it = catTab.getAllRows();
    while (!(it.isEnd()))
    {
      result << Category(db, *it);
      ++it;
    }
    
    return result;
  }

//----------------------------------------------------------------------------

  ERR CatMngr::setMatchSystem(Category& c, MATCH_SYSTEM s)
  {
    // TODO: implement checks, updates to other tables etc
    int sysInt = static_cast<int>(s);
    c.row.update(CAT_SYS, sysInt);
    
    return OK;
  }

//----------------------------------------------------------------------------

  ERR CatMngr::setMatchType(Category& c, MATCH_TYPE t)
  {
    // TODO: implement checks, updates to other tables etc
    int typeInt = static_cast<int>(t);
    c.row.update(CAT_MATCH_TYPE, typeInt);
    
    return OK;
  }

//----------------------------------------------------------------------------

  ERR CatMngr::setSex(Category& c, SEX s)
  {
    // TODO: implement checks, updates to other tables etc
    int sexInt = static_cast<int>(s);
    c.row.update(CAT_SEX, sexInt);
    
    return OK;
  }

//----------------------------------------------------------------------------

  ERR CatMngr::addPlayerToCategory(const Player& p, const Category& c)
  {
    if (!(c.canAddPlayers()))
    {
      return CATEGORY_CLOSED_FOR_MORE_PLAYERS;
    }
    
    if (c.hasPlayer(p))
    {
      return PLAYER_ALREADY_IN_CATEGORY;
    }
    
    if (c.getAddState(p) != CAN_JOIN)
    {
      return PLAYER_NOT_SUITABLE;
    }
    
    // TODO: check that player is not permanently disabled
    
    // actually add the player
    QVariantList qvl;
    qvl << P2C_CAT_REF << c.getId();
    qvl << P2C_PLAYER_REF << p.getId();
    db->getTab(TAB_P2C).insertRow(qvl);
    
    emit playerAddedToCategory(p, c);
    
    return OK;
  }

//----------------------------------------------------------------------------

  ERR CatMngr::removePlayerFromCategory(const Player& p, const Category& c)
  {
    if (!(c.canRemovePlayer(p)))
    {
      return PLAYER_NOT_REMOVABLE_FROM_CATEGORY;
    }
    
    if (!(c.hasPlayer(p)))
    {
      return PLAYER_NOT_IN_CATEGORY;
    }
    
    if (c.isPaired(p))
    {
      Player partner = c.getPartner(p);
      ERR e = splitPlayers(c, p, partner);
      if (e != OK)
      {
	return e;
      }
    }
    
    // And: unless we play with random partners, the double partner
    // has to be removed from the category as well if we're beyond
    // "Category Configuration" state
    
    // actually delete the assignment
    QVariantList qvl;
    qvl << P2C_CAT_REF << c.getId();
    qvl << P2C_PLAYER_REF << p.getId();
    db->getTab(TAB_P2C).deleteRowsByColumnValue(qvl);
    
    emit playerRemovedFromCategory(p, c);
    
    return OK;
  }

//----------------------------------------------------------------------------

  Category CatMngr::getCategoryById(int id)
  {
    try {
      TabRow r = catTab[id];
      return Category(db, r);
    }
    catch (std::exception e)
    {
     throw std::invalid_argument("The category with ID " + QString2String(QString::number(id)) + " does not exist");
    }
  }

//----------------------------------------------------------------------------

  Category CatMngr::getCategoryBySeqNum(int seqNum)
  {
    try {
      TabRow r = catTab.getSingleRowByColumnValue(GENERIC_SEQNUM_FIELD_NAME, seqNum);
      return Category(db, r);
    }
    catch (std::exception e)
    {
     throw std::invalid_argument("The category with sequence number " + QString2String(QString::number(seqNum)) + " does not exist");
    }
  }

//----------------------------------------------------------------------------

  QHash<Category, CAT_ADD_STATE> CatMngr::getAllCategoryAddStates(SEX s)
  {
    QList<Category> allCat = getAllCategories();
    QHash<Category, CAT_ADD_STATE> result;
    
    QList<Category>::const_iterator it;
    for (it = allCat.begin(); it != allCat.constEnd(); ++it)
    {
      result[(*it)] = (*it).getAddState(s);
    }
    
    return result;
  }

//----------------------------------------------------------------------------

  QHash<Category, CAT_ADD_STATE> CatMngr::getAllCategoryAddStates(const Player& p)
  {
    QList<Category> allCat = getAllCategories();
    QHash<Category, CAT_ADD_STATE> result;
    
    QList<Category>::const_iterator it;
    for (it = allCat.begin(); it != allCat.constEnd(); ++it)
    {
      result[(*it)] = (*it).getAddState(p);
    }
    
    return result;
  }

//----------------------------------------------------------------------------

  bool CatMngr::setCatParameter(Category& c, CAT_PARAMETER p, const QVariant& v)
  {
    if (p == ALLOW_DRAW)
    {
      return setCatParam_AllowDraw(c, v);
    }
    if (p == DRAW_SCORE)
    {
      return setCatParam_Score(c, v.toInt(), true);
    }
    if (p == WIN_SCORE)
    {
      return setCatParam_Score(c, v.toInt(), false);
    }
    
    return false;
  }

//----------------------------------------------------------------------------

  bool CatMngr::setCatParam_AllowDraw(Category& c, const QVariant& v)
  {
    if (c.getState() != STAT_CAT_CONFIG)
    {
      return false;
    }
    
    bool allowDraw = v.toBool();
    bool oldState = c.getParameter(ALLOW_DRAW).toBool();

    if (allowDraw == oldState)
    {
      return true; // no change necessary;
    }

    // ensure consistent scoring before accepting draw
    if (allowDraw)
    {
      int winScore = c.getParameter_int(WIN_SCORE);
      int drawScore = c.getParameter_int(DRAW_SCORE);

      if (drawScore < 1)
      {
	drawScore = 1;
	c.row.update(CAT_DRAW_SCORE, 1);
      }
      if (winScore <= drawScore)
      {
	winScore = drawScore + 1;
	c.row.update(CAT_WIN_SCORE, winScore);
      }
    }

    // set the new status
    c.row.update(CAT_ACCEPT_DRAW, allowDraw);
    return true;
  }

//----------------------------------------------------------------------------

  bool CatMngr::setCatParam_Score(Category& c, int newScore, bool isDraw)
  {
    if (c.getState() != STAT_CAT_CONFIG)
    {
      return false;
    }
    
    int winScore = c.getParameter_int(WIN_SCORE);
    int drawScore = c.getParameter_int(DRAW_SCORE);
    bool allowDraw = c.getParameter_bool(ALLOW_DRAW);
    
    // only scores above 0 make sense
    if (newScore < 1)
    {
      return false;
    }
    
    // can't update draw score if draw is not enabled
    if (!allowDraw && isDraw)
    {
      return false;
    }
    
    if (isDraw)
    {
      if (newScore >= winScore)
      {
	return false;
      }
      
      c.row.update(CAT_DRAW_SCORE, newScore);
      return true;
    }

    // if we're here, we're updating the win score
    if (allowDraw && (newScore <= drawScore))
    {
      return false;
    }

    c.row.update(CAT_WIN_SCORE, newScore);
    return true;
  }

//----------------------------------------------------------------------------

  ERR CatMngr::pairPlayers(const Category c, const Player& p1, const Player& p2)
  {
    // all pre-conditions for pairing two players are checked
    // in the category. If this check is positive, we can start
    // right away with creating the pair in the database
    ERR e = c.canPairPlayers(p1, p2);
    if (e != OK)
    {
      return e;
    }
    
    // create the pair
    QVariantList qvl;
    qvl << PAIRS_CAT_REF << c.getId();
    qvl << PAIRS_PLAYER1_REF << p1.getId();
    qvl << PAIRS_PLAYER2_REF << p2.getId();
    db->getTab(TAB_PAIRS).insertRow(qvl);
    
    emit playersPaired(c, p1, p2);
    
    return OK;
  }

//----------------------------------------------------------------------------

  ERR CatMngr::splitPlayers(const Category c, const Player& p1, const Player& p2)
  {
    // all pre-conditions for splitting two players are checked
    // in the category. If this check is positive, we can start
    // right away with deleting the pair from the database
    ERR e = c.canSplitPlayers(p1, p2);
    if (e != OK)
    {
      return e;
    }
    
    // delete all combinations of p1/p2 pairs from the database
    QVariantList qvl;
    qvl << PAIRS_CAT_REF << c.getId();
    qvl << PAIRS_PLAYER1_REF << p1.getId();
    qvl << PAIRS_PLAYER2_REF << p2.getId();
    DbTab pairsTab = db->getTab(TAB_PAIRS);
    pairsTab.deleteRowsByColumnValue(qvl);
    qvl.clear();
    qvl << PAIRS_CAT_REF << c.getId();
    qvl << PAIRS_PLAYER1_REF << p2.getId();
    qvl << PAIRS_PLAYER2_REF << p1.getId();
    pairsTab.deleteRowsByColumnValue(qvl);
    
    emit playersSplit(c, p1, p2);
    
    return OK;
  }


//----------------------------------------------------------------------------

  ERR CatMngr::splitPlayers(const Category c, int pairId)
  {
    DbTab pairsTab = db->getTab(TAB_PAIRS);
    PlayerMngr* pmngr = Tournament::getPlayerMngr();
    try
    {
      TabRow r = pairsTab[pairId];
      Player p1 = pmngr->getPlayer(r[PAIRS_PLAYER1_REF].toInt());
      Player p2 = pmngr->getPlayer(r[PAIRS_PLAYER2_REF].toInt());
      return splitPlayers(c, p1, p2);
    }
    catch (exception e)
    {
      
    }
    return INVALID_ID;
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


}