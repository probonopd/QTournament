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
#include <QtCore/qdebug.h>
#include <QtCore/qjsonarray.h>
#include <QList>

#include <SqliteOverlay/Transaction.h>

#include "HelperFunc.h"
#include "Category.h"
#include "Player.h"
#include "KO_Config.h"
#include "CatRoundStatus.h"
#include "CatMngr.h"
#include "TournamentErrorCodes.h"
#include "TournamentDataDefs.h"
#include "TournamentDB.h"
#include "HelperFunc.h"
#include "CentralSignalEmitter.h"
#include "MatchMngr.h"
#include "PlayerMngr.h"

namespace QTournament
{

  CatMngr::CatMngr(TournamentDB* _db)
  : TournamentDatabaseObjectManager(_db, TAB_CATEGORY)
  {
  }

//----------------------------------------------------------------------------

  ERR CatMngr::createNewCategory(const QString& cn)
  {
    QString catName = cn.trimmed();
    
    if (catName.isEmpty())
    {
      return INVALID_NAME;
    }
    
    if (catName.length() > MAX_NAME_LEN)
    {
      return INVALID_NAME;
    }
    
    if (hasCategory(catName))
    {
      return NAME_EXISTS;
    }
    
    // create a new table row and set some arbitrary default data
    SqliteOverlay::ColumnValueClause cvc;
    cvc.addStringCol(GENERIC_NAME_FIELD_NAME, catName.toUtf8().constData());
    cvc.addIntCol(CAT_ACCEPT_DRAW, false);
    cvc.addIntCol(CAT_SYS, static_cast<int>(GROUPS_WITH_KO));
    cvc.addIntCol(CAT_MATCH_TYPE, static_cast<int>(SINGLES));
    cvc.addIntCol(CAT_SEX, static_cast<int>(M));
    cvc.addIntCol(GENERIC_STATE_FIELD_NAME, static_cast<int>(STAT_CAT_CONFIG));
    cvc.addIntCol(CAT_WIN_SCORE, 2);
    cvc.addIntCol(CAT_DRAW_SCORE, 1);
    cvc.addStringCol(CAT_GROUP_CONFIG, KO_Config(QUARTER, false).toString().toUtf8().constData());
    
    CentralSignalEmitter* cse = CentralSignalEmitter::getInstance();
    cse->beginCreateCategory();
    tab->insertRow(cvc);
    fixSeqNumberAfterInsert();
    cse->endCreateCategory(tab->length() - 1); // the new sequence number is always the greatest
    
    return OK;
  }

  //----------------------------------------------------------------------------

  ERR CatMngr::cloneCategory(const Category& src, const QString& catNamePostfix)
  {
    if (catNamePostfix.isEmpty())
    {
      return INVALID_NAME;
    }

    // set an arbitrarily chosen maximum of 10 characters for the postfix
    if (catNamePostfix.length() > 10)
    {
      return INVALID_NAME;
    }

    // try create a new category until we've found a valid name
    int cnt = 0;
    ERR err;
    QString dstCatName;
    do
    {
      ++cnt;

      // create the clone name as a combination of source name, a postfix
      // and a number. Make sure the destination name does not exceed the
      // maximum name length
      QString trimmedSrcCatName = src.getName();
      do
      {
        dstCatName = trimmedSrcCatName + " - " + catNamePostfix + " " + QString::number(cnt);
        trimmedSrcCatName.chop(1);
      } while (dstCatName.length() > MAX_NAME_LEN);

      // try to actually create the category
      err = createNewCategory(dstCatName);
    } while ((err == NAME_EXISTS) && (cnt < 100));   // a maximum limit of 100 retries

    // did we succeed?
    if (err != OK)
    {
      return err;   // give up
    }
    Category clone = getCategory(dstCatName);

    // copy the settings fromt the source category to clone
    err = clone.setMatchSystem(src.getMatchSystem());
    assert(err == OK);
    err = clone.setMatchType(src.getMatchType());
    assert(err == OK);
    err = clone.setSex(src.getSex());
    assert(err == OK);
    bool isOk = setCatParam_AllowDraw(clone, src.getParameter_bool(ALLOW_DRAW));
    assert(isOk);
    isOk = setCatParam_Score(clone, src.getParameter_int(WIN_SCORE), false);
    assert(isOk);
    setCatParam_Score(clone, src.getParameter_int(DRAW_SCORE), true);  // no assert here; setting draw score may fail if draw is not allowed
    KO_Config ko{src.getParameter_string(GROUP_CONFIG)};
    isOk = clone.setParameter(GROUP_CONFIG, ko.toString());
    assert(isOk);
    setCatParameter(clone, ROUND_ROBIN_ITERATIONS, src.getParameter_int(ROUND_ROBIN_ITERATIONS));

    // Do not copy the BracketVisData here, because the clone is still in
    // CONFIG and BracketVisData is created when starting the cat

    // assign the players to the category
    for (const Player& pl : src.getAllPlayersInCategory())
    {
      err = addPlayerToCategory(pl, clone);
      if (err != OK)
      {
        return err;   // shouldn't happen
      }
    }

    // pair players, if applicable
    if (src.getMatchType() != SINGLES)
    {
      for (const PlayerPair& pp : src.getPlayerPairs())
      {
        if (!(pp.hasPlayer2())) continue;

        err = pairPlayers(clone, pp.getPlayer1(), pp.getPlayer2());
        if (err != OK)
        {
          return err;   // shouldn't happen
        }
      }
    }

    return OK;
  }

//----------------------------------------------------------------------------

