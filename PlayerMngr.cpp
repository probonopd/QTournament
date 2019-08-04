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
  : TournamentDatabaseObjectManager(_db, TAB_PLAYER)
  {
  }

//----------------------------------------------------------------------------

  ERR PlayerMngr::createNewPlayer(const QString& firstName, const QString& lastName, SEX sex, const QString& teamName)
  {
    QString first = firstName.trimmed();
    QString last = lastName.trimmed();
    
    if (first.isEmpty() || last.isEmpty())
    {
      return INVALID_NAME;
    }
    
    if ((first.length() > MAX_NAME_LEN) || (last.length() > MAX_NAME_LEN))
    {
      return INVALID_NAME;
    }
    
    if (hasPlayer(first, last))
    {
      return NAME_EXISTS;
    }
    
    if (sex == DONT_CARE)
    {
      return INVALID_SEX;
    }
    
    // prepare a new table row
    ColumnValueClause cvc;
    cvc.addStringCol(PL_FNAME, first.toUtf8().constData());
    cvc.addStringCol(PL_LNAME, last.toUtf8().constData());
    cvc.addIntCol(PL_SEX, static_cast<int>(sex));
    cvc.addIntCol(GENERIC_STATE_FIELD_NAME, static_cast<int>(STAT_PL_IDLE));
    
    // set the team reference, if applicable
    auto cfg = KeyValueTab::getTab(db, TAB_CFG, false);
    if (cfg == nullptr)
    {
      throw std::runtime_error("Config table not found -- this shouldn't happen!");
    }
    if (cfg->getInt(CFG_KEY_USE_TEAMS) != 0)
    {
      if (teamName.isEmpty())
      {
        return INVALID_TEAM;
      }
      
      TeamMngr tm{db};
      if (!(tm.hasTeam(teamName)))
      {
        return INVALID_TEAM;
      }
      
      Team t = tm.getTeam(teamName);
      cvc.addIntCol(PL_TEAM_REF, t.getId());
    }
    
    // lock the database before writing
    DbLockHolder lh{db, DatabaseAccessRoles::MainThread};

    // create the new player row
    CentralSignalEmitter* cse = CentralSignalEmitter::getInstance();
    cse->beginCreatePlayer();
    tab->insertRow(cvc);
    fixSeqNumberAfterInsert();
    cse->endCreatePlayer(tab->length() - 1); // the new sequence number is always the greatest
    
    return OK;
  }

//----------------------------------------------------------------------------

  bool PlayerMngr::hasPlayer(const QString& firstName, const QString& lastName)
  {
    WhereClause wc;
    wc.addStringCol(PL_FNAME, firstName.toUtf8().constData());
    wc.addStringCol(PL_LNAME, lastName.toUtf8().constData());
    
    return (tab->getMatchCountForWhereClause(wc) > 0);
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
    if (!(hasPlayer(firstName, lastName)))
    {
      throw std::invalid_argument("The player '" + QString2StdString(firstName + " " + lastName) + "' does not exist");
    }
    
    WhereClause wc;
    wc.addStringCol(PL_FNAME, firstName.toUtf8().constData());
    wc.addStringCol(PL_LNAME, lastName.toUtf8().constData());
    TabRow r = tab->getSingleRowByWhereClause(wc);
    
    return Player(db, r);
  }

//----------------------------------------------------------------------------

  /**
   * Returns a list of all players
   *
   * @Return QList holding all Teams
   */
  vector<Player> PlayerMngr::getAllPlayers()
  {
    return getAllObjects<Player>();
  }

