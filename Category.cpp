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
    QString newName = nn.trimmed();
    
    // Ensure the new name is valid
    if ((newName.isEmpty()) || (newName.length() > MAX_NAME_LEN))
    {
      return INVALID_NAME;
    }
    
    // make sure the new name doesn't exist yet
    CatMngr* cm = Tournament::getCatMngr();
    if (cm->hasCategory(newName))
    {
      return NAME_EXISTS;
    }
    
    row.update(GENERIC_NAME_FIELD_NAME, newName);
    
    return OK;
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
    return (getState() == STAT_CAT_CONFIG);
    
    // TODO: make more sophisticated tests depending e. g. on
    // the match system. For instance, if we have random
    // matches, players should be removable after every round
    //
    // Also, we should be able to remove players that we're not
    // yet involved/scheduled for any matches.
  }

//----------------------------------------------------------------------------

  ERR Category::removePlayer(const Player& p)
  {
    return Tournament::getCatMngr()->removePlayerFromCategory(p, *this);
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

}