  bool CatMngr::hasCategory(const QString& catName) const
  {
    return (tab->getMatchCountForColumnValue(GENERIC_NAME_FIELD_NAME, catName.toUtf8().constData()) > 0);
  }

//----------------------------------------------------------------------------

  /**
   * Returns a database object for a category identified by its name
   *
   * Note: the category must exist, otherwise this method throws an exception!
   *
   * @param name is the name of the category to look up
   *
   * @return a Category instance of that team
   */
  Category CatMngr::getCategory(const QString& name)
  {
    if (!(hasCategory(name)))
    {
      throw std::invalid_argument("The category '" + QString2StdString(name) + "' does not exist");
    }
    
    SqliteOverlay::TabRow r = tab->getSingleRowByColumnValue(GENERIC_NAME_FIELD_NAME, name.toUtf8().constData());
    
    return Category(db, r);
  }

//----------------------------------------------------------------------------

  unique_ptr<Category> CatMngr::getCategory(int id)
  {
    return getSingleObjectByColumnValue<Category>("id", id);
  }

//----------------------------------------------------------------------------

  /**
   * Returns a list of all categories
   *
   * @Return QList holding all categories
   */
  CategoryList CatMngr::getAllCategories()
  {
    return getAllObjects<Category>();
  }

//----------------------------------------------------------------------------

  ERR CatMngr::setMatchSystem(Category& c, MATCH_SYSTEM s)
  {
    if (c.getState() != STAT_CAT_CONFIG)
    {
      return CATEGORY_NOT_CONFIGURALE_ANYMORE;
    }

    // TODO: implement checks, updates to other tables etc
    int sysInt = static_cast<int>(s);
    c.row.update(CAT_SYS, sysInt);
    
    // if we switch to single elimination categories or
    // to the ranking system, we want to
    // prevent draw
    if ((s == SINGLE_ELIM) || (s == RANKING))
    {
      setCatParam_AllowDraw(c, false);
    }

    return OK;
  }

//----------------------------------------------------------------------------

  ERR CatMngr::setMatchType(Category& c, MATCH_TYPE t)
  {
    // we can only change the match type while being in config mode
    if (c.getState() != STAT_CAT_CONFIG)
    {
      return CATEGORY_NOT_CONFIGURALE_ANYMORE;
    }
    
    // temporarily store all existing player pairs
    PlayerPairList pairList = c.getPlayerPairs();
    
    // check if we can split all pairs
    bool canSplit = true;
    for (const PlayerPair& pp : pairList)
    {
      if (!(pp.hasPlayer2())) continue;
      
      if (c.canSplitPlayers(pp.getPlayer1(), pp.getPlayer2()) != OK)
      {
        canSplit = false;
      }
    }
    if (!canSplit)
    {
      return INVALID_RECONFIG;  // if only one pair can't be split, refuse to change the match type
    }
    
    // actually split all pairs
    for (const PlayerPair& pp : pairList)
    {
      if (!(pp.hasPlayer2())) continue;
      
      splitPlayers(c, pp.getPlayer1(), pp.getPlayer2());
    }
    
    // if we come from mixed, we force the sex type "Don't care" to avoid
    // that too many players will be removed (unwanted) from the category
    MATCH_TYPE oldType = c.getMatchType();
    if (oldType == MIXED)
    {
      setSex(c, DONT_CARE);   // no error checking here, setting "don't care" should always work because it's least restrictive
    }
    
    // change the match type
    int typeInt = static_cast<int>(t);
    c.row.update(CAT_MATCH_TYPE, typeInt);
    
    // try to recreate as many pairs as possible
    for (const PlayerPair& pp : pairList)
    {
      if (!(pp.hasPlayer2())) continue;
      
      pairPlayers(c, pp.getPlayer1(), pp.getPlayer2());
    }
    
    // IMPORTANT:
    // THIS OPERATION CHANGED THE PAIR-IDs OF ALL PAIRS!
    
    return OK;
  }

//----------------------------------------------------------------------------

