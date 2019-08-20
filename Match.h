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

#ifndef Error::MATCH_H
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

  enum class RefereeAction
  {
    PreAssign,
    MatchCall,
    Swap,
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
                                 QString& row1Right_out, QString& row2Right_out) const;
    QString getDisplayName(const QString& localWinnerName, const QString& localLoserName) const;

    std::optional<MatchScore> getScore(Error *err=nullptr) const;
    std::optional<PlayerPair> getWinner() const;
    std::optional<PlayerPair> getLoser() const;

    std::optional<Court> getCourt(Error *err) const;
    PlayerList determineActualPlayers() const;

    int getSymbolicPlayerPair1Name() const;
    int getSymbolicPlayerPair2Name() const;

    int getWinnerRank() const;
    int getLoserRank() const;

    bool isWalkoverPossible() const;
    bool isWonByWalkover() const;

    QDateTime getStartTime() const;
    QDateTime getFinishTime() const;
    void addAddtionalCallTime() const;
    QList<QDateTime> getAdditionalCallTimes() const;
    int getMatchDuration() const;

    RefereeMode get_RAW_RefereeMode() const;
    RefereeMode get_EFFECTIVE_RefereeMode() const;
    std::optional<Player> getAssignedReferee() const;
    bool hasRefereeAssigned() const;
    Error canAssignReferee(RefereeAction refAction) const;

  private:
    Match (const TournamentDB& _db, int rowId);
    Match (const TournamentDB& _db, const SqliteOverlay::TabRow& _row);
    int getSymbolicPlayerPairName(int playerPos) const;

  };

  using MatchList = std::vector<Match>;
}
#endif	/* Error::MATCH_H */

