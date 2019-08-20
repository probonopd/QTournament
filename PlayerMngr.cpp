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

#include <stdexcept>

#include <QDebug>
#include <QList>
#include <QFile>

#include "PlayerMngr.h"
#include "Player.h"
#include "TournamentErrorCodes.h"
#include "TournamentDataDefs.h"
#include "HelperFunc.h"
#include "TeamMngr.h"
#include "CatMngr.h"
#include "CentralSignalEmitter.h"
#include <SqliteOverlay/KeyValueTab.h>
#include "MatchMngr.h"

using namespace SqliteOverlay;

namespace QTournament
{

  PlayerMngr::PlayerMngr(const TournamentDB& _db)
  : TournamentDatabaseObjectManager(_db, TabPlayer)
  {
  }

//----------------------------------------------------------------------------

  Error PlayerMngr::createNewPlayer(const QString& firstName, const QString& lastName, Sex sex, const QString& teamName)
  {
    QString first = firstName.trimmed();
    QString last = lastName.trimmed();
    
    if (first.isEmpty() || last.isEmpty())
    {
      return Error::InvalidName;
    }
    
    if ((first.length() > MaxNameLen) || (last.length() > MaxNameLen))
    {
      return Error::InvalidName;
    }
    
    if (hasPlayer(first, last))
    {
      return Error::NameExists;
    }
    
    if (sex == Sex::DontCare)
    {
      return Error::InvalidSex;
    }
    
    // prepare a new table row
    ColumnValueClause cvc;
    cvc.addCol(PL_Fname, first.toUtf8().constData());
    cvc.addCol(PL_Lname, last.toUtf8().constData());
    cvc.addCol(PL_Sex, static_cast<int>(sex));
    cvc.addCol(GenericStateFieldName, static_cast<int>(ObjState::PL_Idle));
    
    // set the team reference, if applicable
    auto cfg = SqliteOverlay::KeyValueTab{db.get(), TabCfg};
    if (cfg.getInt(CfgKey_UseTeams) != 0)
    {
      if (teamName.isEmpty())
      {
        return Error::InvalidTeam;
      }
      
      TeamMngr tm{db};
      if (!(tm.hasTeam(teamName)))
      {
        return Error::InvalidTeam;
      }
      
      Team t = tm.getTeam(teamName);
      cvc.addCol(PL_TeamRef, t.getId());
    }
    
    // create the new player row
    CentralSignalEmitter* cse = CentralSignalEmitter::getInstance();
    cse->beginCreatePlayer();
    tab.insertRow(cvc);
    fixSeqNumberAfterInsert();
    cse->endCreatePlayer(tab.length() - 1); // the new sequence number is always the greatest
    
    return Error::OK;
  }

//----------------------------------------------------------------------------

  bool PlayerMngr::hasPlayer(const QString& firstName, const QString& lastName)
  {
    WhereClause wc;
    wc.addCol(PL_Fname, firstName.toUtf8().constData());
    wc.addCol(PL_Lname, lastName.toUtf8().constData());
    
    return (tab.getMatchCountForWhereClause(wc) > 0);
  }

//----------------------------------------------------------------------------

  /**
   * Returns a database object for a player identified by its name
   *
   * Note: the player must exist, otherwise this method throws an exception!
   *
   * @param firstName is the first name of the player to look up
   * @param lastName is the last name of the player to look up
   *
   * @return a Player instance of that player
   */
  Player PlayerMngr::getPlayer(const QString& firstName, const QString& lastName)
  {
    WhereClause wc;
    wc.addCol(PL_Fname, firstName.toUtf8().constData());
    wc.addCol(PL_Lname, lastName.toUtf8().constData());
    auto r = tab.getSingleRowByWhereClause2(wc);

    if (!r.has_value())
    {
      throw std::invalid_argument("The player '" + QString2StdString(firstName + " " + lastName) + "' does not exist");
    }
    
    return Player{db, *r};
  }

//----------------------------------------------------------------------------

  /**
   * Returns a list of all players
   *
   * @Return QList holding all Teams
   */
  std::vector<Player> PlayerMngr::getAllPlayers()
  {
    return getAllObjects<Player>();
  }

//----------------------------------------------------------------------------