  ERR CatMngr::setSex(Category& c, SEX s)
  {
    // we can only change the sex while being in config mode
    if (c.getState() != STAT_CAT_CONFIG)
    {
      return CATEGORY_NOT_CONFIGURALE_ANYMORE;
    }
    
    // unless we switch to "don't care", we have to make sure that
    // we can remove the wrong players
    if ((s != DONT_CARE) && (c.getMatchType() != MIXED))
    {
      PlayerList allPlayers = c.getAllPlayersInCategory();
      for (const Player& p : allPlayers)
      {
        // skip players with matching sex
        if (p.getSex() == s) continue;

        // for all other players, check if we can remove them
        if (!(c.canRemovePlayer(p)))
        {
          return INVALID_RECONFIG;
        }
      }
      
      // okay, we can be sure that all "unwanted" players can be removed.
      // do it.
      for (const Player& p : allPlayers)
      {
        // skip players with matching sex
        if (p.getSex() == s) continue;

        // for all other players, check if we can remove them
        c.removePlayer(p);
      }
    }
    
    // if we de-active "don't care" in a mixed category, we have to split
    // all non-compliant pairs. The players itself can remain in the category
    if ((s != DONT_CARE) && (c.getMatchType() == MIXED)) {
      PlayerPairList allPairs = c.getPlayerPairs();
      
      for (const PlayerPair& pp : allPairs) {

        // Skip unpaired players
        if (!(pp.hasPlayer2())) continue;

        // Skip true mixed pairs
        if (pp.getPlayer1().getSex() != pp.getPlayer2().getSex()) continue;

        // check if we can split "false" mixed pairs (= same sex pairs)
        if (c.canSplitPlayers(pp.getPlayer1(), pp.getPlayer2()) != OK) {
          return INVALID_RECONFIG;
        }
      }
      
      // now we can be sure that all unwanted pairs can be split
      for (const PlayerPair& pp : allPairs) {

        // Skip unpaired players
        if (!(pp.hasPlayer2())) continue;

        // Skip true mixed pairs
        if (pp.getPlayer1().getSex() != pp.getPlayer2().getSex()) continue;

        // check if we can split "false" mixed pairs (= same sex pairs)
        splitPlayers(c, pp.getPairId());
      }
    }
    
    // execute the actual change
    int sexInt = static_cast<int>(s);
    c.row.update(CAT_SEX, sexInt);
    
    return OK;
  }

//----------------------------------------------------------------------------

  ERR CatMngr::addPlayerToCategory(const Player& p, const Category& c)
  {
    if (!(c.canAddPlayers()))
    {
      return CATEGORY_CLOSED_FOR_MORE_PLAYERS;
    }
    
    if (c.hasPlayer(p))
    {
      return PLAYER_ALREADY_IN_CATEGORY;
    }
    
    if (c.getAddState(p) != CAN_JOIN)
    {
      return PLAYER_NOT_SUITABLE;
    }
    
    // TODO: check that player is not permanently disabled
    
    // actually add the player
    SqliteOverlay::ColumnValueClause cvc;
    cvc.addIntCol(P2C_CAT_REF, c.getId());
    cvc.addIntCol(P2C_PLAYER_REF, p.getId());
    db->getTab(TAB_P2C)->insertRow(cvc);
    
    CentralSignalEmitter::getInstance()->playerAddedToCategory(p, c);
    
    return OK;
  }

//----------------------------------------------------------------------------

  ERR CatMngr::removePlayerFromCategory(const Player& p, const Category& c) const
  {
    if (!(c.canRemovePlayer(p)))
    {
      return PLAYER_NOT_REMOVABLE_FROM_CATEGORY;
    }
    
    if (!(c.hasPlayer(p)))
    {
      return PLAYER_NOT_IN_CATEGORY;
    }
    
    if (c.isPaired(p))
    {
      Player partner = c.getPartner(p);
      ERR e = splitPlayers(c, p, partner);
      if (e != OK)
      {
        return e;
      }
    }
    
    // And: unless we play with random partners, the double partner
    // has to be removed from the category as well if we're beyond
    // "Category Configuration" state
    
    // actually delete the assignment
    SqliteOverlay::WhereClause wc;
    wc.addIntCol(P2C_CAT_REF, c.getId());
    wc.addIntCol(P2C_PLAYER_REF, p.getId());
    int cnt = db->getTab(TAB_P2C)->deleteRowsByWhereClause(wc);
    assert(cnt == 1);
    
    CentralSignalEmitter::getInstance()->playerRemovedFromCategory(p, c);
    
    return OK;
  }

  //----------------------------------------------------------------------------

  ERR CatMngr::deleteCategory(const Category& cat) const
  {
    ERR e = canDeleteCategory(cat);
    if (e != OK) return e;

    // remove all players from the category
    PlayerList allPlayers = cat.getAllPlayersInCategory();
    for (const Player& pl : allPlayers)
    {
      e = removePlayerFromCategory(pl, cat);
      if (e != OK)
      {
        return e;   // after all the checks before, this shouldn't happen
      }
    }

    // a few checks for the cowards
    int catId = cat.getId();
    assert(db->getTab(TAB_P2C)->getMatchCountForColumnValue(P2C_CAT_REF, catId) == 0);
    assert(db->getTab(TAB_PAIRS)->getMatchCountForColumnValue(PAIRS_CAT_REF, catId) == 0);
    assert(db->getTab(TAB_MATCH_GROUP)->getMatchCountForColumnValue(MG_CAT_REF, catId) == 0);
    assert(db->getTab(TAB_RANKING)->getMatchCountForColumnValue(RA_CAT_REF, catId) == 0);
    assert(db->getTab(TAB_BRACKET_VIS)->getMatchCountForColumnValue(BV_CAT_REF, catId) == 0);

    // the actual deletion
    int oldSeqNum = cat.getSeqNum();
    CentralSignalEmitter* cse = CentralSignalEmitter::getInstance();
    cse->beginDeleteCategory(oldSeqNum);
    tab->deleteRowsByColumnValue("id", catId);
    fixSeqNumberAfterDelete(tab, oldSeqNum);
    cse->endDeleteCategory();

    return OK;
  }

  //----------------------------------------------------------------------------

