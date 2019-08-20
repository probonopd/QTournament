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
    : TournamentDatabaseObjectManager(_db, TabTeam)
  {
  }

//----------------------------------------------------------------------------

  ERR TeamMngr::createNewTeam(const QString& tm)
  {
    auto cfg = SqliteOverlay::KeyValueTab{db.get(), TabCfg};

    if (!(cfg.getBool(CfgKey_UseTeams)))
    {
      return ERR::NotUsingTeams;
    }
    
    QString teamName = tm.trimmed();
    
    if (teamName.isEmpty())
    {
      return ERR::InvalidName;
    }
    
    if (teamName.length() > MaxNameLen)
    {
      return ERR::InvalidName;
    }
    
    if (hasTeam(teamName))
    {
      return ERR::NameExists;
    }
    
    // create a new table row
    ColumnValueClause cvc;
    cvc.addCol(GenericNameFieldName, teamName.toUtf8().constData());
    
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
    return (tab.getMatchCountForColumnValue(GenericNameFieldName, teamName.toUtf8().constData()) > 0);
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
    
    TabRow r = tab.getSingleRowByColumnValue(GenericNameFieldName, name.toUtf8().constData());
    
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
    if ((newName.isEmpty()) || (newName.length() > MaxNameLen))
    {
      return ERR::InvalidName;
    }
    
    // make sure the new name doesn't exist yet
    if (hasTeam(newName))
    {
      return ERR::NameExists;
    }
    
    t.row.update(GenericNameFieldName, newName.toUtf8().constData());
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
      TabRow r = tab.getSingleRowByColumnValue(GenericSeqnumFieldName, seqNum);
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
    return getSingleObjectByColumnValue<Team>(GenericSeqnumFieldName, seqNum);
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
    auto cfg = SqliteOverlay::KeyValueTab{db.get(), TabCfg};

    if (!(cfg.getBool(CfgKey_UseTeams)))
    {
      return ERR::NotUsingTeams;
    }
    
    Team oldTeam = p.getTeam();
    
    if (oldTeam.getId() == newTeam.getId())
    {
      return ERR::OK;  // no database access necessary
    }
    
    p.row.update(PL_TeamRef, newTeam.getId());
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
    DbTab playerTab = DbTab{db.get(), TabPlayer, false};
    return getObjectsByColumnValue<Player>(playerTab, PL_TeamRef, t.getId());
  }

  //----------------------------------------------------------------------------

  std::string TeamMngr::getSyncString(const std::vector<int>& rows) const
  {
    std::vector<Sloppy::estring> cols = {"id", GenericNameFieldName};

    return db.get().getSyncStringForTable(TabTeam, cols, rows);
  }

//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


}