  Error PlayerMngr::renamePlayer(const Player& p, const QString& nf, const QString& nl)
  {
    QString newFirst = nf.trimmed();
    QString newLast = nl.trimmed();
    
    // Ensure the new name is valid
    if ((newFirst.isEmpty()) && (newLast.isEmpty()))
    {
      return Error::InvalidName;
    }
    if ((newFirst.length() > MaxNameLen) || (newLast.length() > MaxNameLen))
    {
      return Error::InvalidName;
    }
    
    // combine the new name from old and new values
    if (newFirst.isEmpty())
    {
      newFirst = p.getFirstName();
    }
    if (newLast.isEmpty())
    {
      newLast = p.getLastName();
    }
    
    // make sure the new name doesn't exist yet
    if (hasPlayer(newFirst, newLast))
    {
      return Error::NameExists;
    }
    
    ColumnValueClause cvc;
    cvc.addCol(PL_Fname, newFirst.toUtf8().constData());
    cvc.addCol(PL_Lname, newLast.toUtf8().constData());

    p.row.update(cvc);
    
    CentralSignalEmitter::getInstance()->playerRenamed(p);
    
    return Error::OK;
  }

//----------------------------------------------------------------------------

  /**
   * Returns a database object for a player identified by its sequence number
   *
   * @param seqNum is the sequence number of the player to look up
   *
   * @return a unique_ptr to a Player instance of that player
   */
  std::optional<Player> PlayerMngr::getPlayerBySeqNum(int seqNum)
  {
    return getSingleObjectByColumnValue<Player>(GenericSeqnumFieldName, seqNum);
  }


//----------------------------------------------------------------------------

  bool PlayerMngr::hasPlayer(int id)
  {
    return tab.hasRowId(id);
  }

//----------------------------------------------------------------------------

  Player PlayerMngr::getPlayer(int id)
  {
    // this public function essentially short-circuits the private player()-constructor... Hmmm...
    return Player{db, id};
  }

  //----------------------------------------------------------------------------

  std::optional<Player> PlayerMngr::getPlayer2(int id) const
  {
    return getSingleObjectByColumnValue<Player>("id", id);
  }

//----------------------------------------------------------------------------

  PlayerPair PlayerMngr::getPlayerPair(int id)
  {
    return PlayerPair{db, id};
    
    /*
    Player p1(db, r[Pairs_Player1Ref].toInt());
    
    QVariant _id2 = r[Pairs_Player2Ref];
    if (_id2.isNull())
    {
      // we have a "pair-without-partner"
      return PlayerPair(p1, id);
    }
    
    // we do have a second player
    Player p2(db, _id2.toInt());
    
    return PlayerPair(p1, p2, id);
    */
  }

  //----------------------------------------------------------------------------

  std::optional<PlayerPair> PlayerMngr::getPlayerPair2(int pairId) const
  {
    DbTab pairsTab{db.get(), TabPairs, false};
    auto r = pairsTab.getSingleRowByColumnValue2("id", pairId);

    return (r.has_value()) ? PlayerPair{db.get(), pairId} : std::optional<PlayerPair>{};
  }

//----------------------------------------------------------------------------

  Error PlayerMngr::canAcquirePlayerPairsForMatch(const Match& ma)
  {
    PlayerList pl = determineActualPlayersForMatch(ma);

    for (Player p : pl)
    {
      if (p.getState() != ObjState::PL_Idle) return Error::PlayerNotIdle;
    }

    // check for the referee, if any
    RefereeMode refMode = ma.get_EFFECTIVE_RefereeMode();
    if ((refMode != RefereeMode::None) && (refMode != RefereeMode::HandWritten))
    {
      auto referee = ma.getAssignedReferee();

      // maybe no referee has been assigned yet. That's okay for now.
      if (!referee.has_value()) return Error::OK;

      // if a referee has been assigned, check its availability
      if (referee->getState() != ObjState::PL_Idle) return Error::RefereeNotIdle;
    }

    return Error::OK;
  }

//----------------------------------------------------------------------------

  Error PlayerMngr::acquirePlayerPairsForMatch(const Match& ma)
  {
    Error e = canAcquirePlayerPairsForMatch(ma);
    if (e != Error::OK) return e;

    // update the status of all players to PLAYING
    PlayerList pl = determineActualPlayersForMatch(ma);

    for (const Player& p : pl)
    {
      ObjState oldStat = p.getState();
      p.row.update(GenericStateFieldName, static_cast<int>(ObjState::PL_Playing));
      CentralSignalEmitter::getInstance()->playerStatusChanged(p.getId(), p.getSeqNum(), oldStat, ObjState::PL_Playing);
    }

    return Error::OK;
  }

//----------------------------------------------------------------------------

