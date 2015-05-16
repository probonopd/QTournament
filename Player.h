/* 
 * File:   Team.h
 * Author: volker
 *
 * Created on March 2, 2014, 6:13 PM
 */

#ifndef PLAYER_H
#define	PLAYER_H

#include "GenericDatabaseObject.h"
#include "GenericObjectManager.h"
#include "TournamentDB.h"
#include "TabRow.h"
#include "TournamentErrorCodes.h"
#include "Team.h"
//#include "Category.h"

#include <QList>

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

}
#endif	/* TEAM_H */

