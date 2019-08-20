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

using namespace SqliteOverlay;

namespace QTournament
{

  CatMngr::CatMngr(const TournamentDB& _db)
  : TournamentDatabaseObjectManager(_db, TabCategory)
  {
  }

//----------------------------------------------------------------------------

  Error CatMngr::createNewCategory(const QString& cn)
  {
    QString catName = cn.trimmed();
    
    if (catName.isEmpty())
    {
      return Error::InvalidName;
    }
    
    if (catName.length() > MaxNameLen)
    {
      return Error::InvalidName;
    }
    
    if (hasCategory(catName))
    {
      return Error::NameExists;
    }
    
    // create a new table row and set some arbitrary default data
    ColumnValueClause cvc;
    cvc.addCol(GenericNameFieldName, QString2StdString(catName));
    cvc.addCol(CAT_AcceptDraw, false);
    cvc.addCol(CAT_Sys, static_cast<int>(MatchSystem::GroupsWithKO));
    cvc.addCol(CAT_MatchType, static_cast<int>(MatchType::Singles));
    cvc.addCol(CAT_Sex, static_cast<int>(Sex::M));
    cvc.addCol(GenericStateFieldName, static_cast<int>(ObjState::CAT_Config));
    cvc.addCol(CAT_WinScore, 2);
    cvc.addCol(CAT_DrawScore, 1);
    cvc.addCol(CAT_GroupConfig, KO_Config(KO_Start::Quarter, false).toString().toUtf8().constData());
    
    CentralSignalEmitter* cse = CentralSignalEmitter::getInstance();
    cse->beginCreateCategory();
    tab.insertRow(cvc);
    fixSeqNumberAfterInsert();
    cse->endCreateCategory(tab.length() - 1); // the new sequence number is always the highest
    
    return Error::OK;
  }

  //----------------------------------------------------------------------------

  Error CatMngr::cloneCategory(const Category& src, const QString& catNamePostfix)
  {
    if (catNamePostfix.isEmpty())
    {
      return Error::InvalidName;
    }

    // set an arbitrarily chosen maximum of 10 characters for the postfix
    if (catNamePostfix.length() > 10)
    {
      return Error::InvalidName;
    }

    // try create a new category until we've found a valid name
    int cnt = 0;
    Error err;
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
      } while (dstCatName.length() > MaxNameLen);

      // try to actually create the category
      err = createNewCategory(dstCatName);
    } while ((err == Error::NameExists) && (cnt < 100));   // a maximum limit of 100 retries

    // did we succeed?
    if (err != Error::OK)
    {
      return err;   // give up
    }
    Category clone = getCategory(dstCatName);

    // copy the settings fromt the source category to clone
    err = clone.setMatchSystem(src.getMatchSystem());
    assert(err == Error::OK);
    err = clone.setMatchType(src.getMatchType());
    assert(err == Error::OK);
    err = clone.setSex(src.getSex());
    assert(err == Error::OK);
    bool isOk = setCatParam_AllowDraw(clone, src.getParameter_bool(CatParameter::AllowDraw));
    assert(isOk);
    isOk = setCatParam_Score(clone, src.getParameter_int(CatParameter::WinScore), false);
    assert(isOk);
    setCatParam_Score(clone, src.getParameter_int(CatParameter::DrawScore), true);  // no assert here; setting draw score may fail if draw is not allowed
    KO_Config ko{src.getParameter_string(CatParameter::GroupConfig)};
    isOk = clone.setParameter(CatParameter::GroupConfig, ko.toString());
    assert(isOk);
    setCatParameter(clone, CatParameter::RoundRobinIterations, src.getParameter_int(CatParameter::RoundRobinIterations));

    // Do not copy the BracketVisData here, because the clone is still in
    // CONFIG and BracketVisData is created when starting the cat

    // assign the players to the category
    for (const Player& pl : src.getAllPlayersInCategory())
    {
      err = addPlayerToCategory(pl, clone);
      if (err != Error::OK)
      {
        return err;   // shouldn't happen
      }
    }

    // pair players, if applicable
    if (src.getMatchType() != MatchType::Singles)
    {
      for (const PlayerPair& pp : src.getPlayerPairs())
      {
        if (!(pp.hasPlayer2())) continue;

        err = pairPlayers(clone, pp.getPlayer1(), pp.getPlayer2());
        if (err != Error::OK)
        {
          return err;   // shouldn't happen
        }
      }
    }

    return Error::OK;
  }