  Error PlayerMngr::releasePlayerPairsAfterMatch(const Match& ma)
  {
    PlayerList pl = determineActualPlayersForMatch(ma);

    // update all player states back to idle
    for (const Player& p : pl)
    {
      p.row.update(GenericStateFieldName, static_cast<int>(ObjState::PL_Idle));
      CentralSignalEmitter::getInstance()->playerStatusChanged(p.getId(), p.getSeqNum(), ObjState::PL_Playing, ObjState::PL_Idle);
    }

    return Error::OK;
  }

//----------------------------------------------------------------------------

  PlayerList PlayerMngr::determineActualPlayersForMatch(const Match &ma) const
  {
    // Important assumption: the "actual players" section has to be always
    // filled completely or not at all.
    // E.g., it is not permitted to set only "actual player 1a" without setting
    // "actual player 2a" at the same time!


    //
    // this function serves as a hook to (later) introduce
    // resolving player substitutions etc. into a list of
    // effective players for a game
    //

    PlayerList result;
    PlayerMngr pm{db};

    // have "actual players" already been assigned?
    // if yes, return those values. They overrule everything else
    TabRow matchRow{db, TabMatch, ma.getId(), true};
    auto pRef = matchRow.getInt2(MA_ActualPlayer1aRef);
    if (pRef.has_value())
    {
      result.push_back(pm.getPlayer(*pRef));

      pRef = matchRow.getInt2(MA_ActualPlayer1bRef);
      if (pRef.has_value()) result.push_back(pm.getPlayer(*pRef));

      pRef = matchRow.getInt2(MA_ActualPlayer2aRef);
      if (pRef.has_value()) result.push_back(pm.getPlayer(*pRef));

      pRef = matchRow.getInt2(MA_ActualPlayer2bRef);
      if (pRef.has_value()) result.push_back(pm.getPlayer(*pRef));

      return result;
    }


    // okay, if we don't have "actual players" stored, we go by player pairs


    // return an empty list if player pairs are incomplete.
    // Otherwise, we can't distinguish between a singles match and
    // an incomplete doubles match
    if ((!(ma.hasPlayerPair1())) || (!(ma.hasPlayerPair2())))
    {
      return PlayerList();
    }

    PlayerPair pp1 = ma.getPlayerPair1();
    PlayerPair pp2 = ma.getPlayerPair2();
    result.push_back(pp1.getPlayer1());
    result.push_back(pp2.getPlayer1());
    if (pp1.hasPlayer2()) result.push_back(pp1.getPlayer2());
    if (pp2.hasPlayer2()) result.push_back(pp2.getPlayer2());

    //
    // TODO:
    // Here, a potential subsitution algorithm could kick in
    // and replace the "theoretical players" with "actual players", if applicable
    //

    return result;
  }

  //----------------------------------------------------------------------------

  Error PlayerMngr::setWaitForRegistration(const Player& p, bool waitForPlayerRegistration) const
  {
    // A player can be set to "Wait For Registration" if and only if:
    //  * The player is currently idle; and
    //  * The player is only assigned to categories in state "CONFIG" or "FINISHED"
    //
    // A player can be released from "Wait For Registration" at any time. The player
    // always changes to state "IDLE". However, it is not guaranteed that the player is
    // in IDLE after calling this method, because we don't change the player state at all if
    // it was in a different state than WAIT_FOR_REGISTRATION (e.g., PLAYING)

    ObjState plStat = p.getState();
    CentralSignalEmitter* cse = CentralSignalEmitter::getInstance();

    // easiest case first: un-set "wait for registration"
    if (waitForPlayerRegistration == false)
    {
      // if the player wasn't in wait state, return directly without error
      if (plStat != ObjState::PL_WaitForRegistration) return Error::OK;

      // switch to IDLE
      p.setState(ObjState::PL_Idle);
      cse->playerStatusChanged(p.getId(), p.getSeqNum(), ObjState::PL_WaitForRegistration, ObjState::PL_Idle);
      return Error::OK;
    }

    // second case: enable "wait for registration"

    // there is nothing to do for us if the player is already in wait state
    if (plStat == ObjState::PL_WaitForRegistration) return Error::OK;

    // if the player isn't IDLE, we can't switch to "wait for registration"
    if (plStat != ObjState::PL_Idle)
    {
      return Error::PlayerAlreadyInMatches;
    }

    // okay, the player is idle and shall be switched to "wait state".
    // make sure that all assigned categories are either in CONFIG or FINISHED
    for (const Category& cat : p.getAssignedCategories())
    {
      ObjState catStat = cat.getState();
      if ((catStat != ObjState::CAT_Config) && (catStat != ObjState::CAT_Finalized))
      {
        return Error::PlayerAlreadyInMatches;
      }
    }

    // all checks passed ==> we can switch the player to "wait for registration"
    p.setState(ObjState::PL_WaitForRegistration);
    cse->playerStatusChanged(p.getId(), p.getSeqNum(), ObjState::PL_Idle, ObjState::PL_WaitForRegistration);

    return Error::OK;
  }

