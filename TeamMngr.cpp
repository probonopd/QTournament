/* 
 * File:   TeamMngr.cpp
 * Author: nyoknvk1
 * 
 * Created on 18. Februar 2014, 14:04
 */

#include "TeamMngr.h"
#include "TournamentErrorCodes.h"
#include "TournamentDataDefs.h"
#include <stdexcept>
#include <qt/QtCore/qdebug.h>
#include "HelperFunc.h"

using namespace dbOverlay;

namespace QTournament
{

  TeamMngr::TeamMngr(TournamentDB* _db)
  : GenericObjectManager(_db), teamTab((*db)[TAB_TEAM])
  {
  }

//----------------------------------------------------------------------------

  ERR TeamMngr::createNewTeam(const QString& tm)
  {
    if (!(cfg.getBool(CFG_KEY_USE_TEAMS)))
    {
      return NOT_USING_TEAMS;
    }
    
    QString teamName = tm.trimmed();
    
    if (teamName.isEmpty())
    {
      return INVALID_NAME;
    }
    
    if (teamName.length() > MAX_NAME_LEN)
    {
      return INVALID_NAME;
    }
    
    if (hasTeam(teamName))
    {
      return NAME_EXISTS;
    }
    
    // create a new table row
    QVariantList qvl;
    qvl << GENERIC_NAME_FIELD_NAME << teamName;
    
    teamTab.insertRow(qvl);
    fixSeqNumberAfterInsert(TAB_TEAM);
    
    return OK;
  }

//----------------------------------------------------------------------------

  bool TeamMngr::hasTeam(const QString& teamName)
  {
    return (teamTab.getMatchCountForColumnValue(GENERIC_NAME_FIELD_NAME, teamName) > 0);
  }

//----------------------------------------------------------------------------

  /**
   * Returns a database object for a team identified by its name
   *
   * Note: the team must exist, otherwise this method throws an exception!
   *
   * @param name is the name of the team to look up
   *
   * @return a Team instance of that team
   */
  Team TeamMngr::getTeam(const QString& name)
  {
    if (!(hasTeam(name)))
    {
      throw std::invalid_argument("The team '" + QString2String(name) + "' does not exist");
    }
    
    TabRow r = teamTab.getSingleRowByColumnValue(GENERIC_NAME_FIELD_NAME, name);
    
    return Team(db, r);
  }

//----------------------------------------------------------------------------

  /**
   * Returns a list of all teams
   *
   * @Return QList holding all Teams
   */
  QList<Team> TeamMngr::getAllTeams()
  {
    QList<Team> result;
    
    DbTab::CachingRowIterator it = teamTab.getAllRows();
    while (!(it.isEnd()))
    {
      result << Team(db, *it);
      ++it;
    }
    
    return result;
  }

//----------------------------------------------------------------------------

  ERR TeamMngr::renameTeam(Team& t, const QString& nn)
  {
    QString newName = nn.trimmed();
    
    // Ensure the new name is valid
    if ((newName.isEmpty()) || (newName.length() > MAX_NAME_LEN))
    {
      return INVALID_NAME;
    }
    
    // make sure the new name doesn't exist yet
    if (hasTeam(newName))
    {
      return NAME_EXISTS;
    }
    
    t.row.update(GENERIC_NAME_FIELD_NAME, newName);
    
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