//----------------------------------------------------------------------------

  bool CatMngr::hasCategory(const QString& catName) const
  {
    return (tab.getMatchCountForColumnValue(GenericNameFieldName, QString2StdString(catName)) > 0);
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
    auto cat = getSingleObjectByColumnValue<Category>(GenericNameFieldName, QString2StdString(name));
    if (!cat)
    {
      throw std::invalid_argument("The category '" + QString2StdString(name) + "' does not exist");
    }
    
    return *cat;
  }

//----------------------------------------------------------------------------

  std::optional<Category> CatMngr::getCategory(int id)
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

  Error CatMngr::setMatchSystem(Category& cat, MatchSystem newMatchSystem)
  {
    if (cat.getState() != ObjState::CAT_Config)
    {
      return Error::CategoryNotConfiguraleAnymore;
    }

    // TODO: implement checks, updates to other tables etc
    cat.row.update(CAT_Sys, static_cast<int>(newMatchSystem));
    
    // if we switch to single elimination categories or
    // to the ranking system, we want to
    // prevent draw
    if ((newMatchSystem == MatchSystem::SingleElim) || (newMatchSystem == MatchSystem::Ranking))
    {
      setCatParam_AllowDraw(cat, false);
    }

    return Error::OK;
  }

//----------------------------------------------------------------------------

  Error CatMngr::setMatchType(Category& cat, MatchType newMatchType)
  {
    // we can only change the match type while being in config mode
    if (cat.getState() != ObjState::CAT_Config)
    {
      return Error::CategoryNotConfiguraleAnymore;
    }
    
    // temporarily store all existing player pairs
    PlayerPairList pairList = cat.getPlayerPairs();
    
    // check if we can split all pairs
    bool canSplit = true;
    for (const PlayerPair& pp : pairList)
    {
      if (!(pp.hasPlayer2())) continue;
      
      if (cat.canSplitPlayers(pp.getPlayer1(), pp.getPlayer2()) != Error::OK)
      {
        canSplit = false;
      }
    }
    if (!canSplit)
    {
      return Error::InvalidReconfig;  // if only one pair can't be split, refuse to change the match type
    }
    
    // actually split all pairs
    for (const PlayerPair& pp : pairList)
    {
      if (!(pp.hasPlayer2())) continue;
      
      splitPlayers(cat, pp.getPlayer1(), pp.getPlayer2());
    }
    
    // if we come from mixed, we force the sex type "Don't care" to avoid
    // that too many players will be removed (unwanted) from the category
    MatchType oldType = cat.getMatchType();
    if (oldType == MatchType::Mixed)
    {
      setSex(cat, Sex::DontCare);   // no error checking here, setting "don't care" should always work because it's least restrictive
    }
    
    // change the match type
    cat.row.update(CAT_MatchType, static_cast<int>(newMatchType));
    
    // try to recreate as many pairs as possible
    for (const PlayerPair& pp : pairList)
    {
      if (!(pp.hasPlayer2())) continue;
      
      pairPlayers(cat, pp.getPlayer1(), pp.getPlayer2());
    }
    
    // IMPORTANT:
    // THIS OPERATION CHANGED THE PAIR-IDs OF ALL PAIRS!
    
    return Error::OK;
  }

//----------------------------------------------------------------------------

  Error CatMngr::setSex(Category& cat, Sex newSex)
  {
    // we can only change the sex while being in config mode
    if (cat.getState() != ObjState::CAT_Config)
    {
      return Error::CategoryNotConfiguraleAnymore;
    }
    
    // unless we switch to "don't care", we have to make sure that
    // we can remove the wrong players
    if ((newSex != Sex::DontCare) && (cat.getMatchType() != MatchType::Mixed))
    {
      PlayerList allPlayers = cat.getAllPlayersInCategory();
      for (const Player& p : allPlayers)
      {
        // skip players with matching sex
        if (p.getSex() == newSex) continue;

        // for all other players, check if we can remove them
        if (!(cat.canRemovePlayer(p)))
        {
          return Error::InvalidReconfig;
        }
      }
      
      // okay, we can be sure that all "unwanted" players can be removed.
      // do it.
      for (const Player& p : allPlayers)
      {
        // skip players with matching sex
        if (p.getSex() == newSex) continue;

        // for all other players, check if we can remove them
        cat.removePlayer(p);
      }
    }
    
    // if we de-active "don't care" in a mixed category, we have to split
    // all non-compliant pairs. The players itself can remain in the category
    if ((newSex != Sex::DontCare) && (cat.getMatchType() == MatchType::Mixed)) {
      PlayerPairList allPairs = cat.getPlayerPairs();
      
      for (const PlayerPair& pp : allPairs) {

        // Skip unpaired players
        if (!(pp.hasPlayer2())) continue;

        // Skip true mixed pairs
        if (pp.getPlayer1().getSex() != pp.getPlayer2().getSex()) continue;

        // check if we can split "false" mixed pairs (= same sex pairs)
        if (cat.canSplitPlayers(pp.getPlayer1(), pp.getPlayer2()) != Error::OK) {
          return Error::InvalidReconfig;
        }
      }
      
      // now we can be sure that all unwanted pairs can be split
      for (const PlayerPair& pp : allPairs) {

        // Skip unpaired players
        if (!(pp.hasPlayer2())) continue;

        // Skip true mixed pairs
        if (pp.getPlayer1().getSex() != pp.getPlayer2().getSex()) continue;

        // check if we can split "false" mixed pairs (= same sex pairs)
        splitPlayers(cat, pp.getPairId());
      }
    }
    
    // execute the actual change
    int sexInt = static_cast<int>(newSex);
    cat.row.update(CAT_Sex, sexInt);
    
    return Error::OK;
  }