  //----------------------------------------------------------------------------

  std::function<bool (Player&, Player&)> PlayerMngr::getPlayerSortFunction_byName()
  {
    return [](Player& p1, Player& p2) {
      // compare last name
      int cmpLast = QString::localeAwareCompare(p1.getLastName(), p2.getLastName());
      if (cmpLast < 0) return true;
      if (cmpLast > 0) return false;

      // last name is identical ==> compare first name
      int cmpFirst = QString::localeAwareCompare(p1.getFirstName(), p2.getFirstName());
      if (cmpFirst < 0) return true;
      if (cmpFirst > 0) return false;

      // names are identical. So we display the player who has registered earlier
      // as the first player
      if (p1.getId() < p2.getId()) return true;
      return false;
    };
  }

  //----------------------------------------------------------------------------

  bool PlayerMngr::hasExternalPlayerDatabaseOpen() const
  {
    return (extPlayerDb.has_value());
  }

  //----------------------------------------------------------------------------

  bool PlayerMngr::hasExternalPlayerDatabaseAvailable() const
  {
    // first criterion: the database must be configured
    if (!(hasExternalPlayerDatabaseConfigured())) return false;

    // second criterion: the file must exist
    QString dbName = getExternalDatabaseName();
    return QFile::exists(dbName);
  }

  //----------------------------------------------------------------------------

  bool PlayerMngr::hasExternalPlayerDatabaseConfigured() const
  {
    auto cfg = SqliteOverlay::KeyValueTab{db.get(), TabCfg};

    auto fName = cfg.getString2(CfgKey_ExtPlayerDb);

    if (!fName.has_value()) return false;

    return !(fName->empty());
  }

  //----------------------------------------------------------------------------

  ExternalPlayerDB* PlayerMngr::getExternalPlayerDatabaseHandle()
  {
    return (!extPlayerDb.has_value()) ? nullptr : &(*extPlayerDb);
  }

  //----------------------------------------------------------------------------

  QString PlayerMngr::getExternalDatabaseName() const
  {
    auto cfg = SqliteOverlay::KeyValueTab{db.get(), TabCfg};

    auto fName = cfg.getString2(CfgKey_ExtPlayerDb);

    if (!fName.has_value()) return QString{};

    return QString::fromUtf8(fName->data());
  }

  //----------------------------------------------------------------------------

  Error PlayerMngr::setExternalPlayerDatabase(const QString& fname, bool createNew)
  {
    std::optional<ExternalPlayerDB> extDb{};

    if (fname.isEmpty()) return Error::EPD_InvalidDatabaseName;

    // try to create the new database
    if (createNew)
    {
      extDb = ExternalPlayerDB::createNew(fname);
      if (!extDb) return Error::EPD_CreationFailed;
    }
    // try to open an existing database
    else
    {
      extDb = ExternalPlayerDB::openExisting(fname);
      if (!extDb) return Error::EPD_NotFound;
    }

    // close the old database, if open
    closeExternalPlayerDatabase();

    // store the handle to the new database and
    // overwrite the database config key with
    // the new filename
    //
    // do this only if the database name actually changed
    extPlayerDb = std::move(extDb);
    auto cfg = SqliteOverlay::KeyValueTab{db.get(), TabCfg};
    std::string _oldFileName = cfg[CfgKey_ExtPlayerDb];
    QString oldFileName = QString::fromUtf8(_oldFileName.data());
    if (oldFileName != fname)
    {
      cfg.set(CfgKey_ExtPlayerDb, fname.toUtf8().constData());
    }

    CentralSignalEmitter::getInstance()->externalPlayerDatabaseChanged();

    return Error::OK;
  }

