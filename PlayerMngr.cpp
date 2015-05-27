/* 
 * File:   TeamMngr.cpp
 * Author: nyoknvk1
 * 
 * Created on 18. Februar 2014, 14:04
 */

#include "PlayerMngr.h"
#include "Player.h"
#include "TournamentErrorCodes.h"
#include "TournamentDataDefs.h"
#include <stdexcept>
#include <QtCore/qdebug.h>
#include <QList>
#include "HelperFunc.h"
#include "TeamMngr.h"
#include "Tournament.h"

using namespace dbOverlay;

namespace QTournament
{

  PlayerMngr::PlayerMngr(TournamentDB* _db)
  : GenericObjectManager(_db), playerTab((*db)[TAB_PLAYER])
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
    QVariantList qvl;
    qvl << PL_FNAME << first;
    qvl << PL_LNAME << last;
    qvl << PL_SEX << static_cast<int>(sex);
    qvl << GENERIC_STATE_FIELD_NAME << static_cast<int>(STAT_PL_IDLE);
    
    // set the team reference, if applicable
    if (cfg.getBool(CFG_KEY_USE_TEAMS))
    {
      if (teamName.isEmpty())
      {
        return INVALID_TEAM;
      }
      
      TeamMngr* tm = Tournament::getTeamMngr();
      if (!(tm->hasTeam(teamName)))
      {
        return INVALID_TEAM;
      }
      
      Team t = tm->getTeam(teamName);
      qvl << PL_TEAM_REF << t.getId();
    }
    
    // create the new player row
    emit beginCreatePlayer();
    playerTab.insertRow(qvl);
    fixSeqNumberAfterInsert(TAB_PLAYER);
    emit endCreatePlayer(playerTab.length() - 1); // the new sequence number is always the greatest
    
    return OK;
  }

//----------------------------------------------------------------------------

  bool PlayerMngr::hasPlayer(const QString& firstName, const QString& lastName)
  {
    QVariantList qvl;
    qvl << PL_FNAME << firstName;
    qvl << PL_LNAME << lastName;
    
    return (playerTab.getMatchCountForColumnValue(qvl) > 0);
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
      throw std::invalid_argument("The player '" + QString2String(firstName + " " + lastName) + "' does not exist");
    }
    
    QVariantList qvl;
    qvl << PL_FNAME << firstName;
    qvl << PL_LNAME << lastName;
    TabRow r = playerTab.getSingleRowByColumnValue(qvl);
    
    return Player(db, r);
  }

//----------------------------------------------------------------------------

  /**
   * Returns a list of all players
   *
   * @Return QList holding all Teams
   */
  QList<Player> PlayerMngr::getAllPlayers()
  {
    QList<Player> result;
    
    DbTab::CachingRowIterator it = playerTab.getAllRows();
    while (!(it.isEnd()))
    {
      result << Player(db, *it);
      ++it;
    }
    
    return result;
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
    
    QVariantList qvl;
    qvl << PL_FNAME << newFirst;
    qvl << PL_LNAME << newLast;
    p.row.update(qvl);
    
    emit playerRenamed(p);
    
    return OK;
  }

//----------------------------------------------------------------------------

  /**
   * Returns a database object for a player identified by its sequence number
   *
   * Note: the player must exist, otherwise this method throws an exception!
   *
   * @param seqNum is the sequence number of the player to look up
   *
   * @return a Player instance of that player
   */
  Player PlayerMngr::getPlayerBySeqNum(int seqNum)
  {
    try {
      TabRow r = playerTab.getSingleRowByColumnValue(GENERIC_SEQNUM_FIELD_NAME, seqNum);
      return Player(db, r);
    }
    catch (std::exception e)
    {
     throw std::invalid_argument("The player with sequence number " + QString2String(QString::number(seqNum)) + " does not exist");
    }
  }


//----------------------------------------------------------------------------

  bool PlayerMngr::hasPlayer(int id)
  {
    try
    {
      TabRow r = playerTab[id];
      return true;
    }
    catch (std::exception e)
    {
      
    }
    return false;
  }

//----------------------------------------------------------------------------

  Player PlayerMngr::getPlayer(int id)
  {
    // this public function essentially short-circuits the private player()-constructor... Hmmm...
    return Player(db, id);
  }

//----------------------------------------------------------------------------

  PlayerPair PlayerMngr::getPlayerPair(int id)
  {
    TabRow r = (*db)[TAB_PAIRS][id];
    
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
  }