//----------------------------------------------------------------------------

  Error CatMngr::addPlayerToCategory(const Player& p, const Category& cat)
  {
    if (!(cat.canAddPlayers()))
    {
      return Error::CategoryClosedForMorePlayers;
    }
    
    if (cat.hasPlayer(p))
    {
      return Error::PlayerAlreadyInCategory;
    }
    
    if (cat.getAddState(p) != CatAddState::CanJoin)
    {
      return Error::PlayerNotSuitable;
    }
    
    // TODO: check that player is not permanently disabled
    
    // actually add the player
    ColumnValueClause cvc;
    cvc.addCol(P2C_CatRef, cat.getId());
    cvc.addCol(P2C_PlayerRef, p.getId());
    DbTab tabP2C{db, TabP2C, false};
    tabP2C.insertRow(cvc);
    
    CentralSignalEmitter::getInstance()->playerAddedToCategory(p, cat);
    
    return Error::OK;
  }

//----------------------------------------------------------------------------

  Error CatMngr::removePlayerFromCategory(const Player& p, const Category& cat) const
  {
    if (!(cat.canRemovePlayer(p)))
    {
      return Error::PlayerNotRemovableFromCategory;
    }
    
    if (!(cat.hasPlayer(p)))
    {
      return Error::PlayerNotInCategory;
    }
    
    if (cat.isPaired(p))
    {
      Player partner = cat.getPartner(p);
      Error e = splitPlayers(cat, p, partner);
      if (e != Error::OK)
      {
        return e;
      }
    }
    
    // And: unless we play with random partners, the double partner
    // has to be removed from the category as well if we're beyond
    // "Category Configuration" state
    
    // actually delete the assignment
    WhereClause wc;
    wc.addCol(P2C_CatRef, cat.getId());
    wc.addCol(P2C_PlayerRef, p.getId());
    DbTab tabP2C{db, TabP2C, false};
    int cnt = tabP2C.deleteRowsByWhereClause(wc);
    assert(cnt == 1);
    
    CentralSignalEmitter::getInstance()->playerRemovedFromCategory(p, cat);
    
    return Error::OK;
  }

  //----------------------------------------------------------------------------

  Error CatMngr::deleteCategory(const Category& cat) const
  {
    Error e = canDeleteCategory(cat);
    if (e != Error::OK) return e;

    // remove all players from the category
    PlayerList allPlayers = cat.getAllPlayersInCategory();
    for (const Player& pl : allPlayers)
    {
      e = removePlayerFromCategory(pl, cat);
      if (e != Error::OK)
      {
        return e;   // after all the checks before, this shouldn't happen
      }
    }

    // a few checks for the cowards
    int catId = cat.getId();
    assert(DbTab(db, TabP2C, false).getMatchCountForColumnValue(P2C_CatRef, catId) == 0);
    assert(DbTab(db, TabPairs, false).getMatchCountForColumnValue(Pairs_CatRef, catId) == 0);
    assert(DbTab(db, TabMatchGroup, false).getMatchCountForColumnValue(MG_CatRef, catId) == 0);
    assert(DbTab(db, TabMatchSystem, false).getMatchCountForColumnValue(RA_CatRef, catId) == 0);
    assert(DbTab(db, TabBracketVis, false).getMatchCountForColumnValue(BV_CatRef, catId) == 0);

    // the actual deletion
    int oldSeqNum = cat.getSeqNum();
    CentralSignalEmitter* cse = CentralSignalEmitter::getInstance();
    cse->beginDeleteCategory(oldSeqNum);
    tab.deleteRowsByColumnValue("id", catId);
    fixSeqNumberAfterDelete(tab, oldSeqNum);
    cse->endDeleteCategory();

    return Error::OK;
  }

  //----------------------------------------------------------------------------

  Error CatMngr::deleteRunningCategory(const Category& cat) const
  {
    // only one initial check: can we do it "the soft way"?
    if (canDeleteCategory(cat) == Error::OK)
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

      Error err = mm.undoMatchCall(ma);
      if (err != Error::OK) return err;   // shouldn't happen
    }

    // step 2: un-stage all staged match groups of this category
    auto stagedMatchGroups = mm.getStagedMatchGroupsOrderedBySequence();
    for (const MatchGroup& mg : stagedMatchGroups)
    {
      if (mg.getCategory() != cat) continue;

      Error err = mm.unstageMatchGroup(mg);
      if (err != Error::OK) return err;   // shouldn't happen
    }

    // step 3: tell everyone that something baaaad is about to happen
    CentralSignalEmitter* cse = CentralSignalEmitter::getInstance();
    cse->beginResetAllModels();

    //
    // now the actual deletion starts
    //
    try
    {
      auto trans = db.get().startTransaction();

      int catId = cat.getId();

      // deletion 1: bracket vis data, because it has only outgoing refs
      DbTab t{db, TabBracketVis, false};
      t.deleteRowsByColumnValue(BV_CatRef, catId);

      // deletion 2: ranking data, because it has only outgoing refs
      t = DbTab{db, TabMatchSystem, false};
      t.deleteRowsByColumnValue(RA_CatRef, catId);

      // deletion 3a: matches, they are refered to by bracket vis data only
      // deletion 3b: match groups, they are refered to only by ranking data and matches
      t = DbTab{db, TabMatch, false};
      DbTab mgTab{db, TabMatchGroup, false};
      for (const auto& mg : getObjectsByColumnValue<MatchGroup>(mgTab, MG_CatRef, cat.getId()))
      {
        for (const auto& ma : mg.getMatches())
        {
          int deletedSeqNum = ma.getSeqNum();
          t.deleteRowsByColumnValue("id", ma.getId());
          fixSeqNumberAfterDelete(t, deletedSeqNum);
        }

        // the match group has incoming links from matches
        // and ranking and both have been deleted by now
        int deletedSeqNum = mg.getSeqNum();
        mgTab.deleteRowsByColumnValue("id", mg.getId());
        fixSeqNumberAfterDelete(mgTab, deletedSeqNum);
      }

      // deletion 4: player pairs
      t = DbTab{db, TabPairs, false};
      t.deleteRowsByColumnValue(Pairs_CatRef, catId);

      // deletion 5: player to category allocation
      t = DbTab{db, TabP2C, false};
      t.deleteRowsByColumnValue(P2C_CatRef, catId);

      // final deletion: the category itself
      int deletedSeqNum = cat.getSeqNum();
      tab.deleteRowsByColumnValue("id", catId);
      fixSeqNumberAfterDelete(tab, deletedSeqNum);

      //
      // deletion completed
      //
      trans.commit();

      // refresh all models and the reports tab
      cse->endResetAllModels();

      // tell all other widgets that a category has been deleted
      cse->categoryRemovedFromTournament(catId, deletedSeqNum);

      return Error::OK;
    }
    catch (...)
    {
      return Error::DatabaseError;
    }
  }

