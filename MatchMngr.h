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
#include "Court.h"
#include "Score.h"

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
    // ctor
    MatchMngr(TournamentDB* _db);

    // creators
    unique_ptr<MatchGroup> createMatchGroup(const Category& cat, const int round, const int grpNum, ERR* err);
    unique_ptr<Match> createMatch(const MatchGroup& grp, ERR* err);

    // retrievers / enumerators for MATCHES
    MatchList getCurrentlyRunningMatches() const;
    MatchList getMatchesForMatchGroup(const MatchGroup& grp) const;
    unique_ptr<Match> getMatchForCourt(const Court& court);

    // boolean hasXXXXX functions for MATCHES
    bool hasMatchesInCategory(const Category& cat, int round=-1) const;

    // retrievers / enumerators for MATCH GROUPS
    MatchGroupList getMatchGroupsForCat(const Category& cat, int round=-1) const;
    MatchGroupList getAllMatchGroups();
    unique_ptr<MatchGroup> getMatchGroup(const Category& cat, const int round, const int grpNum,  ERR* err);
    unique_ptr<MatchGroup> getMatchGroupBySeqNum(int mgSeqNum);
    MatchGroupList getStagedMatchGroupsOrderedBySequence() const;

    // boolean hasXXXXX functions for MATCH GROUPS
    bool hasMatchGroup(const Category& cat, const int round, const int grpNum, ERR* err=nullptr);
    unique_ptr<Match> getMatchBySeqNum(int maSeqNum) const;
    unique_ptr<Match> getMatch(int id) const;

    // staging of match groups
    ERR stageMatchGroup(const MatchGroup& grp);
    int getMaxStageSeqNum() const;
    ERR canUnstageMatchGroup(const MatchGroup& grp);
    ERR canStageMatchGroup(const MatchGroup& grp);
    ERR unstageMatchGroup(const MatchGroup& grp);

    // scheduling of match groups
    int getMaxMatchNum() const;
    void scheduleAllStagedMatchGroups() const;
    int getHighestUsedRoundNumberInCategory(const Category& cat) const;

    // starting / finishing matches
    ERR canAssignPlayerPairToMatch(const Match& ma, const PlayerPair& pp) const;
    ERR setPlayerPairsForMatch(const Match& ma, const PlayerPair& pp1, const PlayerPair& pp2);
    ERR getNextViableMatchCourtPair(int* matchId, int* courtId, bool includeManualCourts=false) const;
    ERR canAssignMatchToCourt(const Match& ma, const Court &court) const;
    ERR assignMatchToCourt(const Match& ma, const Court& court) const;
    unique_ptr<Court> autoAssignMatchToNextAvailCourt(const Match& ma, ERR* err, bool includeManualCourts=false) const;
    ERR setMatchScoreAndFinalizeMatch(const Match& ma, const MatchScore& score);

    // configuration of MATCH GROUPS
    ERR closeMatchGroup(const MatchGroup& grp);


  private:
    DbTab matchTab;
    DbTab groupTab;
    void updateAllMatchGroupStates(const Category& cat) const;
    void updateMatchStatus(const Match& ma) const;
    bool hasUnfinishedMandatoryPredecessor(const Match& ma) const;
    
  signals:
    void beginCreateMatchGroup ();
    void endCreateMatchGroup (int newMatchGroupSeqNum);
    void beginCreateMatch();
    void endCreateMatch(int newMatchSeqNum);
    void matchStatusChanged(int matchId, int matchSeqNum, OBJ_STATE fromState, OBJ_STATE toState) const;
    void matchGroupStatusChanged(int matchGroupId, int matchGroupSeqNum, OBJ_STATE fromState, OBJ_STATE toState) const;
    void matchResultUpdated(int matchId, int matchSeqNum) const;

  public slots:
    void onPlayerStatusChanged(int playerId, int playerSeqNum, OBJ_STATE fromState, OBJ_STATE toState);
} ;

}
#endif	/* MATCHMNGR_H */

