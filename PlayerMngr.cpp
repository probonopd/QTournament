/* 
 * File:   TeamMngr.cpp
 * Author: nyoknvk1
 * 
 * Created on 18. Februar 2014, 14:04
 */

#include "PlayerMngr.h"
#include "Player.h"
#include "TournamentErrorCodes.h"
#include "TournamentDataDefs.h"
#include <stdexcept>
#include <qt/QtCore/qdebug.h>
#include "HelperFunc.h"
#include "TeamMngr.h"
#include "Tournament.h"

using namespace dbOverlay;

namespace QTournament
{

  PlayerMngr::PlayerMngr(TournamentDB* _db)
  : GenericObjectManager(_db), playerTab((*db)[TAB_PLAYER])
  {
  }

//----------------------------------------------------------------------------

  ERR PlayerMngr::createNewPlayer(const QString& firstName, const QString& lastName, SEX sex, const QString& teamName)
  {
    QString first = firstName.trimmed();
    QString last = lastName.trimmed();
    
    if (first.isEmpty() || last.isEmpty())
    {
      return INVALID_NAME;
    }
    
    if ((first.length() > MAX_NAME_LEN) || (last.length() > MAX_NAME_LEN))
    {
      return INVALID_NAME;
    }
    
    if (hasPlayer(first, last))
    {
      return NAME_EXISTS;
    }
    
    if (sex == DONT_CARE)
    {
      return INVALID_SEX;
    }
    
    // prepare a new table row
    QVariantList qvl;
    qvl << PL_FNAME << first;
    qvl << PL_LNAME << last;
    qvl << PL_SEX << static_cast<int>(sex);
    qvl << GENERIC_STATE_FIELD_NAME << static_cast<int>(STAT_PL_IDLE);
    
    // set the team reference, if applicable
    if (cfg.getBool(CFG_KEY_USE_TEAMS))
    {
      if (teamName.isEmpty())
      {
        return INVALID_TEAM;
      }
      
      TeamMngr* tm = Tournament::getTeamMngr();
      if (!(tm->hasTeam(teamName)))
      {
        return INVALID_TEAM;
      }
      
      Team t = tm->getTeam(teamName);
      qvl << PL_TEAM_REF << t.getId();
    }
    
    // create the new player row
    playerTab.insertRow(qvl);
    fixSeqNumberAfterInsert(TAB_PLAYER);
    
    return OK;
  }

//----------------------------------------------------------------------------

  bool PlayerMngr::hasPlayer(const QString& firstName, const QString& lastName)
  {
    QVariantList qvl;
    qvl << PL_FNAME << firstName;
    qvl << PL_LNAME << lastName;
    
    return (playerTab.getMatchCountForColumnValue(qvl) > 0);
  }

//----------------------------------------------------------------------------

  /**
   * Returns a database object for a player identified by its name
   *
   * Note: the player must exist, otherwise this method throws an exception!
   *
   * @param firstName is the first name of the player to look up
   * @param lastName is the last name of the player to look up
   *
   * @return a Player instance of that player
   */
  Player PlayerMngr::getPlayer(const QString& firstName, const QString& lastName)
  {
    if (!(hasPlayer(firstName, lastName)))
    {
      throw std::invalid_argument("The player '" + QString2String(firstName + " " + lastName) + "' does not exist");
    }
    
    QVariantList qvl;
    qvl << PL_FNAME << firstName;
    qvl << PL_LNAME << lastName;
    TabRow r = playerTab.getSingleRowByColumnValue(qvl);
    
    return Player(db, r);
  }

//----------------------------------------------------------------------------

  /**
   * Returns a list of all players
   *
   * @Return QList holding all Teams
   */
  QList<Player> PlayerMngr::getAllPlayers()
  {
    QList<Player> result;
    
    DbTab::CachingRowIterator it = playerTab.getAllRows();
    while (!(it.isEnd()))
    {
      result << Player(db, *it);
      ++it;
    }
    
    return result;
  }

//----------------------------------------------------------------------------

  ERR PlayerMngr::renamePlayer(Player& p, const QString& nf, const QString& nl)
  {
    QString newFirst = nf.trimmed();
    QString newLast = nl.trimmed();
    
    // Ensure the new name is valid
    if ((newFirst.isEmpty()) && (newLast.isEmpty()))
    {
      return INVALID_NAME;
    }
    if ((newFirst.length() > MAX_NAME_LEN) || (newLast.length() > MAX_NAME_LEN))
    {
      return INVALID_NAME;
    }
    
    // combine the new name from old and new values
    if (newFirst.isEmpty())
    {
      newFirst = p.getFirstName();
    }
    if (newLast.isEmpty())
    {
      newLast = p.getLastName();
    }
    
    // make sure the new name doesn't exist yet
    if (hasPlayer(newFirst, newLast))
    {
      return NAME_EXISTS;
    }
    
    QVariantList qvl;
    qvl << PL_FNAME << newFirst;
    qvl << PL_LNAME << newLast;
    p.row.update(qvl);
    
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


}