//----------------------------------------------------------------------------

  ERR PlayerMngr::renamePlayer(Player& p, const QString& nf, const QString& nl)
  {
    QString newFirst = nf.trimmed();
    QString newLast = nl.trimmed();
    
    // Ensure the new name is valid
    if ((newFirst.isEmpty()) && (newLast.isEmpty()))
    {
      return INVALID_NAME;
    }
    if ((newFirst.length() > MAX_NAME_LEN) || (newLast.length() > MAX_NAME_LEN))
    {
      return INVALID_NAME;
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
      return NAME_EXISTS;
    }
    
    ColumnValueClause cvc;
    cvc.addStringCol(PL_FNAME, newFirst.toUtf8().constData());
    cvc.addStringCol(PL_LNAME, newLast.toUtf8().constData());

    // lock the database before writing
    DbLockHolder lh{db, DatabaseAccessRoles::MainThread};

    p.row.update(cvc);
    
    CentralSignalEmitter::getInstance()->playerRenamed(p);
    
    return OK;
  }

//----------------------------------------------------------------------------

  /**
   * Returns a database object for a player identified by its sequence number
   *
   * @param seqNum is the sequence number of the player to look up
   *
   * @return a unique_ptr to a Player instance of that player
   */
  unique_ptr<Player> PlayerMngr::getPlayerBySeqNum(int seqNum)
  {
    return getSingleObjectByColumnValue<Player>(GENERIC_SEQNUM_FIELD_NAME, seqNum);
  }


//----------------------------------------------------------------------------

  bool PlayerMngr::hasPlayer(int id)
  {
    return tab->hasRowId(id);
  }

//----------------------------------------------------------------------------

  Player PlayerMngr::getPlayer(int id)
  {
    // this public function essentially short-circuits the private player()-constructor... Hmmm...
    return Player(db, id);
  }

  //----------------------------------------------------------------------------

  unique_ptr<Player> PlayerMngr::getPlayer_up(int id) const
  {
    return getSingleObjectByColumnValue<Player>("id", id);
  }