  ERR CatMngr::deleteRunningCategory(const Category& cat) const
  {
    // only one initial check: can we do it "the soft way"?
    if (canDeleteCategory(cat) == OK)
    {
      return deleteCategory(cat);
    }

    // no, we can't.


    // this is a brute-force deletion of an already running category.
    //
    // we don't perform any further pre-checks, because we directly delete everything,
    // no matter what

    // Step 1: undo calls for running matches in this category
    MatchMngr mm{db};
    auto runningMatches = mm.getCurrentlyRunningMatches();
    for (const Match& ma : runningMatches)
    {
      if (ma.getCategory() != cat) continue;

      ERR err = mm.undoMatchCall(ma);
      if (err != OK) return err;   // shouldn't happen
    }

    // step 2: un-stage all staged match groups of this category
    auto stagedMatchGroups = mm.getStagedMatchGroupsOrderedBySequence();
    for (const MatchGroup& mg : stagedMatchGroups)
    {
      if (mg.getCategory() != cat) continue;

      ERR err = mm.unstageMatchGroup(mg);
      if (err != OK) return err;   // shouldn't happen
    }

    // step 3: tell everyone that something baaaad is about to happen
    CentralSignalEmitter* cse = CentralSignalEmitter::getInstance();
    cse->beginResetAllModels();

    //
    // now the actual deletion starts
    //
    bool isDbErr;
    auto tg = db->acquireTransactionGuard(false, &isDbErr);
    if (isDbErr) return DATABASE_ERROR;

    int catId = cat.getId();

    // deletion 1: bracket vis data, because it has only outgoing refs
    int dbErr;
    auto t = db->getTab(TAB_BRACKET_VIS);
    t->deleteRowsByColumnValue(BV_CAT_REF, catId, &dbErr);
    if (dbErr != SQLITE_DONE) return DATABASE_ERROR;  // implicit rollback through tg's dtor

    // deletion 2: ranking data, because it has only outgoing refs
    t = db->getTab(TAB_RANKING);
    t->deleteRowsByColumnValue(RA_CAT_REF, catId, &dbErr);
    if (dbErr != SQLITE_DONE) return DATABASE_ERROR;  // implicit rollback through tg's dtor

    // deletion 3a: matches, they are refered to by bracket vis data only
    // deletion 3b: match groups, they are refered to only by ranking data and matches
    t = db->getTab(TAB_MATCH);
    auto mgTab = db->getTab(TAB_MATCH_GROUP);
    for (const MatchGroup& mg : mm.getAllMatchGroups())
    {
      if (mg.getCategory() == cat)
      {
        auto matchesInGroup = mg.getMatches();
        for (const Match& ma : matchesInGroup)
        {
          int deletedSeqNum = ma.getSeqNum();
          t->deleteRowsByColumnValue("id", ma.getId(), &dbErr);
          if (dbErr != SQLITE_DONE) return DATABASE_ERROR;  // implicit rollback through tg's dtor
          fixSeqNumberAfterDelete(t, deletedSeqNum);
        }

        // the match group has incoming links from matches
        // and ranking and both have been deleted by now
        int deletedSeqNum = mg.getSeqNum();
        mgTab->deleteRowsByColumnValue("id", mg.getId(), &dbErr);
        if (dbErr != SQLITE_DONE) return DATABASE_ERROR;  // implicit rollback through tg's dtor
        fixSeqNumberAfterDelete(mgTab, deletedSeqNum);
      }
    }

    // deletion 4: player pairs
    t = db->getTab(TAB_PAIRS);
    t->deleteRowsByColumnValue(PAIRS_CAT_REF, catId, &dbErr);
    if (dbErr != SQLITE_DONE) return DATABASE_ERROR;  // implicit rollback through tg's dtor

    // deletion 5: player to category allocation
    t = db->getTab(TAB_P2C);
    t->deleteRowsByColumnValue(P2C_CAT_REF, catId, &dbErr);
    if (dbErr != SQLITE_DONE) return DATABASE_ERROR;  // implicit rollback through tg's dtor

    // final deletion: the category itself
    int deletedSeqNum = cat.getSeqNum();
    tab->deleteRowsByColumnValue("id", catId, &dbErr);
    if (dbErr != SQLITE_DONE) return DATABASE_ERROR;  // implicit rollback through tg's dtor
    fixSeqNumberAfterDelete(tab, deletedSeqNum);

    //
    // deletion completed
    //
    bool isOkay = tg ? tg->commit() : true;
    if (!isOkay) return DATABASE_ERROR;
    tg.reset();  // explicit deletion, otherwise tg's dtor might interfere with other transactions

    // refresh all models and the reports tab
    cse->endResetAllModels();

    // tell all other widgets that a category has been deleted
    cse->categoryRemovedFromTournament(catId, deletedSeqNum);

    return OK;
  }

//----------------------------------------------------------------------------

  Category CatMngr::getCategoryById(int id)
  {
    try {
      SqliteOverlay::TabRow r = tab->operator [](id);
      return Category(db, r);
      }
    catch (std::exception e)
    {
     throw std::invalid_argument("The category with ID " + to_string(id) + " does not exist");
    }
  }

//----------------------------------------------------------------------------

  Category CatMngr::getCategoryBySeqNum(int seqNum)
  {
    try {
      SqliteOverlay::TabRow r = tab->getSingleRowByColumnValue(GENERIC_SEQNUM_FIELD_NAME, seqNum);
      return Category(db, r);
    }
    catch (std::exception e)
    {
     throw std::invalid_argument("The category with sequence number " + to_string(seqNum) + " does not exist");
    }
  }

//----------------------------------------------------------------------------

