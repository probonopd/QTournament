/*
 *    This is QTournament, a badminton tournament management program.
 *    Copyright (C) 2014 - 2016  Volker Knollmann
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

#include "Player.h"
#include "Team.h"
#include "TournamentDataDefs.h"
#include "TournamentDB.h"
#include "TournamentErrorCodes.h"
#include "PlayerMngr.h"
#include "CatMngr.h"
#include "TeamMngr.h"

namespace QTournament
{

  Player::Player(TournamentDB* db, int rowId)
  :TournamentDatabaseObject(db, TAB_PLAYER, rowId)
  {
  }

//----------------------------------------------------------------------------

  Player::Player(TournamentDB* db, SqliteOverlay::TabRow row)
  :TournamentDatabaseObject(db, row)
  {
  }

//----------------------------------------------------------------------------

  QString Player::getDisplayName(int maxLen) const
  {
    QString first = QString::fromUtf8(row[PL_FNAME].data());
    QString last = QString::fromUtf8(row[PL_LNAME].data());
    
    QString fullName = last + ", " + first;
    
    if (maxLen < 1)   // no length restriction
    {
      return fullName;
    }
    
    if (maxLen < 6)  // invalid length restriction
    {
      throw std::invalid_argument("Can't generate display name of less than six characters");
    }
    
    if (fullName.length() <= maxLen)   // sufficient length restriction
    {
      return fullName;
    }
    
    // okay, we have to shorten the name somehow. Preferably, we keep as much
    // of the last name as possible and shorten the first name
    while (fullName.length() > maxLen)
    {
      if (first.length() > 1)
      {
        first = first.left(first.length() - 1);
      } else {
        last = last.left(last.length() - 1);
      }
      
      fullName = last + ", " + first;
    }
    
    return fullName;
    
  }

//----------------------------------------------------------------------------

  QString Player::getDisplayName_FirstNameFirst() const
  {
    QString first = QString::fromUtf8(row[PL_FNAME].data());
    QString last = QString::fromUtf8(row[PL_LNAME].data());

    return first + " " + last;
  }

//----------------------------------------------------------------------------

  ERR Player::rename(const QString& newFirst, const QString& newLast)
  {
    PlayerMngr pm{db};
    return pm.renamePlayer(*this, newFirst, newLast);
  }

//----------------------------------------------------------------------------

  QString Player::getFirstName() const
  {
    return QString::fromUtf8(row[PL_FNAME].data());
  }

//----------------------------------------------------------------------------

  QString Player::getLastName() const
  {
    return QString::fromUtf8(row[PL_LNAME].data());
  }

//----------------------------------------------------------------------------

  SEX Player::getSex() const
  {
    int sexInt = row.getInt(PL_SEX);
    return static_cast<SEX>(sexInt);
  }

//----------------------------------------------------------------------------

  Team Player::getTeam() const
  {
    auto teamRef = row.getInt2(PL_TEAM_REF);
    
    // if we don't use teams, throw an exception
    if (teamRef->isNull())
    {
      throw std::runtime_error("Query for team of a player occurred; however, we're not using teams in this tournament!");
    }
    
    TeamMngr tm{db};
    return tm.getTeamById(teamRef->get());
  }

//----------------------------------------------------------------------------

  vector<Category> Player::getAssignedCategories() const
  {
    CategoryList result;
    auto it = db->getTab(TAB_P2C)->getRowsByColumnValue(P2C_PLAYER_REF, getId());
    
    CatMngr cmngr{db};
    while (!(it.isEnd()))
    {
      int catId = (*it).getInt(P2C_CAT_REF);
      result.push_back(cmngr.getCategoryById(catId));
      ++it;
    }
    
    return result;
  }

//----------------------------------------------------------------------------

  int Player::getRefereeCount() const
  {
    return row.getInt(PL_REFEREE_COUNT);
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
