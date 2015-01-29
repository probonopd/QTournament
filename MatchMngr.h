/* 
 * File:   MatchMngr.h
 * Author: volker
 *
 * Created on August 22, 2014, 7:32 PM
 */

#ifndef MATCHMNGR_H
#define	MATCHMNGR_H

#include <memory>
#include "TournamentDataDefs.h"
#include "TournamentErrorCodes.h"
#include "DbTab.h"
#include "GenericObjectManager.h"
#include "Category.h"
#include "MatchGroup.h"
#include "Match.h"

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
    unique_ptr<MatchGroup> createMatchGroup(const Category& cat, const int round, const int grpNum, ERR* err);
    MatchGroupList getMatchGroupsForCat(const Category& cat, int round=-1) const;
    MatchGroupList getAllMatchGroups();
    unique_ptr<MatchGroup> getMatchGroup(const Category& cat, const int round, const int grpNum,  ERR* err);
    bool hasMatchGroup(const Category& cat, const int round, const int grpNum, ERR* err=nullptr);
    unique_ptr<MatchGroup> getMatchGroupBySeqNum(int mgSeqNum);

    unique_ptr<Match> createMatch(const MatchGroup& grp, ERR* err);
    ERR canAssignPlayerPairToMatch(const Match& ma, const PlayerPair& pp) const;
    ERR setPlayerPairsForMatch(const Match& ma, const PlayerPair& pp1, const PlayerPair& pp2);
    ERR closeMatchGroup(const MatchGroup& grp);

    ERR stageMatchGroup(const MatchGroup& grp);
    int getMaxStageSeqNum() const;
    ERR canUnstageMatchGroup(const MatchGroup& grp);
    ERR canStageMatchGroup(const MatchGroup& grp);
    ERR unstageMatchGroup(const MatchGroup& grp);

  private:
    DbTab matchTab;
    DbTab groupTab;
    void updateAllMatchGroupStates(const Category& cat) const;
    
  signals:
    void beginCreateMatchGroup ();
    void endCreateMatchGroup (int newMatchGroupSeqNum);
} ;

}
#endif	/* MATCHMNGR_H */

