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

#ifndef PLAYER_H
#define	PLAYER_H

#include <memory>

#include <QList>

#include "GenericDatabaseObject.h"
#include "GenericObjectManager.h"
#include "TournamentDB.h"
#include "TabRow.h"
#include "TournamentErrorCodes.h"
#include "Team.h"
//#include "Category.h"


namespace QTournament
{
  class Category;

  class Player : public GenericDatabaseObject
  {
    friend class PlayerMngr;
    friend class TeamMngr;
    friend class GenericObjectManager;
    
  public:
    QString getDisplayName(int maxLen = 0) const;
    QString getDisplayName_FirstNameFirst() const;
    QString getFirstName() const;
    QString getLastName() const;
    ERR rename(const QString& newFirst, const QString& newLast);
    SEX getSex() const;
    Team getTeam() const;
    QList<Category> getAssignedCategories() const;

  private:
    Player (TournamentDB* db, int rowId);
    Player (TournamentDB* db, dbOverlay::TabRow row);
  };

  typedef QList<Player> PlayerList;
  typedef unique_ptr<Player> upPlayer;

}
#endif	/* TEAM_H */

