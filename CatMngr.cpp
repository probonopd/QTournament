/* 
 * File:   TeamMngr.cpp
 * Author: nyoknvk1
 * 
 * Created on 18. Februar 2014, 14:04
 */

#include <stdexcept>
#include <QtCore/qdebug.h>
#include <QtCore/qjsonarray.h>
#include <QList>
#include "HelperFunc.h"
#include "Category.h"
#include "Player.h"
#include "Tournament.h"
#include "KO_Config.h"
#include "CatRoundStatus.h"
#include "CatMngr.h"
#include "TournamentErrorCodes.h"
#include "TournamentDataDefs.h"
#include "TournamentDB.h"

using namespace dbOverlay;

namespace QTournament
{

  CatMngr::CatMngr(TournamentDB* _db)
  : GenericObjectManager(_db), catTab((*db)[TAB_CATEGORY])
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
    QVariantList qvl;
    qvl << GENERIC_NAME_FIELD_NAME << catName;
    qvl << CAT_ACCEPT_DRAW << false;
    qvl << CAT_SYS << static_cast<int>(GROUPS_WITH_KO);
    qvl << CAT_MATCH_TYPE << static_cast<int>(SINGLES);
    qvl << CAT_SEX << static_cast<int>(M);
    qvl << GENERIC_STATE_FIELD_NAME << static_cast<int>(STAT_CAT_CONFIG);
    qvl << CAT_WIN_SCORE << 2;
    qvl << CAT_DRAW_SCORE << 1;
    qvl << CAT_GROUP_CONFIG << KO_Config(QUARTER, false).toString();
    
    emit beginCreateCategory();
    catTab.insertRow(qvl);
    fixSeqNumberAfterInsert(TAB_CATEGORY);
    emit endCreateCategory(catTab.length() - 1); // the new sequence number is always the greatest
    
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
    assert(clone.setMatchSystem(src.getMatchSystem()) == OK);
    assert(clone.setMatchType(src.getMatchType()) == OK);
    assert(clone.setSex(src.getSex()) == OK);
    assert(setCatParam_AllowDraw(clone, src.getParameter_bool(ALLOW_DRAW)));
    assert(setCatParam_Score(clone, src.getParameter_int(WIN_SCORE), false));
    setCatParam_Score(clone, src.getParameter_int(DRAW_SCORE), true);  // no assert here; setting draw score may fail if draw is not allowed
    KO_Config ko{src.getParameter_string(GROUP_CONFIG)};
    assert(clone.setParameter(GROUP_CONFIG, ko.toString()));

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
    return (catTab.getMatchCountForColumnValue(GENERIC_NAME_FIELD_NAME, catName) > 0);
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
      throw std::invalid_argument("The category '" + QString2String(name) + "' does not exist");
    }
    
    TabRow r = catTab.getSingleRowByColumnValue(GENERIC_NAME_FIELD_NAME, name);
    
    return Category(db, r);
  }

//----------------------------------------------------------------------------

  unique_ptr<Category> CatMngr::getCategory(int id)
  {
    return getSingleObjectByColumnValue<Category>(catTab, "id", id);
  }