//----------------------------------------------------------------------------

  ERR PlayerMngr::canAcquirePlayerPairsForMatch(const Match& ma)
  {
    PlayerList pl = determineActualPlayersForMatch(ma);

    for (Player p : pl)
    {
      if (p.getState() != STAT_PL_IDLE) return PLAYER_NOT_IDLE;
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
    for (Player p : pl)
    {
      OBJ_STATE oldStat = p.getState();
      TabRow r = playerTab[p.getId()];
      r.update(GENERIC_STATE_FIELD_NAME, STAT_PL_PLAYING);
      emit playerStatusChanged(p.getId(), p.getSeqNum(), oldStat, STAT_PL_PLAYING);
    }

    return OK;
  }

//----------------------------------------------------------------------------

  ERR PlayerMngr::releasePlayerPairsAfterMatch(const Match& ma)
  {
    PlayerList pl = determineActualPlayersForMatch(ma);

    // update all player states back to idle
    for (Player p : pl)
    {
      TabRow r = playerTab[p.getId()];
      r.update(GENERIC_STATE_FIELD_NAME, STAT_PL_IDLE);
      emit playerStatusChanged(p.getId(), p.getSeqNum(), STAT_PL_PLAYING, STAT_PL_IDLE);
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
    PlayerMngr* pm = Tournament::getPlayerMngr();

    // have "actual players" already been assigned?
    // if yes, return those values. They overrule everything else
    TabRow matchRow = (db->getTab(TAB_MATCH))[ma.getId()];
    QVariant pRef = matchRow[MA_ACTUAL_PLAYER1A_REF];
    if (!(pRef.isNull()))
    {
      result << pm->getPlayer(pRef.toInt());

      pRef = matchRow[MA_ACTUAL_PLAYER1B_REF];
      if (!(pRef.isNull())) result << pm->getPlayer(pRef.toInt());

      pRef = matchRow[MA_ACTUAL_PLAYER2A_REF];
      if (!(pRef.isNull())) result << pm->getPlayer(pRef.toInt());  // should always be true, since we have a valid player1a

      pRef = matchRow[MA_ACTUAL_PLAYER2B_REF];
      if (!(pRef.isNull())) result << pm->getPlayer(pRef.toInt());

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
    result << pp1.getPlayer1();
    result << pp2.getPlayer1();
    if (pp1.hasPlayer2()) result << pp1.getPlayer2();
    if (pp2.hasPlayer2()) result << pp2.getPlayer2();

    //
    // TODO:
    // Here, a potential subsitution algorithm could kick in
    // and replace the "theoretical players" with "actual players", if applicable
    //

    return result;
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

  ERR PlayerMngr::canDeletePlayer(const Player &p) const
  {
    // first check: see if we can remove the player from all categories
    CatMngr* cm = Tournament::getCatMngr();
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
    DbTab pairTab = (*db)[TAB_PAIRS];
    PlayerPairList assignedPairs = getObjectsByWhereClause<PlayerPair>(pairTab, where);
    // note: this list SHOULD be empty, because otherwise we shouldn't be able
    // to remove the player from the categories (see above)

    // step 2: check if we have any matches that involve one of these pairs
    //
    // TODO: skipped this test in favour of an assertion
    // that there no player pairs at all
    //assert(assignedPairs.isEmpty());
    if (!(assignedPairs.isEmpty()))
    {
      return PLAYER_ALREADY_IN_MATCHES;
    }

    // third check: the player may not be referenced as an actual player
    // (e.g., as a substitue) in any match
    where = "%1 = %2 OR %3 = %2 OR %4 = %2 OR %5 = %2";
    where = where.arg(MA_ACTUAL_PLAYER1A_REF).arg(p.getId());
    where = where.arg(MA_ACTUAL_PLAYER1B_REF).arg(MA_ACTUAL_PLAYER2A_REF).arg(MA_ACTUAL_PLAYER2B_REF);
    DbTab matchTab = (*db)[TAB_MATCH];
    MatchList maList = getObjectsByWhereClause<Match>(matchTab, where);
    if (!(maList.isEmpty()))
    {
      return PLAYER_ALREADY_IN_MATCHES;
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
    CatMngr* cm = Tournament::getCatMngr();
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

    // the actual deletion
    int oldSeqNum = p.getSeqNum();
    emit beginDeletePlayer(oldSeqNum);
    playerTab.deleteRowsByColumnValue("id", p.getId());
    fixSeqNumberAfterDelete(TAB_PLAYER, oldSeqNum);
    emit endDeletePlayer();

    return OK;
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
