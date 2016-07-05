/*
 *    This is QTournament, a badminton tournament management program.
 *    Copyright (C) 2014 - 2016  Volker Knollmann
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

#ifndef CENTRALSIGNALEMITTER_H
#define CENTRALSIGNALEMITTER_H

#include <QObject>

#include "Category.h"
#include "Player.h"
#include "Court.h"

namespace QTournament
{

  class CentralSignalEmitter : public QObject
  {
    Q_OBJECT

  public:
    static CentralSignalEmitter* getInstance();

  signals:
    // Signals emitted by the CatMngr
    void playersPaired(const Category c, const Player& p1, const Player& p2) const;
    void playersSplit(const Category c, const Player& p1, const Player& p2) const;
    void playerAddedToCategory(const Player& p, const Category& c) const;
    void playerRemovedFromCategory(const Player& p, const Category& c) const;
    void beginCreateCategory() const;
    void endCreateCategory(int newCatSeqNum) const;
    void categoryStatusChanged(const Category& c, const OBJ_STATE fromState, const OBJ_STATE toState);
    void beginDeleteCategory(int catSeqNum) const;
    void endDeleteCategory() const;
    void beginResetAllModels() const;
    void endResetAllModels() const;
    void categoryRemovedFromTournament(int invalidCatId, int invalidCatSeqNum);

    // Signals emitted by the CourtMngr
    void beginCreateCourt ();
    void endCreateCourt (int newCourtSeqNum);
    void courtRenamed (const Court& p);
    void courtStatusChanged(int courtId, int courtSeqNum, OBJ_STATE fromState, OBJ_STATE toState);
    void beginDeleteCourt(int courtSeqNum);
    void endDeleteCourt();

    // Signals emitted by the MatchMngr
    void beginCreateMatchGroup ();
    void endCreateMatchGroup (int newMatchGroupSeqNum);
    void beginCreateMatch();
    void endCreateMatch(int newMatchSeqNum);
    void matchStatusChanged(int matchId, int matchSeqNum, OBJ_STATE fromState, OBJ_STATE toState) const;
    void matchGroupStatusChanged(int matchGroupId, int matchGroupSeqNum, OBJ_STATE fromState, OBJ_STATE toState) const;
    void matchResultUpdated(int matchId, int matchSeqNum) const;
    void roundCompleted(int catId, int round) const;

    // Signals emitted by the PlayerMngr
    void beginCreatePlayer ();
    void endCreatePlayer (int newPlayerSeqNum);
    void playerRenamed (const Player& p);
    void playerStatusChanged(int playerId, int playerSeqNum, OBJ_STATE fromState, OBJ_STATE toState) const;
    void beginDeletePlayer(int playerSeqNum) const;
    void endDeletePlayer() const;
    void externalPlayerDatabaseChanged();

    // Signals emitted by the TeamMngr
    void beginCreateTeam ();
    void endCreateTeam (int newTeamSeqNum);
    void teamRenamed(int teamSeqNum);
    void teamAssignmentChanged(const Player& affectedPlayer, const Team& oldTeam, const Team& newTeam);

    // Signals emitted by the MatchTimePredictor
    void matchTimePredictionChanged(int newAvgMatchDuration, time_t finishOfLastScheduledMatch__UTC);

  public slots:

  private:
    explicit CentralSignalEmitter(QObject *parent = 0);
    static CentralSignalEmitter* inst;
  };

}
#endif // CENTRALSIGNALEMITTER_H
