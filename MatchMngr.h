/*
 *    This is QTournament, a badminton tournament management program.
 *    Copyright (C) 2014 - 2017  Volker Knollmann
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

#include <SqliteOverlay/DbTab.h>

#include "TournamentDataDefs.h"
#include "TournamentErrorCodes.h"
#include "TournamentDatabaseObjectManager.h"
#include "Category.h"
#include "MatchGroup.h"
#include "Match.h"
#include "Court.h"
#include "Score.h"

namespace QTournament
{

  
  class MatchMngr : public QObject, public TournamentDatabaseObjectManager
  {
    Q_OBJECT
  public:
    // ctor
    MatchMngr(const TournamentDB& _db);

    // creators
    std::optional<MatchGroup> createMatchGroup(const Category& cat, const int round, const int grpNum, ERR* err);
    std::optional<Match> createMatch(const MatchGroup& grp, ERR* err);

    // deletion
    void deleteMatchGroupAndMatch(const MatchGroup& mg) const;

    // retrievers / enumerators for MATCHES
    MatchList getCurrentlyRunningMatches() const;
    MatchList getFinishedMatches() const;
    MatchList getMatchesForMatchGroup(const MatchGroup& grp) const;
    std::optional<Match> getMatchForCourt(const Court& court);
    std::optional<Match> getMatchForPlayerPairAndRound(const PlayerPair& pp, int round) const;
    std::optional<Match> getMatchBySeqNum(int maSeqNum) const;
    std::optional<Match> getMatchByMatchNum(int maNum) const;
    std::optional<Match> getMatch(int id) const;
    std::tuple<int, int, int, int> getMatchStats() const;

    // boolean hasXXXXX functions for MATCHES
    bool hasMatchesInCategory(const Category& cat, int round=-1) const;

    // retrievers / enumerators for MATCH GROUPS
    MatchGroupList getMatchGroupsForCat(const Category& cat, int round=-1) const;
    MatchGroupList getAllMatchGroups() const;
    std::optional<MatchGroup> getMatchGroup(const Category& cat, const int round, const int grpNum,  ERR* err);
    std::optional<MatchGroup> getMatchGroupBySeqNum(int mgSeqNum);
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
    std::optional<Court> autoAssignMatchToNextAvailCourt(const Match& ma, ERR* err, bool includeManualCourts=false) const;
    ERR setMatchScoreAndFinalizeMatch(const Match& ma, const MatchScore& score, bool isWalkover=false) const;
    ERR updateMatchScore(const Match& ma, const MatchScore& newScore, bool winnerLoserChangePermitted) const;
    ERR setNextMatchForWinner(const Match& fromMatch, const Match& toMatch, int playerNum) const;
    ERR setNextMatchForLoser(const Match& fromMatch, const Match& toMatch, int playerNum) const;
    ERR walkover(const Match& ma, int winningPlayerNum) const;
    ERR undoMatchCall(const Match& ma) const;
    // configuration of MATCH GROUPS
    ERR closeMatchGroup(const MatchGroup& grp);

    // referee/umpire handling
    ERR setRefereeMode(const Match& ma, REFEREE_MODE newMode) const;
    ERR assignReferee(const Match& ma, const Player& p, REFEREE_ACTION refAction) const;
    ERR removeReferee(const Match& ma) const;

    // swap player between matches if match results are
    // changed after a match has finished
    ERR swapPlayer(const Match& ma, const PlayerPair& ppOld, const PlayerPair& ppNew) const;
    ERR swapPlayers(const Match& ma1, const PlayerPair& ma1PlayerPair,
                    const Match& ma2, const PlayerPair& ma2PlayerPair) const;

    std::string getSyncString(const std::vector<int>& rows) const override;
    std::string getSyncString_MatchGroups(std::vector<int> rows);

  private:
    SqliteOverlay::DbTab groupTab;
    void updateAllMatchGroupStates(const Category& cat) const;
    bool hasUnfinishedMandatoryPredecessor(const Match& ma) const;
    void resolveSymbolicNamesAfterFinishedMatch(const Match& ma) const;
    void updateMatchStatus(const Match& ma) const;
    static constexpr int SYMBOLIC_ID_FOR_UNUSED_PLAYER_PAIR_IN_MATCH = 999999;
    
  signals:

  public slots:
    void onPlayerStatusChanged(int playerId, int playerSeqNum, OBJ_STATE fromState, OBJ_STATE toState);
} ;

}
#endif	/* MATCHMNGR_H */