  //----------------------------------------------------------------------------

  Error PlayerMngr::openConfiguredExternalPlayerDatabase()
  {
    if (!(hasExternalPlayerDatabaseConfigured()))
    {
      return Error::EPD_NotConfigured;
    }

    auto cfg = SqliteOverlay::KeyValueTab{db.get(), TabCfg};
    QString playerDbName = QString::fromUtf8(cfg[CfgKey_ExtPlayerDb].data());

    return setExternalPlayerDatabase(playerDbName, false);
  }

  //----------------------------------------------------------------------------

  void PlayerMngr::closeExternalPlayerDatabase()
  {
    if (!extPlayerDb) return;

    extPlayerDb->close();

    // this operation automatically calls the destructor
    // of the underlying database object
    extPlayerDb.reset();

    CentralSignalEmitter::getInstance()->externalPlayerDatabaseChanged();
  }

  //----------------------------------------------------------------------------

  /*
   * Obviously this method is never called from anyone... weird....
   *
  std::optional<Player> PlayerMngr::importPlayerFromExternalDatabase(ERR* err, int extPlayerId, Sex sexOverride)
  {
    // if no player database has been opened by this instance of the PlayerMngr,
    // try to open the database
    if (!extPlayerDb)
    {
      Error e = openConfiguredExternalPlayerDatabase();
      if (e != Error::OK)
      {
        Sloppy::assignIfNotNull<ERR>(err, e);
        return std::optional<Player>{};
      }
    }

    // check the ID's validity
    auto extPlayer = extPlayerDb->getPlayer(extPlayerId);
    if (!extPlayer)
    {
      Sloppy::assignIfNotNull<ERR>(err, Error::InvalidId);
      return std::optional<Player>{};
    }

    // TODO: remove this function?
    Sloppy::assignIfNotNull<ERR>(err, OK);

    return extPlayer;
  }
  */

  //----------------------------------------------------------------------------

  Error PlayerMngr::exportPlayerToExternalDatabase(int playerId)
  {
    auto p = getPlayer2(playerId);
    if (!p.has_value())
    {
      return Error::InvalidId;
    }

    return exportPlayerToExternalDatabase(*p);
  }

  //----------------------------------------------------------------------------

  Error PlayerMngr::exportPlayerToExternalDatabase(const Player& p)
  {
    // if no player database has been opened by this instance of the PlayerMngr,
    // try to open the database
    if (!extPlayerDb)
    {
      Error err = openConfiguredExternalPlayerDatabase();
      if (err != Error::OK)
      {
        return err;
      }
    }

    auto extPlayer = extPlayerDb->getPlayer(p.getFirstName(), p.getLastName());
    if (!extPlayer)
    {
      // create a new player
      ExternalPlayerDatabaseEntry entry{p.getFirstName(), p.getLastName(), p.getSex()};
      auto newPlayer = extPlayerDb->storeNewPlayer(entry);

      return (!newPlayer) ? Error::EPD_CreationFailed : Error::OK;
    }

    // update existing player, if applicable
    if (extPlayer->getSex() == Sex::DontCare)
    {
      extPlayerDb->updatePlayerSexIfUndefined(extPlayer->getId(), p.getSex());
    }

    return Error::OK;
  }

  //----------------------------------------------------------------------------

  Error PlayerMngr::syncAllPlayersToExternalDatabase()
  {
    // if no player database has been opened by this instance of the PlayerMngr,
    // try to open the database
    if (!extPlayerDb)
    {
      Error err = openConfiguredExternalPlayerDatabase();
      if (err != Error::OK)
      {
        return err;
      }
    }

    for (const Player& p : getAllPlayers())
    {
      exportPlayerToExternalDatabase(p);
    }

    return Error::OK;
  }

  //----------------------------------------------------------------------------

  std::string PlayerMngr::getSyncString(const std::vector<int>& rows) const
  {
    std::vector<Sloppy::estring> cols = {"id", PL_Fname, PL_Lname, GenericStateFieldName, PL_Sex, PL_RefereeCount, PL_TeamRef};

    return db.get().getSyncStringForTable(TabPlayer, cols, rows);
  }

  //----------------------------------------------------------------------------

  std::string PlayerMngr::getSyncString_P2C(std::vector<int> rows) const
  {
    std::vector<Sloppy::estring> cols = {"id", P2C_PlayerRef, P2C_CatRef};

    return db.get().getSyncStringForTable(TabP2C, cols, rows);
  }