//----------------------------------------------------------------------------

  PlayerPair PlayerMngr::getPlayerPair(int id)
  {
    TabRow r = db->getTab(TAB_PAIRS)->operator [](id);

    return PlayerPair(db, r);
    
    /*
    Player p1(db, r[PAIRS_PLAYER1_REF].toInt());
    
    QVariant _id2 = r[PAIRS_PLAYER2_REF];
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

  upPlayerPair PlayerMngr::getPlayerPair_up(int pairId) const
  {
    DbTab* pairsTab = db->getTab(TAB_PAIRS);
    if (!(pairsTab->hasRowId(pairId)))
    {
      return nullptr;
    }

    TabRow r = pairsTab->operator [](pairId);

    return upPlayerPair(new PlayerPair(db, r));
  }

//----------------------------------------------------------------------------

  ERR PlayerMngr::canAcquirePlayerPairsForMatch(const Match& ma)
  {
    PlayerList pl = determineActualPlayersForMatch(ma);

    for (Player p : pl)
    {
      if (p.getState() != STAT_PL_IDLE) return PLAYER_NOT_IDLE;
    }

    // check for the referee, if any
    REFEREE_MODE refMode = ma.get_EFFECTIVE_RefereeMode();
    if ((refMode != REFEREE_MODE::NONE) && (refMode != REFEREE_MODE::HANDWRITTEN))
    {
      upPlayer referee = ma.getAssignedReferee();

      // maybe no referee has been assigned yet. That's okay for now.
      if (referee == nullptr) return OK;

      // if a referee has been assigned, check its availability
      if (referee->getState() != STAT_PL_IDLE) return REFEREE_NOT_IDLE;
    }

    return OK;
  }

//----------------------------------------------------------------------------

  ERR PlayerMngr::acquirePlayerPairsForMatch(const Match& ma)
  {
    ERR e = canAcquirePlayerPairsForMatch(ma);
    if (e != OK) return e;

    // update the status of all players to PLAYING
    PlayerList pl = determineActualPlayersForMatch(ma);

    // lock the database before writing
    DbLockHolder lh{db, DatabaseAccessRoles::MainThread};

    for (Player p : pl)
    {
      OBJ_STATE oldStat = p.getState();
      TabRow r = tab->operator [](p.getId());
      r.update(GENERIC_STATE_FIELD_NAME, static_cast<int>(STAT_PL_PLAYING));
      CentralSignalEmitter::getInstance()->playerStatusChanged(p.getId(), p.getSeqNum(), oldStat, STAT_PL_PLAYING);
    }

    return OK;
  }

//----------------------------------------------------------------------------

  ERR PlayerMngr::releasePlayerPairsAfterMatch(const Match& ma)
  {
    PlayerList pl = determineActualPlayersForMatch(ma);

    // lock the database before writing
    DbLockHolder lh{db, DatabaseAccessRoles::MainThread};

    // update all player states back to idle
    for (Player p : pl)
    {
      TabRow r = tab->operator [](p.getId());
      int dbErr;
      r.update(GENERIC_STATE_FIELD_NAME, static_cast<int>(STAT_PL_IDLE), &dbErr);
      if (dbErr != SQLITE_DONE) return DATABASE_ERROR;
      CentralSignalEmitter::getInstance()->playerStatusChanged(p.getId(), p.getSeqNum(), STAT_PL_PLAYING, STAT_PL_IDLE);
    }

    return OK;
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
    TabRow matchRow = db->getTab(TAB_MATCH)->operator [](ma.getId());
    auto pRef = matchRow.getInt2(MA_ACTUAL_PLAYER1A_REF);
    if (!(pRef->isNull()))
    {
      result.push_back(pm.getPlayer(pRef->get()));

      pRef = matchRow.getInt2(MA_ACTUAL_PLAYER1B_REF);
      if (!(pRef->isNull())) result.push_back(pm.getPlayer(pRef->get()));

      pRef = matchRow.getInt2(MA_ACTUAL_PLAYER2A_REF);
      if (!(pRef->isNull())) result.push_back(pm.getPlayer(pRef->get()));  // should always be true, since we have a valid player1a

      pRef = matchRow.getInt2(MA_ACTUAL_PLAYER2B_REF);
      if (!(pRef->isNull())) result.push_back(pm.getPlayer(pRef->get()));

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

  ERR PlayerMngr::setWaitForRegistration(const Player& p, bool waitForPlayerRegistration) const
  {
    // A player can be set to "Wait For Registration" if and only if:
    //  * The player is currently idle; and
    //  * The player is only assigned to categories in state "CONFIG" or "FINISHED"
    //
    // A player can be released from "Wait For Registration" at any time. The player
    // always changes to state "IDLE". However, it is not guaranteed that the player is
    // in IDLE after calling this method, because we don't change the player state at all if
    // it was in a different state than WAIT_FOR_REGISTRATION (e.g., PLAYING)

    OBJ_STATE plStat = p.getState();
    CentralSignalEmitter* cse = CentralSignalEmitter::getInstance();

    // easiest case first: un-set "wait for registration"
    if (waitForPlayerRegistration == false)
    {
      // if the player wasn't in wait state, return directly without error
      if (plStat != STAT_PL_WAIT_FOR_REGISTRATION) return OK;

      // switch to IDLE
      p.setState(STAT_PL_IDLE);
      cse->playerStatusChanged(p.getId(), p.getSeqNum(), STAT_PL_WAIT_FOR_REGISTRATION, STAT_PL_IDLE);
      return OK;
    }

    // second case: enable "wait for registration"

    // there is nothing to do for us if the player is already in wait state
    if (plStat == STAT_PL_WAIT_FOR_REGISTRATION) return OK;

    // if the player isn't IDLE, we can't switch to "wait for registration"
    if (plStat != STAT_PL_IDLE)
    {
      return PLAYER_ALREADY_IN_MATCHES;
    }

    // okay, the player is idle and shall be switched to "wait state".
    // make sure that all assigned categories are either in CONFIG or FINISHED
    for (const Category& cat : p.getAssignedCategories())
    {
      OBJ_STATE catStat = cat.getState();
      if ((catStat != STAT_CAT_CONFIG) && (catStat != STAT_CAT_FINALIZED))
      {
        return PLAYER_ALREADY_IN_MATCHES;
      }
    }

    // all checks passed ==> we can switch the player to "wait for registration"
    p.setState(STAT_PL_WAIT_FOR_REGISTRATION);
    cse->playerStatusChanged(p.getId(), p.getSeqNum(), STAT_PL_IDLE, STAT_PL_WAIT_FOR_REGISTRATION);

    return OK;
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
    return (extPlayerDb != nullptr);
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
    auto cfg = KeyValueTab::getTab(db, TAB_CFG, false);
    if (cfg == nullptr)
    {
      throw std::runtime_error("Config table not found -- this shouldn't happen!");
    }

    if (!(cfg->hasKey(CFG_KEY_EXT_PLAYER_DB)))
    {
      return false;
    }

    return (!(cfg->operator[](CFG_KEY_EXT_PLAYER_DB).empty()));
  }

  //----------------------------------------------------------------------------

  ExternalPlayerDB*PlayerMngr::getExternalPlayerDatabaseHandle() const
  {
    return ((extPlayerDb == nullptr) ? nullptr : extPlayerDb.get());
  }

  //----------------------------------------------------------------------------

  QString PlayerMngr::getExternalDatabaseName() const
  {
    if (!(hasExternalPlayerDatabaseConfigured()))
    {
      return QString();
    }

    auto cfg = KeyValueTab::getTab(db, TAB_CFG, false);
    if (cfg == nullptr)
    {
      throw std::runtime_error("Config table not found -- this shouldn't happen!");
    }
    return QString::fromUtf8(cfg->operator[](CFG_KEY_EXT_PLAYER_DB).data());
  }

  //----------------------------------------------------------------------------

  ERR PlayerMngr::setExternalPlayerDatabase(const QString& fname, bool createNew)
  {
    upExternalPlayerDB extDb;

    if (fname.isEmpty()) return EPD__INVALID_DATABASE_NAME;

    // try to create the new database
    if (createNew)
    {
      extDb = ExternalPlayerDB::createNew(fname);
      if (extDb == nullptr) return EPD__CREATION_FAILED;
    }
    // try to open an existing database
    else
    {
      extDb = ExternalPlayerDB::openExisting(fname);
      if (extDb == nullptr) return EPD__NOT_FOUND;
    }

    // close the old database, if open
    closeExternalPlayerDatabase();

    // store the pointer to the new database and
    // overwrite the database config key with
    // the new filename
    //
    // do this only if the database name actually changed
    extPlayerDb = std::move(extDb);
    auto cfg = KeyValueTab::getTab(db, TAB_CFG, false);
    if (cfg == nullptr)
    {
      throw std::runtime_error("Config table not found -- this shouldn't happen!");
    }
    string _oldFileName = (*cfg)[CFG_KEY_EXT_PLAYER_DB];
    QString oldFileName = QString::fromUtf8(_oldFileName.data());
    if (oldFileName != fname)
    {
      cfg->set(CFG_KEY_EXT_PLAYER_DB, fname.toUtf8().constData());
    }

    CentralSignalEmitter::getInstance()->externalPlayerDatabaseChanged();

    return OK;
  }

  //----------------------------------------------------------------------------

  ERR PlayerMngr::openConfiguredExternalPlayerDatabase()
  {
    if (!(hasExternalPlayerDatabaseConfigured()))
    {
      return EPD__NOT_CONFIGURED;
    }

    auto cfg = KeyValueTab::getTab(db, TAB_CFG, false);
    if (cfg == nullptr)
    {
      throw std::runtime_error("Config table not found -- this shouldn't happen!");
    }
    QString playerDbName = QString::fromUtf8(cfg->operator[](CFG_KEY_EXT_PLAYER_DB).data());

    return setExternalPlayerDatabase(playerDbName, false);
  }

  //----------------------------------------------------------------------------

  void PlayerMngr::closeExternalPlayerDatabase()
  {
    if (extPlayerDb == nullptr) return;

    extPlayerDb->close();

    // this operation automatically calls the destructor
    // of the underlying database object
    extPlayerDb.reset(nullptr);

    CentralSignalEmitter::getInstance()->externalPlayerDatabaseChanged();
  }

  //----------------------------------------------------------------------------

  unique_ptr<Player> PlayerMngr::importPlayerFromExternalDatabase(ERR* err, int extPlayerId, SEX sexOverride)
  {
    // if no player database has been opened by this instance of the PlayerMngr,
    // try to open the database
    if (extPlayerDb == nullptr)
    {
      ERR e = openConfiguredExternalPlayerDatabase();
      if (e != OK)
      {
        if (err != nullptr)
        {
          *err = e;
        }
        return nullptr;
      }
    }

    // check the ID's validity
    auto extPlayer = extPlayerDb->getPlayer(extPlayerId);
    if (extPlayer == nullptr)
    {
      if (err != nullptr)
      {
        *err = INVALID_ID;
      }
    }

    // TODO: remove this function?
    if (err != nullptr)
    {
      *err = OK;
    }

    return nullptr;

  }

  //----------------------------------------------------------------------------

  ERR PlayerMngr::exportPlayerToExternalDatabase(int playerId)
  {
    auto p = getPlayer_up(playerId);
    if (p == nullptr)
    {
      return INVALID_ID;
    }

    return exportPlayerToExternalDatabase(*p);
  }

  //----------------------------------------------------------------------------

  ERR PlayerMngr::exportPlayerToExternalDatabase(const Player& p)
  {
    // if no player database has been opened by this instance of the PlayerMngr,
    // try to open the database
    if (extPlayerDb == nullptr)
    {
      ERR err = openConfiguredExternalPlayerDatabase();
      if (err != OK)
      {
        return err;
      }
    }

    auto extPlayer = extPlayerDb->getPlayer(p.getFirstName(), p.getLastName());
    if (extPlayer == nullptr)
    {
      // create a new player
      ExternalPlayerDatabaseEntry entry{p.getFirstName(), p.getLastName(), p.getSex()};
      auto newPlayer = extPlayerDb->storeNewPlayer(entry);

      return (newPlayer == nullptr) ? EPD__CREATION_FAILED : OK;
    }

    // update existing player, if applicable
    if (extPlayer->getSex() == DONT_CARE)
    {
      extPlayerDb->updatePlayerSexIfUndefined(extPlayer->getId(), p.getSex());
    }

    return OK;
  }

  //----------------------------------------------------------------------------

  ERR PlayerMngr::syncAllPlayersToExternalDatabase()
  {
    // if no player database has been opened by this instance of the PlayerMngr,
    // try to open the database
    if (extPlayerDb == nullptr)
    {
      ERR err = openConfiguredExternalPlayerDatabase();
      if (err != OK)
      {
        return err;
      }
    }

    for (const Player& p : getAllPlayers())
    {
      exportPlayerToExternalDatabase(p);
    }

    return OK;
  }

  //----------------------------------------------------------------------------

  string PlayerMngr::getSyncString(vector<int> rows)
  {
    vector<string> cols = {"id", PL_FNAME, PL_LNAME, GENERIC_STATE_FIELD_NAME, PL_SEX, PL_REFEREE_COUNT, PL_TEAM_REF};

    return db->getSyncStringForTable(TAB_PLAYER, cols, rows);
  }

  //----------------------------------------------------------------------------

  string PlayerMngr::getSyncString_P2C(vector<int> rows)
  {
    vector<string> cols = {"id", P2C_PLAYER_REF, P2C_CAT_REF};

    return db->getSyncStringForTable(TAB_P2C, cols, rows);
  }

  //----------------------------------------------------------------------------

  string PlayerMngr::getSyncString_Pairs(vector<int> rows)
  {
    vector<string> cols = {"id", PAIRS_PLAYER1_REF, PAIRS_PLAYER2_REF, PAIRS_CAT_REF, PAIRS_GRP_NUM, PAIRS_INITIAL_RANK};

    return db->getSyncStringForTable(TAB_PAIRS, cols, rows);
  }

  //----------------------------------------------------------------------------

  ERR PlayerMngr::canDeletePlayer(const Player &p) const
  {
    // first check: see if we can remove the player from all categories
    auto assignedCats = p.getAssignedCategories();
    for (Category c : assignedCats)
    {
      if (!(c.canRemovePlayer(p)))
      {
        return PLAYER_NOT_REMOVABLE_FROM_CATEGORY;
      }
    }

    // second check: ensure that there are not existing / planned
    // matches that involve this player
    //
    // step 1: get all player pairs that involve this player
    QString where = "%1 = %2 OR %3 = %2";
    where = where.arg(PAIRS_PLAYER1_REF).arg(p.getId()).arg(PAIRS_PLAYER2_REF);
    DbTab* pairTab = db->getTab(TAB_PAIRS);
    PlayerPairList assignedPairs = getObjectsByWhereClause<PlayerPair>(pairTab, where.toUtf8().constData());

    // step 2: check if we have any matches that involve one of these pairs
    DbTab* matchTab = db->getTab(TAB_MATCH);
    for (PlayerPair pp : assignedPairs)
    {
      int ppId = pp.getPairId();
      if (ppId < 0) continue;  // player pair in a not-yet-started category
      where = "%1 = %2 OR %3 = %2";
      where = where.arg(MA_PAIR1_REF).arg(ppId).arg(MA_PAIR2_REF);
      MatchList maList = getObjectsByWhereClause<Match>(matchTab, where.toUtf8().constData());
      if (!(maList.empty()))
      {
        return PLAYER_ALREADY_IN_MATCHES;
      }
    }

    // third check: the player may not be referenced as an actual player
    // (e.g., as a substitue) in any match
    where = "%1 = %2 OR %3 = %2 OR %4 = %2 OR %5 = %2";
    where = where.arg(MA_ACTUAL_PLAYER1A_REF).arg(p.getId());
    where = where.arg(MA_ACTUAL_PLAYER1B_REF).arg(MA_ACTUAL_PLAYER2A_REF).arg(MA_ACTUAL_PLAYER2B_REF);
    MatchList maList = getObjectsByWhereClause<Match>(matchTab, where.toUtf8().constData());
    if (!(maList.empty()))
    {
      return PLAYER_ALREADY_IN_MATCHES;
    }

    // at this point, we have pairs but no matches yet. this means
    // that the player is assigned to a doubles/mixed partner in a
    // not yet started category
    if (!(assignedPairs.empty()))
    {
      return PLAYER_ALREADY_PAIRED;
    }

    return OK;
  }

  //----------------------------------------------------------------------------

  ERR PlayerMngr::deletePlayer(const Player &p) const
  {
    ERR e = canDeletePlayer(p);
    if (e != OK)
    {
      return e;
    }

    // remove the player from all categories
    auto assignedCats = p.getAssignedCategories();
    for (Category c : assignedCats)
    {
      e = c.removePlayer(p);
      if (e != OK)
      {
        return e;   // after the checks before, this shoudln't happen
      }
    }

    // there is nothing more to do for us, because there are no more references
    // to this player. This has been checked by canDeletePlayer() before

    // lock the database before writing
    DbLockHolder lh{db, DatabaseAccessRoles::MainThread};

    // the actual deletion
    int oldSeqNum = p.getSeqNum();
    CentralSignalEmitter* cse = CentralSignalEmitter::getInstance();
    cse->beginDeletePlayer(oldSeqNum);
    tab->deleteRowsByColumnValue("id", p.getId());
    fixSeqNumberAfterDelete(tab, oldSeqNum);
    cse->endDeletePlayer();

    return OK;
  }

  //----------------------------------------------------------------------------

  int PlayerMngr::getTotalPlayerCount() const
  {
    return tab->length();
  }

  //----------------------------------------------------------------------------

  void PlayerMngr::getRecentFinishers(int maxCnt, PlayerPairList& winners_out, PlayerPairList& losers_out, PlayerPairList& draw_out) const
  {
    if (maxCnt < 0) return;

    // search for up to maxCnt recently finished matches
    WhereClause wc;
    DbTab* matchTab = db->getTab(TAB_MATCH);
    wc.addIntCol(GENERIC_STATE_FIELD_NAME, static_cast<int>(STAT_MA_FINISHED));
    wc.setOrderColumn_Desc(MA_FINISH_TIME);
    wc.setLimit(maxCnt);
    MatchList ml = getObjectsByWhereClause<Match>(matchTab, wc);

    // extract the winners and losers from these matches
    for (const Match& ma : ml)
    {
      auto loser = ma.getLoser();
      auto winner = ma.getWinner();

      if (loser != nullptr) losers_out.push_back(*loser);
      if (winner != nullptr) winners_out.push_back(*winner);

      // handle draws
      if ((loser == nullptr) && (winner == nullptr))
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

  upMatch PlayerMngr::getLastFinishedMatchForPlayer(const Player& p)
  {
    QString sql = "SELECT id FROM %1 WHERE %2=%3 OR %4=%3 OR %5=%3 OR %6=%3 ORDER BY %7 DESC LIMIT 1";
    sql = sql.arg(TAB_MATCH);
    sql = sql.arg(MA_ACTUAL_PLAYER1A_REF);
    sql = sql.arg(p.getId());
    sql = sql.arg(MA_ACTUAL_PLAYER1B_REF);
    sql = sql.arg(MA_ACTUAL_PLAYER2A_REF);
    sql = sql.arg(MA_ACTUAL_PLAYER2B_REF);
    sql = sql.arg(MA_FINISH_TIME);

    int matchId;
    bool isOk = db->execScalarQueryInt(sql.toUtf8().constData(), &matchId, nullptr);
    if (!isOk) return nullptr;  // no match found

    MatchMngr mm{db};
    return mm.getMatch(matchId);
  }

  //----------------------------------------------------------------------------

  upMatch PlayerMngr::getNextMatchForPlayer(const Player& p)
  {
    vector<Match> allNextMatches = getAllScheduledMatchesForPlayer(p, true);
    if (allNextMatches.empty()) return nullptr;

    MatchMngr mm{db};
    return mm.getMatch(allNextMatches.at(0).getId());
  }

  //----------------------------------------------------------------------------

  vector<Match> PlayerMngr::getAllScheduledMatchesForPlayer(const Player &p, bool findFirstOnly)
  {
    vector<Match> result;

    // get all scheduled matches
    QString where = "%1 > 0 AND %2 != %3 AND %2 != %4 ORDER BY %5 ASC";
    where = where.arg(MA_NUM);
    where = where.arg(GENERIC_STATE_FIELD_NAME);
    where = where.arg(static_cast<int>(STAT_MA_RUNNING));
    where = where.arg(static_cast<int>(STAT_MA_FINISHED));
    where = where.arg(MA_NUM);

    DbTab* matchTab = db->getTab(TAB_MATCH);
    MatchMngr mm{db};
    for (const Match& ma : getObjectsByWhereClause<Match>(matchTab, where.toUtf8().constData()))
    {
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

      // stop the loop if we only need one match
      // (okay, since we use "continue" above, we make at least one
      // extra cycle after the first match before we hit this "if")
      if (findFirstOnly && (!(result.empty()))) return result;
    }

    return result;
  }

  //----------------------------------------------------------------------------

  void PlayerMngr::increaseRefereeCountForPlayer(const Player& p)
  {
    int oldCount = p.getRefereeCount();

    // lock the database before writing
    DbLockHolder lh{db, DatabaseAccessRoles::MainThread};

    p.row.update(PL_REFEREE_COUNT, oldCount + 1);
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
