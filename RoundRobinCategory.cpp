/* 
 * File:   RoundRobinCategory.cpp
 * Author: volker
 * 
 * Created on August 25, 2014, 8:34 PM
 */

#include "RoundRobinCategory.h"
#include "KO_Config.h"
#include "Tournament.h"

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

  ERR RoundRobinCategory::prepareFirstRound(ProgressQueue *progressNotificationQueue)
  {
    if (getState() != STAT_CAT_IDLE) return WRONG_STATE;

    auto mm = Tournament::getMatchMngr();
    auto pp = Tournament::getPlayerMngr();

    // make sure we have not been called before; to this end, just
    // check that there have no matches been created for us so far
    auto allGrp = mm->getMatchGroupsForCat(*this);
    // do not return an error here, because obviously we have been
    // called successfully before and we only want to avoid
    // double initialization
    if (allGrp.count() != 0) return OK;

    // alright, this is a virgin category. Generate group matches
    // for each group
    KO_Config cfg = KO_Config(getParameter_string(GROUP_CONFIG));
    if (progressNotificationQueue != nullptr)
    {
      progressNotificationQueue->reset(cfg.getNumGroupMatches());
    }
    for (int grpIndex = 0; grpIndex < cfg.getNumGroups(); ++grpIndex)
    {
      PlayerPairList grpMembers = getPlayerPairs(grpIndex+1);
      ERR e = generateGroupMatches(grpMembers, grpIndex+1, 1, progressNotificationQueue);
      if (e != OK) return e;
    }

    return OK;
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