  //----------------------------------------------------------------------------

  std::string PlayerMngr::getSyncString_Pairs(std::vector<int> rows) const
  {
    std::vector<Sloppy::estring> cols = {"id", Pairs_Player1Ref, Pairs_Player2Ref, Pairs_CatRef, Pairs_GrpNum, Pairs_InitialRank};

    return db.get().getSyncStringForTable(TabPairs, cols, rows);
  }

  //----------------------------------------------------------------------------

  Error PlayerMngr::canDeletePlayer(const Player &p) const
  {
    // first check: see if we can remove the player from all categories
    auto assignedCats = p.getAssignedCategories();
    for (const Category& c : assignedCats)
    {
      if (!(c.canRemovePlayer(p)))
      {
        return Error::PlayerNotRemovableFromCategory;
      }
    }

    // second check: ensure that there are not existing / planned
    // matches that involve this player
    //
    // step 1: get all player pairs that involve this player
    Sloppy::estring where{"%1 = %2 OR %3 = %2"};
    where.arg(Pairs_Player1Ref);
    where.arg(p.getId());
    where.arg(Pairs_Player2Ref);
    DbTab pairTab{db, TabPairs, false};
    PlayerPairList assignedPairs = getObjectsByWhereClause<PlayerPair>(pairTab, where);

    // step 2: check if we have any matches that involve one of these pairs
    DbTab matchTab{db, TabMatch, false};
    for (PlayerPair pp : assignedPairs)
    {
      int ppId = pp.getPairId();
      if (ppId < 0) continue;  // player pair in a not-yet-started category
      where = Sloppy::estring{"%1 = %2 OR %3 = %2"};
      where.arg(MA_Pair1Ref);
      where.arg(ppId);
      where.arg(MA_Pair2Ref);
      int cnt = matchTab.getMatchCountForWhereClause(where);
      if (cnt != 0)
      {
        return Error::PlayerAlreadyInMatches;
      }
    }

    // third check: the player may not be referenced as an actual player
    // (e.g., as a substitue) in any match
    where = Sloppy::estring{"%1 = %2 OR %3 = %2 OR %4 = %2 OR %5 = %2"};
    where.arg(MA_ActualPlayer1aRef);
    where.arg(p.getId());
    where.arg(MA_ActualPlayer1bRef);
    where.arg(MA_ActualPlayer2aRef);
    where.arg(MA_ActualPlayer2bRef);
    int cnt = matchTab.getMatchCountForWhereClause(where);
    if (cnt != 0)
    {
      return Error::PlayerAlreadyInMatches;
    }

    // at this point, we have pairs but no matches yet. this means
    // that the player is assigned to a doubles/mixed partner in a
    // not yet started category
    if (!(assignedPairs.empty()))
    {
      return Error::PlayerAlreadyPaired;
    }

    return Error::OK;
  }

  //----------------------------------------------------------------------------

  Error PlayerMngr::deletePlayer(const Player &p) const
  {
    Error e = canDeletePlayer(p);
    if (e != Error::OK)
    {
      return e;
    }

    // remove the player from all categories
    auto assignedCats = p.getAssignedCategories();
    for (Category c : assignedCats)
    {
      e = c.removePlayer(p);
      if (e != Error::OK)
      {
        return e;   // after the checks before, this shoudln't happen
      }
    }

    // there is nothing more to do for us, because there are no more references
    // to this player. This has been checked by canDeletePlayer() before

    // the actual deletion
    int oldSeqNum = p.getSeqNum();
    CentralSignalEmitter* cse = CentralSignalEmitter::getInstance();
    cse->beginDeletePlayer(oldSeqNum);
    tab.deleteRowsByColumnValue("id", p.getId());
    fixSeqNumberAfterDelete(tab, oldSeqNum);
    cse->endDeletePlayer();

    return Error::OK;
  }

  //----------------------------------------------------------------------------

  int PlayerMngr::getTotalPlayerCount() const
  {
    return tab.length();
  }

  //----------------------------------------------------------------------------