  QHash<Category, CAT_ADD_STATE> CatMngr::getAllCategoryAddStates(SEX s)
  {
    CategoryList allCat = getAllCategories();
    QHash<Category, CAT_ADD_STATE> result;
    
    for (Category& c : allCat)
    {
      result[c] = c.getAddState(s);
    }
    
    return result;
  }

//----------------------------------------------------------------------------

  QHash<Category, CAT_ADD_STATE> CatMngr::getAllCategoryAddStates(const Player& p)
  {
    CategoryList allCat = getAllCategories();
    QHash<Category, CAT_ADD_STATE> result;
    
    for (Category& c : allCat)
    {
      result[c] = c.getAddState(p);
    }
    
    return result;
  }

//----------------------------------------------------------------------------

  bool CatMngr::setCatParameter(Category& c, CAT_PARAMETER p, const QVariant& v)
  {
    if (p == ALLOW_DRAW)
    {
      return setCatParam_AllowDraw(c, v);
    }
    if (p == DRAW_SCORE)
    {
      return setCatParam_Score(c, v.toInt(), true);
    }
    if (p == WIN_SCORE)
    {
      return setCatParam_Score(c, v.toInt(), false);
    }
    if (p == GROUP_CONFIG)
    {
      if (c.getState() != STAT_CAT_CONFIG) return false;

      c.row.update(CAT_GROUP_CONFIG, v.toString().toUtf8().constData());
      return true;
    }
    if (p == ROUND_ROBIN_ITERATIONS)
    {
      bool isOk;
      int iterations = v.toInt(&isOk);
      if (!isOk) return false;

      if (iterations <= 0) return false;
      c.row.update(CAT_ROUND_ROBIN_ITERATIONS, iterations);
      return true;
    }
    
    return false;
  }

//----------------------------------------------------------------------------

  bool CatMngr::setCatParam_AllowDraw(Category& c, const QVariant& v)
  {
    if (c.getState() != STAT_CAT_CONFIG)
    {
      return false;
    }
    
    bool allowDraw = v.toBool();
    bool oldState = c.getParameter(ALLOW_DRAW).toBool();

    if (allowDraw == oldState)
    {
      return true; // no change necessary;
    }

    // no draw matches in elimination categories
    MATCH_SYSTEM msys = c.getMatchSystem();
    bool isElimCat = ((msys == SINGLE_ELIM) || (msys == RANKING));
    if (isElimCat && allowDraw)
    {
      return false;
    }

    // ensure consistent scoring before accepting draw
    if (allowDraw)
    {
      int winScore = c.getParameter_int(WIN_SCORE);
      int drawScore = c.getParameter_int(DRAW_SCORE);

      if (drawScore < 1)
      {
        drawScore = 1;
        c.row.update(CAT_DRAW_SCORE, 1);
      }
      if (winScore <= drawScore)
      {
        winScore = drawScore + 1;
        c.row.update(CAT_WIN_SCORE, winScore);
      }
    }

    // set the new status
    c.row.update(CAT_ACCEPT_DRAW, allowDraw);
    return true;
  }

//----------------------------------------------------------------------------

  bool CatMngr::setCatParam_Score(Category& c, int newScore, bool isDraw)
  {
    if (c.getState() != STAT_CAT_CONFIG)
    {
      return false;
    }
    
    int winScore = c.getParameter_int(WIN_SCORE);
    int drawScore = c.getParameter_int(DRAW_SCORE);
    bool allowDraw = c.getParameter_bool(ALLOW_DRAW);
    
    // only scores above 0 make sense
    if (newScore < 1)
    {
      return false;
    }
    
    // can't update draw score if draw is not enabled
    if (!allowDraw && isDraw)
    {
      return false;
    }
    
    if (isDraw)
    {
      if (newScore >= winScore)
      {
        return false;
      }
      
      c.row.update(CAT_DRAW_SCORE, newScore);
      return true;
    }

    // if we're here, we're updating the win score
    if (allowDraw && (newScore <= drawScore))
    {
      return false;
    }

    c.row.update(CAT_WIN_SCORE, newScore);
    return true;
  }

//----------------------------------------------------------------------------

  ERR CatMngr::pairPlayers(const Category c, const Player& p1, const Player& p2)
  {
    // all pre-conditions for pairing two players are checked
    // in the category. If this check is positive, we can start
    // right away with creating the pair in the database
    ERR e = c.canPairPlayers(p1, p2);
    if (e != OK)
    {
      return e;
    }
    
    // create the pair
    SqliteOverlay::ColumnValueClause cvc;
    cvc.addIntCol(PAIRS_CAT_REF, c.getId());
    cvc.addIntCol(PAIRS_PLAYER1_REF, p1.getId());
    cvc.addIntCol(PAIRS_PLAYER2_REF, p2.getId());
    cvc.addIntCol(PAIRS_GRP_NUM, GRP_NUM__NOT_ASSIGNED);   // Default value: no group

    db->getTab(TAB_PAIRS)->insertRow(cvc);
    
    CentralSignalEmitter::getInstance()->playersPaired(c, p1, p2);
    
    return OK;
  }

//----------------------------------------------------------------------------