//----------------------------------------------------------------------------

  Category CatMngr::getCategoryById(int id)
  {
    auto cat = getSingleObjectByColumnValue<Category>("id", id);
    if (!cat)
    {
     throw std::invalid_argument("The category with ID " + std::to_string(id) + " does not exist");
    }

    return *cat;
  }

//----------------------------------------------------------------------------

  Category CatMngr::getCategoryBySeqNum(int seqNum)
  {
    auto cat = getSingleObjectByColumnValue<Category>(GenericSeqnumFieldName, seqNum);
    if (!cat)
    {
      throw std::invalid_argument("The category with sequence number " + std::to_string(seqNum) + " does not exist");
    }

    return *cat;
  }

//----------------------------------------------------------------------------

  QHash<Category, CatAddState> CatMngr::getAllCategoryAddStates(Sex s)
  {
    CategoryList allCat = getAllCategories();
    QHash<Category, CatAddState> result;
    
    for (const Category& c : allCat)
    {
      result[c] = c.getAddState(s);
    }
    
    return result;
  }

//----------------------------------------------------------------------------

  QHash<Category, CatAddState> CatMngr::getAllCategoryAddStates(const Player& p)
  {
    CategoryList allCat = getAllCategories();
    QHash<Category, CatAddState> result;
    
    for (Category& c : allCat)
    {
      result[c] = c.getAddState(p);
    }
    
    return result;
  }

