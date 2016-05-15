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

#ifndef MATCHMNGR_H
#define	MATCHMNGR_H

#include <memory>
#include <tuple>

#include <QList>
#include <QString>

#include "TournamentDataDefs.h"
#include "TournamentErrorCodes.h"
#include "DbTab.h"
#include "TournamentDatabaseObjectManager.h"
#include "Category.h"
#include "MatchGroup.h"
#include "Match.h"
#include "Court.h"
#include "Score.h"

using namespace SqliteOverlay;

namespace QTournament
{

  typedef vector<MatchGroup> MatchGroupList;
  
  class MatchMngr : public QObject, TournamentDatabaseObjectManager
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
    MatchList getFinishedMatches() const;
    MatchList getMatchesForMatchGroup(const MatchGroup& grp) const;
    unique_ptr<Match> getMatchForCourt(const Court& court);
    unique_ptr<Match> getMatchForPlayerPairAndRound(const PlayerPair& pp, int round) const;
    unique_ptr<Match> getMatchBySeqNum(int maSeqNum) const;
    unique_ptr<Match> getMatchByMatchNum(int maNum) const;
    unique_ptr<Match> getMatch(int id) const;
    tuple<int, int, int> getMatchStats() const;

    // boolean hasXXXXX functions for MATCHES
    bool hasMatchesInCategory(const Category& cat, int round=-1) const;

    // retrievers / enumerators for MATCH GROUPS
    MatchGroupList getMatchGroupsForCat(const Category& cat, int round=-1) const;
    MatchGroupList getAllMatchGroups() const;
    unique_ptr<MatchGroup> getMatchGroup(const Category& cat, const int round, const int grpNum,  ERR* err);
    unique_ptr<MatchGroup> getMatchGroupBySeqNum(int mgSeqNum);
    MatchGroupList getStagedMatchGroupsOrderedBySequence() const;

    // boolean hasXXXXX functions for MATCH GROUPS
    bool hasMatchGroup(const Category& cat, const int round, const int grpNum, ERR* err=nullptr);

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
    ERR setPlayerPairForMatch(const Match& ma, const PlayerPair& pp, int ppPos) const;
    ERR setSymbolicPlayerForMatch(const Match& fromMatch, const Match& toMatch, bool asWinner, int dstPlayerPosInMatch) const;
    ERR setPlayerToUnused(const Match& ma, int unusedPlayerPos, int winnerRank) const;   // use only if this is the last match for the winner!
    ERR setRankForWinnerOrLoser(const Match& ma, bool isWinner, int rank) const;
    ERR getNextViableMatchCourtPair(int* matchId, int* courtId, bool includeManualCourts=false) const;
    ERR canAssignMatchToCourt(const Match& ma, const Court &court) const;
    ERR assignMatchToCourt(const Match& ma, const Court& court) const;
    unique_ptr<Court> autoAssignMatchToNextAvailCourt(const Match& ma, ERR* err, bool includeManualCourts=false) const;
    ERR setMatchScoreAndFinalizeMatch(const Match& ma, const MatchScore& score, bool isWalkover=false) const;
    ERR setNextMatchForWinner(const Match& fromMatch, const Match& toMatch, int playerNum) const;
    ERR setNextMatchForLoser(const Match& fromMatch, const Match& toMatch, int playerNum) const;
    ERR walkover(const Match& ma, int playerNum) const;
    ERR undoMatchCall(const Match& ma) const;

    // configuration of MATCH GROUPS
    ERR closeMatchGroup(const MatchGroup& grp);

    // referee/umpire handling
    ERR setRefereeMode(const Match& ma, REFEREE_MODE newMode) const;
    ERR assignReferee(const Match& ma, const Player& p, REFEREE_ACTION refAction) const;
    ERR removeReferee(const Match& ma) const;


  private:
    DbTab* groupTab;
    void updateAllMatchGroupStates(const Category& cat) const;
    void updateMatchStatus(const Match& ma) const;
    bool hasUnfinishedMandatoryPredecessor(const Match& ma) const;
    void resolveSymbolicNamesAfterFinishedMatch(const Match& ma) const;
    static constexpr int SYMBOLIC_ID_FOR_UNUSED_PLAYER_PAIR_IN_MATCH = 999999;
    
  signals:

  public slots:
    void onPlayerStatusChanged(int playerId, int playerSeqNum, OBJ_STATE fromState, OBJ_STATE toState);
} ;

}
#endif	/* MATCHMNGR_H */

