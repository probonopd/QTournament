/*
 *    This is QTournament, a badminton tournament management program.
 *    Copyright (C) 2014 - 2017  Volker Knollmann
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

#include <stdexcept>

#include <SqliteOverlay/KeyValueTab.h>

#include "TeamMngr.h"
#include "TournamentErrorCodes.h"
#include "TournamentDataDefs.h"
#include "HelperFunc.h"
#include "Player.h"
#include "CentralSignalEmitter.h"

using namespace SqliteOverlay;

namespace QTournament
{

  TeamMngr::TeamMngr(const TournamentDB& _db)
    : TournamentDatabaseObjectManager(_db, TAB_TEAM)
  {
  }

//----------------------------------------------------------------------------

  ERR TeamMngr::createNewTeam(const QString& tm)
  {
    auto cfg = SqliteOverlay::KeyValueTab{db.get(), TAB_CFG};

    if (!(cfg.getBool(CFG_KEY_USE_TEAMS)))
    {
      return ERR::NOT_USING_TEAMS;
    }
    
    QString teamName = tm.trimmed();
    
    if (teamName.isEmpty())
    {
      return ERR::INVALID_NAME;
    }
    
    if (teamName.length() > MAX_NAME_LEN)
    {
      return ERR::INVALID_NAME;
    }
    
    if (hasTeam(teamName))
    {
      return ERR::NAME_EXISTS;
    }
    
    // create a new table row
    ColumnValueClause cvc;
    cvc.addCol(GENERIC_NAME_FIELD_NAME, teamName.toUtf8().constData());
    
    CentralSignalEmitter* cse = CentralSignalEmitter::getInstance();
    cse->beginCreateTeam();
    tab.insertRow(cvc);
    fixSeqNumberAfterInsert();
    cse->endCreateTeam(tab.length() - 1);  // the new sequence number is always the greatest
    
    return ERR::OK;
  }

//----------------------------------------------------------------------------

  bool TeamMngr::hasTeam(const QString& teamName)
  {
    return (tab.getMatchCountForColumnValue(GENERIC_NAME_FIELD_NAME, teamName.toUtf8().constData()) > 0);
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
    
    TabRow r = tab.getSingleRowByColumnValue(GENERIC_NAME_FIELD_NAME, name.toUtf8().constData());
    
    return Team{db.get(), r};
  }

//----------------------------------------------------------------------------

  /**
   * Returns a list of all teams
   *
   * @Return QList holding all Teams
   */
  std::vector<Team> TeamMngr::getAllTeams()
  {
    return getAllObjects<Team>();
  }

//----------------------------------------------------------------------------

  ERR TeamMngr::renameTeam(const Team& t, const QString& nn)
  {
    QString newName = nn.trimmed();
    
    // Ensure the new name is valid
    if ((newName.isEmpty()) || (newName.length() > MAX_NAME_LEN))
    {
      return ERR::INVALID_NAME;
    }
    
    // make sure the new name doesn't exist yet
    if (hasTeam(newName))
    {
      return ERR::NAME_EXISTS;
    }
    
    t.row.update(GENERIC_NAME_FIELD_NAME, newName.toUtf8().constData());
    CentralSignalEmitter::getInstance()->teamRenamed(t.getSeqNum());
    
    return ERR::OK;
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
      TabRow r = tab.getSingleRowByColumnValue(GENERIC_SEQNUM_FIELD_NAME, seqNum);
      return Team{db, r};
    }
    catch (SqliteOverlay::NoDataException&)
    {
     throw std::invalid_argument("The team with sequence number " + std::to_string(seqNum) + " does not exist");
    }
  }

  //----------------------------------------------------------------------------

  std::optional<Team> TeamMngr::getTeamBySeqNum2(int seqNum)
  {
    return getSingleObjectByColumnValue<Team>(GENERIC_SEQNUM_FIELD_NAME, seqNum);
  }

//----------------------------------------------------------------------------

  Team TeamMngr::getTeamById(int id)
  {
    try {
      TabRow r = tab.operator [](id);
      return Team(db, r);
    }
    catch (SqliteOverlay::NoDataException&)
    {
     throw std::invalid_argument("The team with ID " + std::to_string(id) + " does not exist");
    }
  }

//----------------------------------------------------------------------------

  ERR TeamMngr::changeTeamAssigment(const Player& p, const Team& newTeam)
  {
    auto cfg = SqliteOverlay::KeyValueTab{db.get(), TAB_CFG};

    if (!(cfg.getBool(CFG_KEY_USE_TEAMS)))
    {
      return ERR::NOT_USING_TEAMS;
    }
    
    Team oldTeam = p.getTeam();
    
    if (oldTeam.getId() == newTeam.getId())
    {
      return ERR::OK;  // no database access necessary
    }
    
    p.row.update(PL_TEAM_REF, newTeam.getId());
    CentralSignalEmitter::getInstance()->teamAssignmentChanged(p, oldTeam, newTeam);
    
    return ERR::OK;
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
    DbTab playerTab = DbTab{db.get(), TAB_PLAYER, false};
    return getObjectsByColumnValue<Player>(playerTab, PL_TEAM_REF, t.getId());
  }

  //----------------------------------------------------------------------------

  std::string TeamMngr::getSyncString(const std::vector<int>& rows) const
  {
    std::vector<Sloppy::estring> cols = {"id", GENERIC_NAME_FIELD_NAME};

    return db.get().getSyncStringForTable(TAB_TEAM, cols, rows);
  }

//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


}