//----------------------------------------------------------------------------

  bool CatMngr::setCatParameter(Category& cat, CatParameter p, const QVariant& v)
  {
    if (p == CatParameter::AllowDraw)
    {
      return setCatParam_AllowDraw(cat, v);
    }
    if (p == CatParameter::DrawScore)
    {
      return setCatParam_Score(cat, v.toInt(), true);
    }
    if (p == CatParameter::WinScore)
    {
      return setCatParam_Score(cat, v.toInt(), false);
    }
    if (p == CatParameter::GroupConfig)
    {
      if (cat.getState() != ObjState::CAT_Config) return false;

      cat.row.update(CAT_GroupConfig, v.toString().toUtf8().constData());
      return true;
    }
    if (p == CatParameter::RoundRobinIterations)
    {
      bool isOk;
      int iterations = v.toInt(&isOk);
      if (!isOk) return false;

      if (iterations <= 0) return false;

      cat.row.update(CAT_RoundRobinIterations, iterations);
      return true;
    }
    
    return false;
  }

//----------------------------------------------------------------------------

  bool CatMngr::setCatParam_AllowDraw(Category& c, const QVariant& v)
  {
    if (c.getState() != ObjState::CAT_Config)
    {
      return false;
    }
    
    bool allowDraw = v.toBool();
    bool oldState = c.getParameter(CatParameter::AllowDraw).toBool();

    if (allowDraw == oldState)
    {
      return true; // no change necessary;
    }

    // no draw matches in elimination categories
    MatchSystem msys = c.getMatchSystem();
    bool isElimCat = ((msys == MatchSystem::SingleElim) || (msys == MatchSystem::Ranking));
    if (isElimCat && allowDraw)
    {
      return false;
    }

    // ensure consistent scoring before accepting draw
    if (allowDraw)
    {
      int winScore = c.getParameter_int(CatParameter::WinScore);
      int drawScore = c.getParameter_int(CatParameter::DrawScore);

      if (drawScore < 1)
      {
        drawScore = 1;
        c.row.update(CAT_DrawScore, 1);
      }
      if (winScore <= drawScore)
      {
        winScore = drawScore + 1;
        c.row.update(CAT_WinScore, winScore);
      }
    }

    // set the new status
    c.row.update(CAT_AcceptDraw, allowDraw);
    return true;
  }

//----------------------------------------------------------------------------

  bool CatMngr::setCatParam_Score(Category& c, int newScore, bool isDraw)
  {
    if (c.getState() != ObjState::CAT_Config)
    {
      return false;
    }
    
    int winScore = c.getParameter_int(CatParameter::WinScore);
    int drawScore = c.getParameter_int(CatParameter::DrawScore);
    bool allowDraw = c.getParameter_bool(CatParameter::AllowDraw);
    
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
      
      c.row.update(CAT_DrawScore, newScore);
      return true;
    }

    // if we're here, we're updating the win score
    if (allowDraw && (newScore <= drawScore))
    {
      return false;
    }

    c.row.update(CAT_WinScore, newScore);
    return true;
  }

//----------------------------------------------------------------------------

  Error CatMngr::pairPlayers(const Category c, const Player& p1, const Player& p2)
  {
    // all pre-conditions for pairing two players are checked
    // in the category. If this check is positive, we can start
    // right away with creating the pair in the database
    Error e = c.canPairPlayers(p1, p2);
    if (e != Error::OK)
    {
      return e;
    }
    
    // create the pair
    ColumnValueClause cvc;
    cvc.addCol(Pairs_CatRef, c.getId());
    cvc.addCol(Pairs_Player1Ref, p1.getId());
    cvc.addCol(Pairs_Player2Ref, p2.getId());
    cvc.addCol(Pairs_GrpNum, GroupNum_NotAssigned);   // Default value: no group

    DbTab{db, TabPairs, false}.insertRow(cvc);
    
    CentralSignalEmitter::getInstance()->playersPaired(c, p1, p2);
    
    return Error::OK;
  }