  ERR CatMngr::splitPlayers(const Category c, const Player& p1, const Player& p2) const
  {
    // all pre-conditions for splitting two players are checked
    // in the category. If this check is positive, we can start
    // right away with deleting the pair from the database
    ERR e = c.canSplitPlayers(p1, p2);
    if (e != OK)
    {
      return e;
    }
    
    // delete all combinations of p1/p2 pairs from the database
    SqliteOverlay::WhereClause wc;
    wc.addIntCol(PAIRS_CAT_REF, c.getId());
    wc.addIntCol(PAIRS_PLAYER1_REF, p1.getId());
    wc.addIntCol(PAIRS_PLAYER2_REF, p2.getId());
    SqliteOverlay::DbTab* pairsTab = db->getTab(TAB_PAIRS);
    pairsTab->deleteRowsByWhereClause(wc);
    wc.clear();
    wc.addIntCol(PAIRS_CAT_REF, c.getId());
    wc.addIntCol(PAIRS_PLAYER1_REF, p2.getId());
    wc.addIntCol(PAIRS_PLAYER2_REF, p1.getId());
    pairsTab->deleteRowsByWhereClause(wc);
    
    CentralSignalEmitter::getInstance()->playersSplit(c, p1, p2);
    
    return OK;
  }


//----------------------------------------------------------------------------

  ERR CatMngr::splitPlayers(const Category c, int pairId) const
  {
    SqliteOverlay::DbTab* pairsTab = db->getTab(TAB_PAIRS);
    PlayerMngr pmngr{db};
    try
    {
      SqliteOverlay::TabRow r = pairsTab->operator [](pairId);
      Player p1 = pmngr.getPlayer(r.getInt(PAIRS_PLAYER1_REF));
      Player p2 = pmngr.getPlayer(r.getInt(PAIRS_PLAYER2_REF));
      return splitPlayers(c, p1, p2);
    }
    catch (exception e)
    {
      
    }
    return INVALID_ID;
  }

//----------------------------------------------------------------------------

  ERR CatMngr::renameCategory(Category& c, const QString& nn)
  {
    QString newName = nn.trimmed();
    
    // Ensure the new name is valid
    if ((newName.isEmpty()) || (newName.length() > MAX_NAME_LEN))
    {
      return INVALID_NAME;
    }
    
    // make sure the new name doesn't exist yet
    if (hasCategory(newName))
    {
      return NAME_EXISTS;
    }
    
    c.row.update(GENERIC_NAME_FIELD_NAME, newName.toUtf8().constData());
    
    return OK;
  }

//----------------------------------------------------------------------------

  ERR CatMngr::freezeConfig(const Category& c)
  {
    // make sure that we can actually freeze the config
    unique_ptr<Category> specialObj = c.convertToSpecializedObject();
    ERR e = specialObj->canFreezeConfig();
    if (e != OK) return e;

    // one additional check that is common for all categories:
    // none of the assigned player is allowed to be in state
    // WAIT_FOR_REGISTRATION
    for (const Player& pl : c.getAllPlayersInCategory())
    {
      if (pl.getState() == STAT_PL_WAIT_FOR_REGISTRATION)
      {
        return NOT_ALL_PLAYERS_REGISTERED;
      }
    }
    
    // Okay, we're good to go
    //
    // The only task when freezing the config is to convert single players
    // to "pairs without a partner" and to set the new state
    
    
    // We can safely convert all remaining "single players" to "pairs". We
    // never make it to this point if the remaining single players are "bad"
    // (e.g. in regular doubles). This is ensured by the call above to "canFreezeConfig()"
    
    /*
     * IMPORTANT:
     * 
     * The application assumes that no "pairs without a partner" exist in the
     * database as long as the category is in STAT_CAT_CONFIG.
     * 
     * This assertion has to be met!
     * 
     * Another assertion:
     * A category can switch back from STAT_CAT_FROZEN to STAT_CAT_CONFIG by
     * just removing all "pairs without a partner" from the database. This means
     * in particular that no links / references to PairIDs may be established
     * while in STAT_CAT_FROZEN.
     * 
     * Links and refs to PairIDs shall be handled in memory only. Only when we transition
     * from STAT_CAT_FROZEN to STAT_CAT_IDLE the references shall be written to the
     * database in one large, "atomic" commit.
     * 
     * Rational:
     * While in STAT_CAT_FROZEN we can do GUI activities for e. g. initial ranking
     * or group assignments. For this, we need PairIDs. So we switch to
     * STAT_CAT_FROZEN during the GUI activities. If the activities are canceled by
     * the user, we switch back to config mode. If the activities are confirmed /
     * committed by the user, we write the results to the DB and make an
     * non-revertable switch to STAT_CAT_IDLE.
     */
    
    PlayerPairList ppList = c.getPlayerPairs();
    int catId = c.getId();
    DbTab* pairsTab = db->getTab(TAB_PAIRS);
    bool isDbErr;
    auto tg = db->acquireTransactionGuard(false, &isDbErr);
    if (isDbErr) return DATABASE_ERROR;
    for (int i=0; i < ppList.size(); ++i)
    {
      PlayerPair pp = ppList.at(i);
      if (!(pp.hasPlayer2()))
      {
        int playerId = pp.getPlayer1().getId();

        SqliteOverlay::ColumnValueClause cvc;
        cvc.addIntCol(PAIRS_CAT_REF, catId);
        cvc.addIntCol(PAIRS_GRP_NUM, GRP_NUM__NOT_ASSIGNED);
        cvc.addIntCol(PAIRS_PLAYER1_REF, playerId);
        // leave out PAIRS_PLAYER2_REF to assign a NULL value

        int dbErr;
        int newId = pairsTab->insertRow(cvc, &dbErr);
        if ((newId < 1) || (dbErr != SQLITE_DONE))
        {
          return DATABASE_ERROR;   // implicit rollback
        }
      }
    }

    // update the category state
    OBJ_STATE oldState = c.getState();  // this MUST be STAT_CAT_CONFIG, ensured by canFreezeConfig
    c.setState(STAT_CAT_FROZEN);

    bool isOkay = tg ? tg->commit() : true;
    if (!isOkay) return DATABASE_ERROR;
    tg.reset();   // explicitly destroy the guard

    CentralSignalEmitter::getInstance()->categoryStatusChanged(c, oldState, STAT_CAT_FROZEN);
    
    return OK;
  }


//----------------------------------------------------------------------------
  
