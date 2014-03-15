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
    
    if (!(c.isPlayerSuitable(p)))
    {
      return PLAYER_NOT_SUITABLE;
    }
    
    if (c.hasPlayer(p))
    {
      return PLAYER_ALREADY_IN_CATEGORY;
    }
    
    // TODO: check that player is not permanently disabled
    
    // actually add the player
    QVariantList qvl;
    qvl << P2C_CAT_REF << c.getId();
    qvl << P2C_PLAYER_REF << p.getId();
    db->getTab(TAB_P2C).insertRow(qvl);
    
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
    // TODO: split player pairs, if necessary
    
    // actually delete the assignment
    QVariantList qvl;
    qvl << P2C_CAT_REF << c.getId();
    qvl << P2C_PLAYER_REF << p.getId();
    db->getTab(TAB_P2C).deleteRowsByColumnValue(qvl);
    
    return OK;
  }

//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


}