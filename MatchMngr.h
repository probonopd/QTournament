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
#include <optional>

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

  using MatchOrError = ObjectOrError<Match>;
  using MatchGroupOrError = ObjectOrError<MatchGroup>;

  //----------------------------------------------------------------------------

  struct MatchFinalizationResult
  {
    Error err;
    std::optional<int> completedRound{};   ///< contains the number of the round that was completed by finalizing the match, if any
    bool hasSwissLadderDeadlock{false};   ///< only for Swiss Ladder: did we encounter a deadlock situation after a match has been finished?
  };

  //----------------------------------------------------------------------------

  class MatchMngr : public QObject, public TournamentDatabaseObjectManager
  {
    Q_OBJECT
  public:
    // ctor
    MatchMngr(const TournamentDB& _db);

    // creators
    MatchGroupOrError createMatchGroup(const Category& cat, const int round, const int grpNum);
    MatchOrError createMatch(const MatchGroup& grp);

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
    MatchGroupOrError getMatchGroup(const Category& cat, const int round, const int grpNum);
    std::optional<MatchGroup> getMatchGroupBySeqNum(int mgSeqNum);
    MatchGroupList getStagedMatchGroupsOrderedBySequence() const;

    // boolean hasXXXXX functions for MATCH GROUPS
    //bool hasMatchGroup(const Category& cat, const int round, const int grpNum, Error* err=nullptr);

    // staging of match groups
    Error stageMatchGroup(const MatchGroup& grp);
    int getMaxStageSeqNum() const;
    Error canUnstageMatchGroup(const MatchGroup& grp);
    Error canStageMatchGroup(const MatchGroup& grp);
    Error unstageMatchGroup(const MatchGroup& grp);

    // scheduling of match groups
    int getMaxMatchNum() const;
    void scheduleAllStagedMatchGroups() const;
    int getHighestUsedRoundNumberInCategory(const Category& cat) const;

    // starting / finishing matches
    Error canAssignPlayerPairToMatch(const Match& ma, const PlayerPair& pp) const;
    Error setPlayerPairsForMatch(const Match& ma, const PlayerPair& pp1, const PlayerPair& pp2);
    Error setPlayerPairForMatch(const Match& ma, const PlayerPair& pp, int ppPos) const;
    Error setSymbolicPlayerForMatch(const Match& fromMatch, const Match& toMatch, bool asWinner, int dstPlayerPosInMatch) const;
    Error setPlayerToUnused(const Match& ma, int unusedPlayerPos, int winnerRank) const;   // use only if this is the last match for the winner!
    Error setRankForWinnerOrLoser(const Match& ma, bool isWinner, int rank) const;
    Error getNextViableMatchCourtPair(int* matchId, int* courtId, bool includeManualCourts=false) const;
    Error canAssignMatchToCourt(const Match& ma, const Court &court) const;
    Error assignMatchToCourt(const Match& ma, const Court& court) const;
    std::optional<Court> autoAssignMatchToNextAvailCourt(const Match& ma, Error* err, bool includeManualCourts=false) const;
    MatchFinalizationResult setMatchScoreAndFinalizeMatch(const Match& ma, const MatchScore& score, bool isWalkover=false) const;
    Error updateMatchScore(const Match& ma, const MatchScore& newScore, bool winnerLoserChangePermitted) const;
    Error setNextMatchForWinner(const Match& fromMatch, const Match& toMatch, int playerNum) const;
    Error setNextMatchForLoser(const Match& fromMatch, const Match& toMatch, int playerNum) const;
    MatchFinalizationResult walkover(const Match& ma, int winningPlayerNum) const;
    Error undoMatchCall(const Match& ma) const;
    // configuration of MATCH GROUPS
    Error closeMatchGroup(const MatchGroup& grp);

    // referee/umpire handling
    Error setRefereeMode(const Match& ma, RefereeMode newMode) const;
    Error assignReferee(const Match& ma, const Player& p, RefereeAction refAction) const;
    Error removeReferee(const Match& ma) const;

    // swap player between matches if match results are
    // changed after a match has finished
    Error swapPlayer(const Match& ma, const PlayerPair& ppOld, const PlayerPair& ppNew) const;
    Error swapPlayers(const Match& ma1, const PlayerPair& ma1PlayerPair,
                    const Match& ma2, const PlayerPair& ma2PlayerPair) const;

    std::string getSyncString(const std::vector<int>& rows) const override;
    std::string getSyncString_MatchGroups(std::vector<int> rows);

  private:
    SqliteOverlay::DbTab groupTab;
    void updateAllMatchGroupStates(const Category& cat) const;
    bool hasUnfinishedMandatoryPredecessor(const Match& ma) const;
    void resolveSymbolicNamesAfterFinishedMatch(const Match& ma) const;
    void updateMatchStatus(const Match& ma) const;
    static constexpr int SymbolicIdForUnusedPlayerPairInMatch = 999999;
    
  signals:

  public slots:
    void onPlayerStatusChanged(int playerId, int playerSeqNum, ObjState fromState, ObjState toState);
} ;

}
#endif	/* MATCHMNGR_H */