  ERR CatMngr::unfreezeConfig(const Category& c)
  {
    OBJ_STATE oldState = c.getState();
    
    if (oldState == STAT_CAT_CONFIG)
    {
      return CATEGORY_NOT_YET_FROZEN;
    }
    
    if (oldState != STAT_CAT_FROZEN)
    {
      return CATEGORY_UNFREEZEABLE;
    }
    
    // remove all player pairs without a partner from the official pair list
    // See also the constraints in freezeConfig()
    PlayerPairList ppList = c.getPlayerPairs();
    SqliteOverlay::DbTab* ppTab = db->getTab(TAB_PAIRS);
    for (int i=0; i < ppList.size(); ++i)
    {
      PlayerPair pp = ppList.at(i);
      if (pp.hasPlayer2()) continue;  // this is a "real" pair and should survive
      
      // okay, we just encountered a player pair for removal
      int ppId = pp.getPairId();
      if (ppId < 1)
      {
        // We should never get here
        throw std::runtime_error("Inconsistent player pair data!");
      }
      
      // the actual removal
      ppTab->deleteRowsByColumnValue("id", ppId);
    }
    // update the category state
    c.setState(STAT_CAT_CONFIG);
    CentralSignalEmitter::getInstance()->categoryStatusChanged(c, STAT_CAT_FROZEN, STAT_CAT_CONFIG);
    
    return OK;
  }


//----------------------------------------------------------------------------

  ERR CatMngr::startCategory(const Category &c, vector<PlayerPairList> grpCfg, PlayerPairList seed, ProgressQueue *progressNotificationQueue)
  {
    // we can only transition to "IDLE" if we are "FROZEN"
    if (c.getState() != STAT_CAT_FROZEN)
    {
      return CATEGORY_NOT_YET_FROZEN;
    }

    // let's check if we have all the data we need
    unique_ptr<Category> specializedCat = c.convertToSpecializedObject();
    if (specializedCat->needsGroupInitialization())
    {
      ERR e = specializedCat->canApplyGroupAssignment(grpCfg);
      if (e != OK) return e;
    }
    if (specializedCat->needsInitialRanking())
    {
      ERR e = specializedCat->canApplyInitialRanking(seed);
      if (e != OK) return e;
    }

    // great, it's safe to apply the settings and write to
    // the database
    if (specializedCat->needsGroupInitialization())
    {
      ERR e = specializedCat->applyGroupAssignment(grpCfg);
      if (e != OK)
      {
        throw std::runtime_error("Applying group settings failed unexpectedly. Database corruption likely. !! H E L P !!");
      }
    }

    if (specializedCat->needsInitialRanking())
    {
      ERR e = specializedCat->applyInitialRanking(seed);
      if (e != OK)
      {
        throw std::runtime_error("Applying initial category ranking failed unexpectedly. Database corruption likely. !! H E L P !!");
      }
    }

    // switch the category to IDLE state
    c.setState(STAT_CAT_IDLE);
    CentralSignalEmitter::getInstance()->categoryStatusChanged(c, STAT_CAT_FROZEN, STAT_CAT_IDLE);

    // do the individual prep of the first round
    ERR result = specializedCat->prepareFirstRound(progressNotificationQueue);

    // trigger another signal for updating match counters, match time
    // predictions etc.
    //
    // this shouldn't do any harm
    CentralSignalEmitter::getInstance()->categoryStatusChanged(c, STAT_CAT_IDLE, STAT_CAT_IDLE);

    // indicate the completeness of the initialization to the queue, if necessary
    if (progressNotificationQueue != nullptr)
    {
      progressNotificationQueue->push(-1);
    }

    return result;
  }

//----------------------------------------------------------------------------

