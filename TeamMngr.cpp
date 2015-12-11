/*
 *    This is QTournament, a badminton tournament management program.
 *    Copyright (C) 2014 - 2015  Volker Knollmann
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

#include "TeamMngr.h"
#include "TournamentErrorCodes.h"
#include "TournamentDataDefs.h"
#include <stdexcept>
#include <QtCore/qdebug.h>
#include "HelperFunc.h"
#include "Player.h"

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
    
    emit beginCreateTeam();
    teamTab.insertRow(qvl);
    fixSeqNumberAfterInsert(TAB_TEAM);
    emit endCreateTeam(teamTab.length() - 1);  // the new sequence number is always the greatest
    
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
      throw std::invalid_argument("The team '" + QString2StdString(name) + "' does not exist");
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
    return getAllObjects<Team>(teamTab);
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
    emit teamRenamed(t.getSeqNum());
    
    return OK;
  }

//----------------------------------------------------------------------------

  /**
   * Returns a database object for a team identified by its sequence number
   *
   * Note: the team must exist, otherwise this method throws an exception!
   *
   * @param seqNum is the sequence number of the team to look up
   *
   * @return a Team instance of that team
   */
  Team TeamMngr::getTeamBySeqNum(int seqNum)
  {
    try {
      TabRow r = teamTab.getSingleRowByColumnValue(GENERIC_SEQNUM_FIELD_NAME, seqNum);
      return Team(db, r);
    }
    catch (std::exception e)
    {
     throw std::invalid_argument("The team with sequence number " + to_string(seqNum) + " does not exist");
    }
  }

//----------------------------------------------------------------------------

  Team TeamMngr::getTeamById(int id)
  {
    try {
      TabRow r = teamTab[id];
      return Team(db, r);
    }
    catch (std::exception e)
    {
     throw std::invalid_argument("The team with ID " + to_string(id) + " does not exist");
    }
  }

//----------------------------------------------------------------------------

  ERR TeamMngr::changeTeamAssigment(const Player& p, const Team& newTeam)
  {
    if (!(cfg.getBool(CFG_KEY_USE_TEAMS)))
    {
      return NOT_USING_TEAMS;
    }
    
    Team oldTeam = p.getTeam();
    
    if (oldTeam.getId() == newTeam.getId())
    {
      return OK;  // no database access necessary
    }
    
    TabRow r = p.row;
    r.update(PL_TEAM_REF, newTeam.getId());
    emit teamAssignmentChanged(p, oldTeam, newTeam);
    
    return OK;
  }

//----------------------------------------------------------------------------

  std::function<bool (Team&, Team&)> TeamMngr::getTeamSortFunction_byName()
  {
    return [](Team& t1, Team& t2) {
      return (QString::localeAwareCompare(t1.getName(), t2.getName()) < 0) ? true : false;
    };
  }

//----------------------------------------------------------------------------

  PlayerList TeamMngr::getPlayersForTeam(const Team& t) const
  {
    return getObjectsByColumnValue<Player>((*db)[TAB_PLAYER], PL_TEAM_REF, t.getId());
  }

//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


}