  void PlayerMngr::getRecentFinishers(int maxCnt, PlayerPairList& winners_out, PlayerPairList& losers_out, PlayerPairList& draw_out) const
  {
    if (maxCnt < 0) return;

    // search for up to maxCnt recently finished matches
    DbTab matchTab{db, TabMatch, false};
    WhereClause wc;
    wc.addCol(GenericStateFieldName, static_cast<int>(ObjState::MA_Finished));
    wc.setOrderColumn_Desc(MA_FinishTime);
    wc.setLimit(maxCnt);
    MatchList ml = getObjectsByWhereClause<Match>(matchTab, wc);

    // extract the winners and losers from these matches
    for (const Match& ma : ml)
    {
      auto loser = ma.getLoser();
      auto winner = ma.getWinner();

      if (loser) losers_out.push_back(*loser);
      if (winner) winners_out.push_back(*winner);

      // handle draws
      if (!loser && !winner)
      {
        if (ma.hasPlayerPair1()) // should always be true
        {
          draw_out.push_back(ma.getPlayerPair1());
        }
        if (ma.hasPlayerPair2()) // should always be true
        {
          draw_out.push_back(ma.getPlayerPair2());
        }
      }
    }
  }

  //----------------------------------------------------------------------------

  std::optional<Match> PlayerMngr::getLastFinishedMatchForPlayer(const Player& p)
  {
    Sloppy::estring sql = "SELECT id FROM %1 WHERE %2=%3 OR %4=%3 OR %5=%3 OR %6=%3 ORDER BY %7 DESC LIMIT 1";
    sql.arg(TabMatch);
    sql.arg(MA_ActualPlayer1aRef);
    sql.arg(p.getId());
    sql.arg(MA_ActualPlayer1bRef);
    sql.arg(MA_ActualPlayer2aRef);
    sql.arg(MA_ActualPlayer2bRef);
    sql.arg(MA_FinishTime);

    try
    {
      int matchId = db.get().execScalarQueryInt(sql);

      MatchMngr mm{db};
      return mm.getMatch(matchId);
    }
    catch (SqliteOverlay::NoDataException)
    {
      return {};  // no match found
    }
  }

  //----------------------------------------------------------------------------

  std::optional<Match> PlayerMngr::getNextMatchForPlayer(const Player& p)
  {
    std::vector<Match> allNextMatches = getAllScheduledMatchesForPlayer(p, true);
    if (allNextMatches.empty()) return {};

    MatchMngr mm{db};
    return allNextMatches[0];
  }

  //----------------------------------------------------------------------------

  std::vector<Match> PlayerMngr::getAllScheduledMatchesForPlayer(const Player &p, bool findFirstOnly)
  {
    std::vector<Match> result;

    // get all scheduled matches
    WhereClause wc;
    wc.addCol(MA_Num, ">", 0);
    wc.addCol(GenericStateFieldName, "!=", static_cast<int>(ObjState::MA_Running));
    wc.addCol(GenericStateFieldName, "!=", static_cast<int>(ObjState::MA_Finished));
    wc.setOrderColumn_Asc(MA_Num);

    DbTab matchTab{db, TabMatch, false};
    MatchMngr mm{db};
    for (const Match& ma : getObjectsByWhereClause<Match>(matchTab, wc))
    {
      // stop the loop if we only need one match
      if (findFirstOnly && (!(result.empty()))) return result;

      if (ma.hasPlayerPair1())
      {
        PlayerPair pp = ma.getPlayerPair1();
        if (pp.getPlayer1() == p)
        {
          result.push_back(ma);
          continue;
        }

        if (pp.hasPlayer2())
        {
          if (pp.getPlayer2() == p)
          {
            result.push_back(ma);
            continue;
          }
        }
      }

      if (ma.hasPlayerPair2())
      {
        PlayerPair pp = ma.getPlayerPair2();
        if (pp.getPlayer1() == p)
        {
          result.push_back(ma);
          continue;
        }

        if (pp.hasPlayer2())
        {
          if (pp.getPlayer2() == p)
          {
            result.push_back(ma);
            continue;
          }
        }
      }
    }

    return result;
  }

  //----------------------------------------------------------------------------

  void PlayerMngr::increaseRefereeCountForPlayer(const Player& p)
  {
    int oldCount = p.getRefereeCount();

    p.row.update(PL_RefereeCount, oldCount + 1);
  }

  //----------------------------------------------------------------------------


  //----------------------------------------------------------------------------


  //----------------------------------------------------------------------------


  //----------------------------------------------------------------------------


  //----------------------------------------------------------------------------


  //----------------------------------------------------------------------------


  //----------------------------------------------------------------------------


  //----------------------------------------------------------------------------


  //----------------------------------------------------------------------------


}
