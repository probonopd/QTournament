/* 
 * File:   RoundRobinCategory.cpp
 * Author: volker
 * 
 * Created on August 25, 2014, 8:34 PM
 */

#include "RoundRobinCategory.h"
#include "KO_Config.h"

#include <QDebug>

using namespace dbOverlay;

namespace QTournament
{

  RoundRobinCategory::RoundRobinCategory(TournamentDB* db, int rowId)
  : Category(db, rowId)
  {
    qDebug() << "!!  RR  !!";
  }

//----------------------------------------------------------------------------

  RoundRobinCategory::RoundRobinCategory(TournamentDB* db, dbOverlay::TabRow row)
  : Category(db, row)
  {
    qDebug() << "!!  RR  !!";
  }

//----------------------------------------------------------------------------

  ERR RoundRobinCategory::canFreezeConfig()
  {
    if (getState() != STAT_CAT_CONFIG)
    {
      return CONFIG_ALREADY_FROZEN;
    }
    
    // make sure there no unpaired players in singles or doubles
    if ((getMatchType() != SINGLES) && (hasUnpairedPlayers()))
    {
      return UNPAIRED_PLAYERS;
    }
    
    // make sure we have a valid group configuration
    PlayerPairList pp = getPlayerPairs();
    KO_Config cfg = KO_Config(getParameter_string(GROUP_CONFIG));
    if (!(cfg.isValid(pp.count())))
    {
      return INVALID_KO_CONFIG;
    }
    
    return OK;
  }

//----------------------------------------------------------------------------

  bool RoundRobinCategory::needsInitialRanking()
  {
    return false;
  }

//----------------------------------------------------------------------------

  bool RoundRobinCategory::needsGroupInitialization()
  {
    return true;
  }

//----------------------------------------------------------------------------

  ERR RoundRobinCategory::prepareFirstRound(QList<PlayerPairList> grpCfg, PlayerPairList seed)
  {
    // let's see if we can apply the group configuration
    ERR e = applyGroupAssignment(grpCfg);
    if (e != OK) return e;


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


}
