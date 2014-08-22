/* 
 * File:   MatchGroup.h
 * Author: volker
 *
 * Created on August 22, 2014, 7:58 PM
 */

#ifndef MATCHGROUP_H
#define	MATCHGROUP_H

#include "GenericDatabaseObject.h"
#include "TournamentDB.h"
#include "TabRow.h"
#include "TournamentErrorCodes.h"
#include "Category.h"

#include <QList>

namespace QTournament
{

  class MatchGroup : public GenericDatabaseObject
  {
    friend class MatchMngr;
    
  public:
    Category getCategory();
    int getGroupNumber();
    int getRound();

  private:
    MatchGroup(TournamentDB* db, int rowId);
    MatchGroup(TournamentDB* db, dbOverlay::TabRow row);

  } ;

}
#endif	/* MATCHGROUP_H */

