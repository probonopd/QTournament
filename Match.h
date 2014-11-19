/* 
 * File:   Team.h
 * Author: volker
 *
 * Created on March 2, 2014, 6:13 PM
 */

#ifndef MATCH_H
#define	MATCH_H

#include "GenericDatabaseObject.h"
#include "TournamentDB.h"
#include "TabRow.h"
#include "TournamentErrorCodes.h"
#include "Category.h"
//#include "MatchGroup.h"

#include <QList>

namespace QTournament
{
  class MatchGroup;

  class Match : public GenericDatabaseObject
  {
    friend class MatchMngr;
    friend class MatchGroup;
    
  public:
    Category getCategory() const;
    MatchGroup getMatchGroup() const;
    bool hasPlayerPair1();
    bool hasPlayerPair2();
    bool hasBothPlayerPairs();
    PlayerPair getPlayerPair1();
    PlayerPair getPlayerPair2();

  private:
    Match (TournamentDB* db, int rowId);
    Match (TournamentDB* db, dbOverlay::TabRow row);

  };

}
#endif	/* MATCH_H */