//----------------------------------------------------------------------------

  Error CatMngr::splitPlayers(const Category c, const Player& p1, const Player& p2) const
  {
    // all pre-conditions for splitting two players are checked
    // in the category. If this check is positive, we can start
    // right away with deleting the pair from the database
    Error e = c.canSplitPlayers(p1, p2);
    if (e != Error::OK)
    {
      return e;
    }
    
    // delete all combinations of p1/p2 pairs from the database
    WhereClause wc;
    wc.addCol(Pairs_CatRef, c.getId());
    wc.addCol(Pairs_Player1Ref, p1.getId());
    wc.addCol(Pairs_Player2Ref, p2.getId());
    DbTab pairsTab{db, TabPairs, false};
    pairsTab.deleteRowsByWhereClause(wc);
    wc.clear();
    wc.addCol(Pairs_CatRef, c.getId());
    wc.addCol(Pairs_Player1Ref, p2.getId());
    wc.addCol(Pairs_Player2Ref, p1.getId());
    pairsTab.deleteRowsByWhereClause(wc);
    
    CentralSignalEmitter::getInstance()->playersSplit(c, p1, p2);
    
    return Error::OK;
  }


//----------------------------------------------------------------------------

  Error CatMngr::splitPlayers(const Category c, int pairId) const
  {
    DbTab pairsTab{db, TabPairs, false};
    auto row = tab.get2(pairId);
    if (!row) return Error::InvalidId;

    auto p1Id = row->getInt2(Pairs_Player1Ref);
    auto p2Id = row->getInt2(Pairs_Player2Ref);
    if (!p1Id || !p2Id)
    {
      return Error::InvalidId;
    }

    PlayerMngr pmngr{db};
    Player p1 = pmngr.getPlayer(*p1Id);
    Player p2 = pmngr.getPlayer(*p2Id);
    return splitPlayers(c, p1, p2);
  }

//----------------------------------------------------------------------------

  Error CatMngr::renameCategory(Category& cat, const QString& nn)
  {
    QString newName = nn.trimmed();
    
    // Ensure the new name is valid
    if ((newName.isEmpty()) || (newName.length() > MaxNameLen))
    {
      return Error::InvalidName;
    }
    
    // make sure the new name doesn't exist yet
    if (hasCategory(newName))
    {
      return Error::NameExists;
    }
    
    cat.row.update(GenericNameFieldName, QString2StdString(newName));
    
    return Error::OK;
  }