  /**
   * Changes to category's status from IDLE to PLAYING or back to IDLE or finished,
   * depending on the matches currently being played.
   *
   * @param c the category to update
   */
  void CatMngr::updateCatStatusFromMatchStatus(const Category &c)
  {
    OBJ_STATE curStat = c.getState();
    if ((curStat != STAT_CAT_IDLE) && (curStat != STAT_CAT_PLAYING) && (curStat != STAT_CAT_WAIT_FOR_INTERMEDIATE_SEEDING))
    {
      return;  // nothing to do for us
    }

    CentralSignalEmitter* cse = CentralSignalEmitter::getInstance();

    // determine whether we have at least one RUNING
    // match in the category
    MatchMngr mm{db};
    bool hasMatchRunning = false;
    for (auto mg : mm.getMatchGroupsForCat(c))
    {
      hasMatchRunning = mg.hasMatchesInState(STAT_MA_RUNNING);
      //hasUnfinishedMatch = mg.hasMatches__NOT__InState(STAT_MA_FINISHED);

      if (hasMatchRunning) break;
    }

    // if we're IDLE and at least one match is being played,
    // change state to PLAYING
    if ((curStat == STAT_CAT_IDLE) && hasMatchRunning)
    {
      c.setState(STAT_CAT_PLAYING);
      cse->categoryStatusChanged(c, STAT_CAT_IDLE, STAT_CAT_PLAYING);
      return;
    }

    // check if the whole category is finished
    CatRoundStatus crs = c.getRoundStatus();
    int lastFinishedRound = crs.getFinishedRoundsCount();
    int totalRounds = crs.getTotalRoundsCount();
    bool catIsFinished = ((totalRounds > 0) && (totalRounds == lastFinishedRound));

    // if we've finished the last round
    // change state to FINALIZED
    if ((curStat != STAT_CAT_FINALIZED) && catIsFinished)
    {
      c.setState(STAT_CAT_FINALIZED);
      cse->categoryStatusChanged(c, STAT_CAT_PLAYING, STAT_CAT_FINALIZED);
      return;
    }

    // if we're PLAYING and were not finished
    // change state back to IDLE
    if ((curStat == STAT_CAT_PLAYING) && !catIsFinished && !hasMatchRunning)
    {
      c.setState(STAT_CAT_IDLE);
      cse->categoryStatusChanged(c, STAT_CAT_PLAYING, STAT_CAT_IDLE);
      return;
    }
  }

//----------------------------------------------------------------------------

  bool CatMngr::switchCatToWaitForSeeding(const Category& cat)
  {
    // only switch to SEEDING if no match is currently running
    if (cat.getState() != STAT_CAT_IDLE) return false;

    cat.setState(STAT_CAT_WAIT_FOR_INTERMEDIATE_SEEDING);
    CentralSignalEmitter::getInstance()->categoryStatusChanged(cat, STAT_CAT_IDLE, STAT_CAT_WAIT_FOR_INTERMEDIATE_SEEDING);
    return true;
  }

//----------------------------------------------------------------------------

  std::function<bool (Category&, Category&)> CatMngr::getCategorySortFunction_byName()
  {
    return [](Category& c1, Category& c2) {
      return (QString::localeAwareCompare(c1.getName(), c2.getName()) < 0) ? true : false;
    };
  }

//----------------------------------------------------------------------------

  vector<PlayerPair> CatMngr::getSeeding(const Category& c) const
  {
    // as long as the category is still in configuration, we can't rely
    // on the existence of valid player pairs in the database and thus
    // we'll return an empty list as an error indicator
    if (c.getState() == STAT_CAT_CONFIG) return PlayerPairList();

    // get the player pairs for the category
    SqliteOverlay::DbTab* pairTab = db->getTab(TAB_PAIRS);
    SqliteOverlay::WhereClause wc;
    wc.addIntCol(PAIRS_CAT_REF, c.getId());
    wc.setOrderColumn_Asc(PAIRS_INITIAL_RANK);

    return getObjectsByWhereClause<PlayerPair>(pairTab, wc);
  }

  //----------------------------------------------------------------------------

  ERR CatMngr::canDeleteCategory(const Category& cat) const
  {
    // check 1: the category must be in state CONFIG
    if (cat.getState() != STAT_CAT_CONFIG)
    {
      return CATEGORY_NOT_CONFIGURALE_ANYMORE;
    }

    // check 2: all players must be removable from this category
    for (const Player& pl : cat.getAllPlayersInCategory())
    {
      if (!(cat.canRemovePlayer(pl)))
      {
        return PLAYER_NOT_REMOVABLE_FROM_CATEGORY;
      }
    }

    // okay, we're good to go
    return OK;
  }

//----------------------------------------------------------------------------

  ERR CatMngr::continueWithIntermediateSeeding(const Category& c, const PlayerPairList& seeding, ProgressQueue* progressNotificationQueue)
  {
    if (c.getState() != STAT_CAT_WAIT_FOR_INTERMEDIATE_SEEDING)
    {
      return CATEGORY_NEEDS_NO_SEEDING;
    }

    auto specialCat = c.convertToSpecializedObject();
    ERR e = specialCat->resolveIntermediateSeeding(seeding, progressNotificationQueue);

    // indicate the completeness of the initialization to the queue, if necessary
    if (progressNotificationQueue != nullptr)
    {
      progressNotificationQueue->push(-1);
    }

    if (e != OK) return e;

    // if the previous calls succeeded, we are guaranteed to
    // safely transit to IDLE and continue with new matches,
    // if necessary
    c.setState(STAT_CAT_IDLE);
    CentralSignalEmitter::getInstance()->categoryStatusChanged(c, STAT_CAT_WAIT_FOR_INTERMEDIATE_SEEDING, STAT_CAT_IDLE);

    return OK;
  }

//----------------------------------------------------------------------------


}
