/* 
 * File:   MatchMngr.h
 * Author: volker
 *
 * Created on August 22, 2014, 7:32 PM
 */

#ifndef MATCHMNGR_H
#define	MATCHMNGR_H

#include "TournamentDataDefs.h"
#include "TournamentErrorCodes.h"
#include "DbTab.h"
#include "GenericObjectManager.h"
#include "Category.h"
#include "MatchGroup.h"

#include <QList>
#include <QString>

using namespace dbOverlay;

namespace QTournament
{

  typedef QList<MatchGroup> MatchGroupList;
  
  class MatchMngr : public QObject, GenericObjectManager
  {
    Q_OBJECT
  public:
    MatchMngr(TournamentDB* _db);
    ERR createMatchGroup(const Category& cat, const int round, const int grpNum, MatchGroup** newGroup);
    MatchGroupList getMatchGroupsForCat(const Category& cat, int round=-1);
    MatchGroupList getAllMatchGroups();
    ERR getMatchGroup(const Category& cat, const int round, const int grpNum, MatchGroup** grp);
    bool hasMatchGroup(const Category& cat, const int round, const int grpNum, ERR* err=0);
    
  private:
    DbTab matchTab;
    DbTab groupTab;
    
} ;

}
#endif	/* MATCHMNGR_H */