//----------------------------------------------------------------------------

  Error CatMngr::freezeConfig(const Category& c)
  {
    // make sure that we can actually freeze the config
    auto specialObj = c.convertToSpecializedObject();
    Error e = specialObj->canFreezeConfig();
    if (e != Error::OK) return e;

    // one additional check that is common for all categories:
    // none of the assigned player is allowed to be in state
    // WAIT_FOR_REGISTRATION
    for (const Player& pl : c.getAllPlayersInCategory())
    {
      if (pl.getState() == ObjState::PL_WaitForRegistration)
      {
        return Error::NotAllPlayersRegistered;
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
     * database as long as the category is in ObjState::CAT_Config.
     * 
     * This assertion has to be met!
     * 
     * Another assertion:
     * A category can switch back from ObjState::CAT_Frozen to ObjState::CAT_Config by
     * just removing all "pairs without a partner" from the database. This means
     * in particular that no links / references to PairIDs may be established
     * while in ObjState::CAT_Frozen.
     * 
     * Links and refs to PairIDs shall be handled in memory only. Only when we transition
     * from ObjState::CAT_Frozen to ObjState::CAT_Idle the references shall be written to the
     * database in one large, "atomic" commit.
     * 
     * Rational:
     * While in ObjState::CAT_Frozen we can do GUI activities for e. g. initial ranking
     * or group assignments. For this, we need PairIDs. So we switch to
     * ObjState::CAT_Frozen during the GUI activities. If the activities are canceled by
     * the user, we switch back to config mode. If the activities are confirmed /
     * committed by the user, we write the results to the DB and make an
     * non-revertable switch to ObjState::CAT_Idle.
     */
    
    PlayerPairList ppList = c.getPlayerPairs();
    int catId = c.getId();
    DbTab pairsTab{db, TabPairs, false};

    try
    {
      auto trans = db.get().startTransaction();

      for (const auto& pp : ppList)
      {
        if (pp.hasPlayer2()) continue;

        ColumnValueClause cvc;
        cvc.addCol(Pairs_CatRef, catId);
        cvc.addCol(Pairs_GrpNum, GroupNum_NotAssigned);
        cvc.addCol(Pairs_Player1Ref, pp.getPlayer1().getId());
        cvc.addNullCol(Pairs_Player2Ref);

        pairsTab.insertRow(cvc);
      }

      // update the category state
      ObjState oldState = c.getState();  // this MUST be ObjState::CAT_Config, ensured by canFreezeConfig
      c.setState(ObjState::CAT_Frozen);

      trans.commit();

      CentralSignalEmitter::getInstance()->categoryStatusChanged(c, oldState, ObjState::CAT_Frozen);

      return Error::OK;
    }
    catch (...)
    {
      return Error::DatabaseError;
    }
  }


//----------------------------------------------------------------------------
  
  Error CatMngr::unfreezeConfig(const Category& cat)
  {
    ObjState oldState = cat.getState();
    
    if (oldState == ObjState::CAT_Config)
    {
      return Error::CategoryNotYetFrozen;
    }
    
    if (oldState != ObjState::CAT_Frozen)
    {
      return Error::CategoryNotUnfreezeable;
    }
    
    // remove all player pairs without a partner from the official pair list
    // See also the constraints in freezeConfig()
    PlayerPairList ppList = cat.getPlayerPairs();
    DbTab pairsTab{db, TabPairs, false};
    try
    {
      auto trans = db.get().startTransaction();

      for (const auto& pp : ppList)
      {
        if (pp.hasPlayer2()) continue;  // this is a "real" pair and should survive

        // okay, we just encountered a player pair for removal
        int ppId = pp.getPairId();
        if (ppId < 1)
        {
          // We should never get here
          throw std::runtime_error("Inconsistent player pair data!");
        }

        // the actual removal
        pairsTab.deleteRowsByColumnValue("id", ppId);
      }
      // update the category state
      cat.setState(ObjState::CAT_Config);

      trans.commit();

      CentralSignalEmitter::getInstance()->categoryStatusChanged(cat, ObjState::CAT_Frozen, ObjState::CAT_Config);

      return Error::OK;
    }
    catch (...)
    {
      return Error::DatabaseError;
    }
  }


//----------------------------------------------------------------------------

  Error CatMngr::startCategory(const Category& cat, const std::vector<PlayerPairList>& grpCfg, const PlayerPairList& seed)
  {
    // we can only transition to "IDLE" if we are "FROZEN"
    if (cat.getState() != ObjState::CAT_Frozen)
    {
      return Error::CategoryNotYetFrozen;
    }

    // let's check if we have all the data we need
    std::unique_ptr<Category> specializedCat = cat.convertToSpecializedObject();
    if (specializedCat->needsGroupInitialization())
    {
      Error e = specializedCat->canApplyGroupAssignment(grpCfg);
      if (e != Error::OK) return e;
    }
    if (specializedCat->needsInitialRanking())
    {
      Error e = specializedCat->canApplyInitialRanking(seed);
      if (e != Error::OK) return e;
    }

    // great, it's safe to apply the settings and write to
    // the database
    if (specializedCat->needsGroupInitialization())
    {
      Error e = specializedCat->applyGroupAssignment(grpCfg);
      if (e != Error::OK)
      {
        throw std::runtime_error("Applying group settings failed unexpectedly. Database corruption likely. !! H E L P !!");
      }
    }

    if (specializedCat->needsInitialRanking())
    {
      Error e = specializedCat->applyInitialRanking(seed);
      if (e != Error::OK)
      {
        throw std::runtime_error("Applying initial category ranking failed unexpectedly. Database corruption likely. !! H E L P !!");
      }
    }

    // switch the category to IDLE state
    cat.setState(ObjState::CAT_Idle);
    CentralSignalEmitter::getInstance()->categoryStatusChanged(cat, ObjState::CAT_Frozen, ObjState::CAT_Idle);

    // do the individual prep of the first round
    Error result = specializedCat->prepareFirstRound();

    // trigger another signal for updating match counters, match time
    // predictions etc.
    //
    // this shouldn't do any harm
    CentralSignalEmitter::getInstance()->categoryStatusChanged(cat, ObjState::CAT_Idle, ObjState::CAT_Idle);

    return result;
  }

//----------------------------------------------------------------------------

  /**
   * Changes to category's status from IDLE to PLAYING or back to IDLE or finished,
   * depending on the matches currently being played.
   *
   * @param c the category to update
   */
  void CatMngr::updateCatStatusFromMatchStatus(const Category &cat)
  {
    ObjState curStat = cat.getState();
    if ((curStat != ObjState::CAT_Idle) && (curStat != ObjState::CAT_Playing) && (curStat != ObjState::CAT_WaitForIntermediateSeeding))
    {
      return;  // nothing to do for us
    }

    CentralSignalEmitter* cse = CentralSignalEmitter::getInstance();

    // determine whether we have at least one RUNING
    // match in the category
    MatchMngr mm{db};
    bool hasMatchRunning = false;
    for (auto mg : mm.getMatchGroupsForCat(cat))
    {
      hasMatchRunning = mg.hasMatchesInState(ObjState::MA_Running);
      //hasUnfinishedMatch = mg.hasMatches__NOT__InState(ObjState::MA_Finished);

      if (hasMatchRunning) break;
    }

    // if we're IDLE and at least one match is being played,
    // change state to PLAYING
    if ((curStat == ObjState::CAT_Idle) && hasMatchRunning)
    {
      cat.setState(ObjState::CAT_Playing);
      cse->categoryStatusChanged(cat, ObjState::CAT_Idle, ObjState::CAT_Playing);
      return;
    }

    // check if the whole category is finished
    CatRoundStatus crs = cat.getRoundStatus();
    int lastFinishedRound = crs.getFinishedRoundsCount();
    int totalRounds = crs.getTotalRoundsCount();
    bool catIsFinished = ((totalRounds > 0) && (totalRounds == lastFinishedRound));

    // if we've finished the last round
    // change state to KO_Start::FinalIZED
    if ((curStat != ObjState::CAT_Finalized) && catIsFinished)
    {
      cat.setState(ObjState::CAT_Finalized);
      cse->categoryStatusChanged(cat, ObjState::CAT_Playing, ObjState::CAT_Finalized);
      return;
    }

    // if we're PLAYING and were not finished
    // change state back to IDLE
    if ((curStat == ObjState::CAT_Playing) && !catIsFinished && !hasMatchRunning)
    {
      cat.setState(ObjState::CAT_Idle);
      cse->categoryStatusChanged(cat, ObjState::CAT_Playing, ObjState::CAT_Idle);
      return;
    }
  }

//----------------------------------------------------------------------------

  bool CatMngr::switchCatToWaitForSeeding(const Category& cat)
  {
    // only switch to SEEDING if no match is currently running
    if (cat.getState() != ObjState::CAT_Idle) return false;

    cat.setState(ObjState::CAT_WaitForIntermediateSeeding);
    CentralSignalEmitter::getInstance()->categoryStatusChanged(cat, ObjState::CAT_Idle, ObjState::CAT_WaitForIntermediateSeeding);
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

  std::vector<PlayerPair> CatMngr::getSeeding(const Category& cat) const
  {
    // as long as the category is still in configuration, we can't rely
    // on the existence of valid player pairs in the database and thus
    // we'll return an empty list as an error indicator
    if (cat.getState() == ObjState::CAT_Config) return PlayerPairList();

    // get the player pairs for the category
    DbTab pairsTab{db, TabPairs, false};
    WhereClause wc;
    wc.addCol(Pairs_CatRef, cat.getId());
    wc.setOrderColumn_Asc(Pairs_InitialRank);

    return getObjectsByWhereClause<PlayerPair>(pairsTab, wc);
  }

  //----------------------------------------------------------------------------

  Error CatMngr::canDeleteCategory(const Category& cat) const
  {
    // check 1: the category must be in state CONFIG
    if (cat.getState() != ObjState::CAT_Config)
    {
      return Error::CategoryNotConfiguraleAnymore;
    }

    // check 2: all players must be removable from this category
    for (const Player& pl : cat.getAllPlayersInCategory())
    {
      if (!(cat.canRemovePlayer(pl)))
      {
        return Error::PlayerNotRemovableFromCategory;
      }
    }

    // okay, we're good to go
    return Error::OK;
  }

//----------------------------------------------------------------------------

  Error CatMngr::continueWithIntermediateSeeding(const Category& c, const PlayerPairList& seeding)
  {
    if (c.getState() != ObjState::CAT_WaitForIntermediateSeeding)
    {
      return Error::CategoryNeedsNoSeeding;
    }

    auto specialCat = c.convertToSpecializedObject();
    Error e = specialCat->resolveIntermediateSeeding(seeding);
    if (e != Error::OK) return e;

    // if the previous calls succeeded, we are guaranteed to
    // safely transit to IDLE and continue with new matches,
    // if necessary
    c.setState(ObjState::CAT_Idle);
    CentralSignalEmitter::getInstance()->categoryStatusChanged(c, ObjState::CAT_WaitForIntermediateSeeding, ObjState::CAT_Idle);

    return Error::OK;
  }

  //----------------------------------------------------------------------------

  std::string CatMngr::getSyncString(const std::vector<int>& rows) const
  {
    std::vector<Sloppy::estring> cols = {"id", GenericNameFieldName, GenericStateFieldName, CAT_MatchType, CAT_Sex, CAT_Sys, CAT_AcceptDraw,
                          CAT_WinScore, CAT_DrawScore, CAT_GroupConfig, CAT_RoundRobinIterations};

    return db.get().getSyncStringForTable(TabCategory, cols, rows);
  }

//----------------------------------------------------------------------------


}
