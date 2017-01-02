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

#ifndef MATCH_H
#define	MATCH_H

#include <memory>

#include <QDateTime>
#include <QList>

#include <SqliteOverlay/GenericObjectManager.h>

#include "TournamentDatabaseObject.h"
#include "TournamentDB.h"
#include <SqliteOverlay/TabRow.h>
#include "TournamentErrorCodes.h"
#include "Category.h"
#include "Court.h"
//#include "MatchGroup.h"
#include "Score.h"


namespace QTournament
{
  class MatchGroup;

  enum class REFEREE_ACTION
  {
    PRE_ASSIGN,
    MATCH_CALL,
    SWAP,
  };

  class Match : public TournamentDatabaseObject
  {
    friend class MatchMngr;
    friend class MatchGroup;
    friend class SqliteOverlay::GenericObjectManager<TournamentDB>;
    friend class TournamentDatabaseObjectManager;
    
  public:
    Category getCategory() const;
    MatchGroup getMatchGroup() const;
    bool hasPlayerPair1() const;
    bool hasPlayerPair2() const;
    bool hasBothPlayerPairs() const;
    PlayerPair getPlayerPair1() const;
    PlayerPair getPlayerPair2() const;
    int getMatchNumber() const;
    void getDisplayNameTextItems(const QString& localWinnerName, const QString& localLoserName,
                                 QString& row1Left_out, QString& row2Left_out,
                                 QString& row1Right_out, QString& row2Right_out,
                                 bool& isDoubles) const;
    QString getDisplayName(const QString& localWinnerName, const QString& localLoserName) const;

    unique_ptr<MatchScore> getScore(ERR *err=nullptr) const;
    unique_ptr<PlayerPair> getWinner() const;
    unique_ptr<PlayerPair> getLoser() const;

    unique_ptr<Court> getCourt(ERR *err=nullptr) const;
    PlayerList determineActualPlayers() const;

    int getSymbolicPlayerPair1Name() const;
    int getSymbolicPlayerPair2Name() const;

    int getWinnerRank() const;
    int getLoserRank() const;

    bool isWalkoverPossible() const;

    QDateTime getStartTime() const;
    QDateTime getFinishTime() const;
    bool addAddtionalCallTime() const;
    QList<QDateTime> getAdditionalCallTimes() const;
    int getMatchDuration() const;

    REFEREE_MODE get_RAW_RefereeMode() const;
    REFEREE_MODE get_EFFECTIVE_RefereeMode() const;
    upPlayer getAssignedReferee() const;
    bool hasRefereeAssigned() const;
    ERR canAssignReferee(REFEREE_ACTION refAction) const;

  private:
    Match (TournamentDB* db, int rowId);
    Match (TournamentDB* db, SqliteOverlay::TabRow row);
    int getSymbolicPlayerPairName(int playerPos) const;

  };

  typedef unique_ptr<Match> upMatch;

}
#endif	/* MATCH_H */

