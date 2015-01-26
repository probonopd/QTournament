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
#include "Match.h"

#include <QList>

namespace QTournament
{

  typedef QList<Match> MatchList;

  class MatchGroup : public GenericDatabaseObject
  {
    friend class MatchMngr;
    friend class Match;
    
  public:
    Category getCategory() const;
    int getGroupNumber() const;
    int getRound() const;
    MatchList getMatches() const;
    int getMatchCount() const;
    int getStageSequenceNumber() const;

  private:
    MatchGroup(TournamentDB* db, int rowId);
    MatchGroup(TournamentDB* db, dbOverlay::TabRow row);

  } ;

}
#endif	/* MATCHGROUP_H */

