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

#ifndef PLAYERMNGR_H
#define	PLAYERMNGR_H

#include <functional>
#include <memory>

#include <QList>
#include <QObject>

#include "TournamentDB.h"
#include "Team.h"
#include "Player.h"
#include "PlayerPair.h"
#include "TournamentDataDefs.h"
#include "TournamentErrorCodes.h"
#include <SqliteOverlay/DbTab.h>
#include "TournamentDatabaseObjectManager.h"
#include "Match.h"
#include "ExternalPlayerDB.h"


namespace QTournament
{

  class PlayerMngr : public QObject, public TournamentDatabaseObjectManager
  {
    Q_OBJECT
    
  public:
    // Ctor
    PlayerMngr (const TournamentDB& _db);

    // player creation
    ERR createNewPlayer (const QString& firstName, const QString& lastName, SEX sex, const QString& teamName);

    // getters and (boolean) queries
    bool hasPlayer (const QString& firstName, const QString& lastName);
    Player getPlayer(const QString& firstName, const QString& lastName);
    std::vector<Player> getAllPlayers();
    std::optional<Player> getPlayerBySeqNum(int seqNum);
    bool hasPlayer(int id);
    Player getPlayer(int id);
    std::optional<Player> getPlayer2(int id) const;
    PlayerPair getPlayerPair(int id);
    std::optional<PlayerPair> getPlayerPair2(int pairId) const;
    PlayerList determineActualPlayersForMatch(const Match& ma) const;
    ERR canDeletePlayer(const Player& p) const;
    int getTotalPlayerCount() const;
    void getRecentFinishers(int maxCnt, PlayerPairList& winners_out, PlayerPairList& losers_out, PlayerPairList& draw_out) const;
    std::optional<Match> getLastFinishedMatchForPlayer(const Player& p);
    std::optional<Match> getNextMatchForPlayer(const Player& p);
    std::vector<Match> getAllScheduledMatchesForPlayer(const Player& p, bool findFirstOnly = false);

    // referee functions
    void increaseRefereeCountForPlayer(const Player& p);

    // player modification
    ERR renamePlayer (const Player& p, const QString& newFirst, const QString& newLast);
    ERR deletePlayer(const Player& p) const;

    // allocating and releasing players for matches
    ERR canAcquirePlayerPairsForMatch(const Match& ma);
    ERR acquirePlayerPairsForMatch(const Match& ma);
    ERR releasePlayerPairsAfterMatch(const Match& ma);

    // handling of "Wait for registration"
    ERR setWaitForRegistration(const Player& p, bool waitForPlayerRegistration) const;

    // sort functions
    static std::function<bool (Player&, Player&)> getPlayerSortFunction_byName();



    //
    // handling of the external player database
    //

    // getters and (boolean) queries
    bool hasExternalPlayerDatabaseOpen() const;
    bool hasExternalPlayerDatabaseAvailable() const;
    bool hasExternalPlayerDatabaseConfigured() const;
    ExternalPlayerDB* getExternalPlayerDatabaseHandle();  // this returns the address of the LOCAL, internal database handle!
    QString getExternalDatabaseName() const;

    // creation, opening, closing
    ERR setExternalPlayerDatabase(const QString& fname, bool createNew);
    ERR openConfiguredExternalPlayerDatabase();
    void closeExternalPlayerDatabase();

    // importing and exporting players
    //std::optional<Player> importPlayerFromExternalDatabase(ERR* err, int extPlayerId, SEX sexOverride = DONT_CARE);  // never used in the project
    ERR exportPlayerToExternalDatabase(int playerId);
    ERR exportPlayerToExternalDatabase(const Player& p);
    ERR syncAllPlayersToExternalDatabase();

    std::string getSyncString(const std::vector<int>& rows) const override;
    std::string getSyncString_P2C(std::vector<int> rows) const;
    std::string getSyncString_Pairs(std::vector<int> rows) const;

  protected:
    std::optional<ExternalPlayerDB> extPlayerDb;
  };
}

#endif	/* TEAMMNGR_H */