//----------------------------------------------------------------------------

  /**
   * Returns a list of all categories
   *
   * @Return QList holding all categories
   */
  QList<Category> CatMngr::getAllCategories()
  {
    return getAllObjects<Category>(catTab);
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
    PlayerPairList::const_iterator it = pairList.constBegin();
    bool canSplit = true;
    for (it = pairList.constBegin(); it != pairList.constEnd(); ++it)
    {
      PlayerPair pp = *it;
      
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
    for (it = pairList.constBegin(); it != pairList.constEnd(); ++it)
    {
      PlayerPair pp = *it;
      
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
    for (it = pairList.constBegin(); it != pairList.constEnd(); ++it)
    {
      PlayerPair pp = *it;
      
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
      QList<Player> allPlayers = c.getAllPlayersInCategory();
      QList<Player>::const_iterator it;
      for (it = allPlayers.constBegin(); it != allPlayers.constEnd(); ++it)
      {
	// skip players with matching sex
	if ((*it).getSex() == s) continue;
	
	// for all other players, check if we can remove them
	if (!(c.canRemovePlayer(*it)))
	{
	  return INVALID_RECONFIG;
	}
      }
      
      // okay, we can be sure that all "unwanted" players can be removed.
      // do it.
      for (it = allPlayers.constBegin(); it != allPlayers.constEnd(); ++it)
      {
	// skip players with matching sex
	if ((*it).getSex() == s) continue;
	
	// for all other players, check if we can remove them
	c.removePlayer(*it);
      }
    }
    
    // if we de-active "don't care" in a mixed category, we have to split
    // all non-compliant pairs. The players itself can remain in the category
    if ((s != DONT_CARE) && (c.getMatchType() == MIXED)) {
      PlayerPairList allPairs = c.getPlayerPairs();
      
      PlayerPairList::const_iterator it;
      for (it = allPairs.constBegin(); it != allPairs.constEnd(); ++it) {
	PlayerPair pp = *it;
	
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
      CatMngr* cmngr = Tournament::getCatMngr();
      for (it = allPairs.constBegin(); it != allPairs.constEnd(); ++it) {
	PlayerPair pp = *it;
	
	// Skip unpaired players
	if (!(pp.hasPlayer2())) continue;
	
	// Skip true mixed pairs
	if (pp.getPlayer1().getSex() != pp.getPlayer2().getSex()) continue;

	// check if we can split "false" mixed pairs (= same sex pairs)
	cmngr->splitPlayers(c, pp.getPairId());
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
    QVariantList qvl;
    qvl << P2C_CAT_REF << c.getId();
    qvl << P2C_PLAYER_REF << p.getId();
    db->getTab(TAB_P2C).insertRow(qvl);
    
    emit playerAddedToCategory(p, c);
    
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
    QVariantList qvl;
    qvl << P2C_CAT_REF << c.getId();
    qvl << P2C_PLAYER_REF << p.getId();
    db->getTab(TAB_P2C).deleteRowsByColumnValue(qvl);
    
    emit playerRemovedFromCategory(p, c);
    
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
    assert(db->getTab(TAB_P2C).getMatchCountForColumnValue(P2C_CAT_REF, catId) == 0);
    assert(db->getTab(TAB_PAIRS).getMatchCountForColumnValue(PAIRS_CAT_REF, catId) == 0);
    assert(db->getTab(TAB_MATCH_GROUP).getMatchCountForColumnValue(MG_CAT_REF, catId) == 0);
    assert(db->getTab(TAB_RANKING).getMatchCountForColumnValue(RA_CAT_REF, catId) == 0);
    assert(db->getTab(TAB_BRACKET_VIS).getMatchCountForColumnValue(BV_CAT_REF, catId) == 0);

    // the actual deletion
    int oldSeqNum = cat.getSeqNum();
    emit beginDeleteCategory(oldSeqNum);
    catTab.deleteRowsByColumnValue("id", cat.getId());
    fixSeqNumberAfterDelete(TAB_CATEGORY, oldSeqNum);
    emit endDeleteCategory();

    return OK;
  }

//----------------------------------------------------------------------------

  Category CatMngr::getCategoryById(int id)
  {
    try {
      TabRow r = catTab[id];
      return Category(db, r);
      }
    catch (std::exception e)
    {
     throw std::invalid_argument("The category with ID " + QString2String(QString::number(id)) + " does not exist");
    }
  }

//----------------------------------------------------------------------------

  Category CatMngr::getCategoryBySeqNum(int seqNum)
  {
    try {
      TabRow r = catTab.getSingleRowByColumnValue(GENERIC_SEQNUM_FIELD_NAME, seqNum);
      return Category(db, r);
    }
    catch (std::exception e)
    {
     throw std::invalid_argument("The category with sequence number " + QString2String(QString::number(seqNum)) + " does not exist");
    }
  }

//----------------------------------------------------------------------------

  QHash<Category, CAT_ADD_STATE> CatMngr::getAllCategoryAddStates(SEX s)
  {
    QList<Category> allCat = getAllCategories();
    QHash<Category, CAT_ADD_STATE> result;
    
    QList<Category>::const_iterator it;
    for (it = allCat.begin(); it != allCat.constEnd(); ++it)
    {
      result[(*it)] = (*it).getAddState(s);
    }
    
    return result;
  }

//----------------------------------------------------------------------------

  QHash<Category, CAT_ADD_STATE> CatMngr::getAllCategoryAddStates(const Player& p)
  {
    QList<Category> allCat = getAllCategories();
    QHash<Category, CAT_ADD_STATE> result;
    
    QList<Category>::const_iterator it;
    for (it = allCat.begin(); it != allCat.constEnd(); ++it)
    {
      result[(*it)] = (*it).getAddState(p);
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

      c.row.update(CAT_GROUP_CONFIG, v.toString());
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
    QVariantList qvl;
    qvl << PAIRS_CAT_REF << c.getId();
    qvl << PAIRS_PLAYER1_REF << p1.getId();
    qvl << PAIRS_PLAYER2_REF << p2.getId();
    qvl << PAIRS_GRP_NUM << GRP_NUM__NOT_ASSIGNED;   // Default value: no group

    db->getTab(TAB_PAIRS).insertRow(qvl);
    
    emit playersPaired(c, p1, p2);
    
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
    QVariantList qvl;
    qvl << PAIRS_CAT_REF << c.getId();
    qvl << PAIRS_PLAYER1_REF << p1.getId();
    qvl << PAIRS_PLAYER2_REF << p2.getId();
    DbTab pairsTab = db->getTab(TAB_PAIRS);
    pairsTab.deleteRowsByColumnValue(qvl);
    qvl.clear();
    qvl << PAIRS_CAT_REF << c.getId();
    qvl << PAIRS_PLAYER1_REF << p2.getId();
    qvl << PAIRS_PLAYER2_REF << p1.getId();
    pairsTab.deleteRowsByColumnValue(qvl);
    
    emit playersSplit(c, p1, p2);
    
    return OK;
  }


//----------------------------------------------------------------------------

  ERR CatMngr::splitPlayers(const Category c, int pairId) const
  {
    DbTab pairsTab = db->getTab(TAB_PAIRS);
    PlayerMngr* pmngr = Tournament::getPlayerMngr();
    try
    {
      TabRow r = pairsTab[pairId];
      Player p1 = pmngr->getPlayer(r[PAIRS_PLAYER1_REF].toInt());
      Player p2 = pmngr->getPlayer(r[PAIRS_PLAYER2_REF].toInt());
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
    
    c.row.update(GENERIC_NAME_FIELD_NAME, newName);
    
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
    
    // Okax, we're good to go
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
    for (int i=0; i < ppList.count(); i++)
    {
      PlayerPair pp = ppList.at(i);
      if (!(pp.hasPlayer2()))
      {
        int playerId = pp.getPlayer1().getId();

        QVariantList qvl;
        qvl << PAIRS_CAT_REF << catId;
        qvl << PAIRS_GRP_NUM << GRP_NUM__NOT_ASSIGNED;
        qvl << PAIRS_PLAYER1_REF << playerId;
        // leave out PAIRS_PLAYER2_REF to assign a NULL value

        db->getTab(TAB_PAIRS).insertRow(qvl);
      }
    }
    
    // update the category state
    OBJ_STATE oldState = c.getState();  // this MUST be STAT_CAT_CONFIG, ensured by canFreezeConfig
    c.setState(STAT_CAT_FROZEN);
    emit categoryStatusChanged(c, oldState, STAT_CAT_FROZEN);
    
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
    DbTab ppTab = db->getTab(TAB_PAIRS);
    for (int i=0; i < ppList.count(); i++)
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
      ppTab.deleteRowsByColumnValue("id", ppId);
    }
    // update the category state
    c.setState(STAT_CAT_CONFIG);
    emit categoryStatusChanged(c, STAT_CAT_FROZEN, STAT_CAT_CONFIG);
    
    return OK;
  }


//----------------------------------------------------------------------------

  ERR CatMngr::startCategory(const Category &c, QList<PlayerPairList> grpCfg, PlayerPairList seed, ProgressQueue *progressNotificationQueue)
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
    emit categoryStatusChanged(c, STAT_CAT_FROZEN, STAT_CAT_IDLE);

    // do the individual prep of the first round
    ERR result = specializedCat->prepareFirstRound(progressNotificationQueue);

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

    // determine whether we have at least one RUNING and/or one
    // unfinished match in the category
    auto mm = Tournament::getMatchMngr();
    bool hasMatchRunning = false;
    bool hasUnfinishedMatch = false;
    for (auto mg : mm->getMatchGroupsForCat(c))
    {
      hasMatchRunning = mg.hasMatchesInState(STAT_MA_RUNNING);
      hasUnfinishedMatch = mg.hasMatches__NOT__InState(STAT_MA_FINISHED);

      if (hasMatchRunning) break;
    }

    // if we're IDLE and least one match is being played,
    // change state to PLAYING
    if ((curStat == STAT_CAT_IDLE) && hasMatchRunning)
    {
      c.setState(STAT_CAT_PLAYING);
      emit categoryStatusChanged(c, STAT_CAT_IDLE, STAT_CAT_PLAYING);
      return;
    }

    // check if the whole category is finished
    CatRoundStatus crs = c.getRoundStatus();
    int lastFinishedRound = crs.getFinishedRoundsCount();
    int totalRounds = crs.getTotalRoundsCount();
    bool catIsFinished = ((totalRounds > 0) && (totalRounds == lastFinishedRound));

    // if we've finished the last round
    // change state to FINALIZED
    if ((curStat == STAT_CAT_PLAYING) && catIsFinished)
    {
      {
        c.setState(STAT_CAT_FINALIZED);
        emit categoryStatusChanged(c, STAT_CAT_PLAYING, STAT_CAT_FINALIZED);
        return;
      }
    }

    // if we're PLAYING and where not finished
    // change state back to IDLE
    if ((curStat == STAT_CAT_PLAYING) && !catIsFinished && !hasMatchRunning)
    {
      c.setState(STAT_CAT_IDLE);
      emit categoryStatusChanged(c, STAT_CAT_PLAYING, STAT_CAT_IDLE);
      return;
    }
  }

//----------------------------------------------------------------------------

  bool CatMngr::switchCatToWaitForSeeding(const Category& cat)
  {
    // only switch to SEEDING if no match is currently running
    if (cat.getState() != STAT_CAT_IDLE) return false;

    cat.setState(STAT_CAT_WAIT_FOR_INTERMEDIATE_SEEDING);
    emit categoryStatusChanged(cat, STAT_CAT_IDLE, STAT_CAT_WAIT_FOR_INTERMEDIATE_SEEDING);
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

  PlayerPairList CatMngr::getSeeding(const Category& c) const
  {
    // as long as the category is still in configuration, we can't rely
    // on the existence of valid player pairs in the database and thus
    // we'll return an empty list as an error indicator
    if (c.getState() == STAT_CAT_CONFIG) return PlayerPairList();

    // get the player pairs for the category
    DbTab pairTab = (*db)[TAB_PAIRS];
    QString where = PAIRS_CAT_REF + " = ? ORDER BY " + PAIRS_INITIAL_RANK + " ASC";
    QVariantList qvl;
    qvl << c.getId();

    return getObjectsByWhereClause<PlayerPair>(pairTab, where, qvl);
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
    emit categoryStatusChanged(c, STAT_CAT_WAIT_FOR_INTERMEDIATE_SEEDING, STAT_CAT_IDLE);

    return OK;
  }

//----------------------------------------------------------------------------


}
