/* 
 * File:   MatchMngr.cpp
 * Author: volker
 * 
 * Created on August 22, 2014, 7:32 PM
 */

#include <assert.h>
#include "MatchMngr.h"
#include "Tournament.h"

using namespace dbOverlay;

namespace QTournament {

  MatchMngr::MatchMngr(TournamentDB* _db)
  : GenericObjectManager(_db), matchTab((*db)[TAB_MATCH]), groupTab((*db)[TAB_MATCH_GROUP])
  {
  }

//----------------------------------------------------------------------------

  unique_ptr<MatchGroup> MatchMngr::createMatchGroup(const Category& cat, const int round, const int grpNum, ERR *err)
  {
    assert(err != nullptr);

    // we can only create match groups, if the category configuration is stable
    // this means, we may not be in STAT_CAT_CONFIG or _FROZEN
    OBJ_STATE catState = cat.getState();
    if ((catState == STAT_CAT_CONFIG) || (catState == STAT_CAT_FROZEN))
    {
      *err = CATEGORY_STILL_CONFIGURABLE;
      return nullptr;
    }

    // check parameters for validity
    if (grpNum <= 0)
    {
      if ((grpNum != GROUP_NUM__FINAL)
       && (grpNum != GROUP_NUM__SEMIFINAL)
       && (grpNum != GROUP_NUM__QUARTERFINAL)
       && (grpNum != GROUP_NUM__L16)
       && (grpNum != GROUP_NUM__ITERATION))
      {
        *err = INVALID_GROUP_NUM;
        return nullptr;
      }
    }

    if (round <= 0)
    {
      *err = INVALID_ROUND;
      return nullptr;
    }

    // ensure that we don't mix "normal" group numbers with "special" group numbers
    if (grpNum<= 0)
    {
      // in this case, no other match groups in this round may exist, i. e.
      // there can only be one match group for semi finals
      int nOtherGroups = getMatchGroupsForCat(cat, round).count();
      if (nOtherGroups != 0)
      {
        *err = INVALID_GROUP_NUM;
        return nullptr;
      }
    }
    if (grpNum > 0)
    {
      // in this case, no other match groups with group numbers
      // below zero (e. g. semi finals) may exist
      QString where = MG_GRP_NUM + " <= 0 AND " + MG_ROUND + " = " + QString::number(round) + " AND " + MG_CAT_REF + " = " + QString::number(cat.getId());
      int nOtherGroups = db->getTab(TAB_MATCH_GROUP).getMatchCountForWhereClause(where);
      if (nOtherGroups != 0)
      {
        *err = INVALID_GROUP_NUM;
        return nullptr;
      }
    }

    // make sure the match group doesn't already exist
    ERR e;
    unique_ptr<MatchGroup> mg = getMatchGroup(cat, round, grpNum, &e);
    if (e == OK)    // match group exists
    {
      *err = MATCH_GROUP_EXISTS;
      return nullptr;
    }
    if (e != NO_SUCH_MATCH_GROUP)   // catch any other error except "no such group"
    {
      *err = e;
      return nullptr;
    }

    
    /*
     * Theoretically, we would need more logic checks here. Examples:
     *   - create no match group with a round number that's less than the number of an already finished / running round
     *   - don't have e.g. semi finals in an earlier round than quarter finals
     *   - etc.
     * 
     * But for now, let's skip that ;)
     */
    
    // Okay, parameters are valid
    // create a new match group entry in the database
    QVariantList qvl;
    qvl << MG_CAT_REF << cat.getId();
    qvl << MG_ROUND << round;
    qvl << MG_GRP_NUM << grpNum;
    qvl << GENERIC_STATE_FIELD_NAME << STAT_MG_CONFIG;
    int newId = groupTab.insertRow(qvl);
    
    // create a match group object for the new group an return a pointer
    // to this new object
    MatchGroup* mg_raw = new MatchGroup(db, newId);
    *err = OK;
    return unique_ptr<MatchGroup>(mg_raw);
  }

//----------------------------------------------------------------------------

  MatchGroupList MatchMngr::getMatchGroupsForCat(const Category& cat, int round) const
  {
    QVariantList qvl;
    qvl << MG_CAT_REF << cat.getId();
    if (round > 0)
    {
      qvl << MG_ROUND << round;
    }
    DbTab::CachingRowIterator it = groupTab.getRowsByColumnValue(qvl);
    
    MatchGroupList result;
    while (!(it.isEnd()))
    {
      result << MatchGroup(db, *it);
      ++it;
    }
    
    return result;
  }
  
//----------------------------------------------------------------------------

  MatchGroupList MatchMngr::getAllMatchGroups()
  {
    MatchGroupList result;
    
    DbTab::CachingRowIterator it = groupTab.getAllRows();
    while (!(it.isEnd()))
    {
      result << MatchGroup(db, *it);
      ++it;
    }
    
    return result;
  }

//----------------------------------------------------------------------------

  unique_ptr<MatchGroup> MatchMngr::getMatchGroup(const Category& cat, const int round, const int grpNum, ERR *err)
  {
    assert(err != nullptr);

    // check round parameter for validity
    if (round <= 0)
    {
      *err = INVALID_ROUND;
      return nullptr;
    }
    
    // check group parameter for validity
    if (grpNum <= 0)
    {
      if ((grpNum != GROUP_NUM__FINAL)
	   && (grpNum != GROUP_NUM__SEMIFINAL)
	   && (grpNum != GROUP_NUM__QUARTERFINAL)
	   && (grpNum != GROUP_NUM__L16)
       && (grpNum != GROUP_NUM__ITERATION))
      {
        *err = INVALID_GROUP_NUM;
        return nullptr;
      }
    }
    
    QVariantList qvl;
    qvl << MG_CAT_REF << cat.getId();
    qvl << MG_ROUND << round;
    qvl << MG_GRP_NUM << grpNum;
    
    try
    {
      TabRow r = groupTab.getSingleRowByColumnValue(qvl);
      MatchGroup* grp = new MatchGroup(db, r);
      *err = OK;
      return unique_ptr<MatchGroup>(grp);
    } catch (exception e) {
      *err = NO_SUCH_MATCH_GROUP;
      return nullptr;
    }
  }

//----------------------------------------------------------------------------

  bool MatchMngr::hasMatchGroup(const Category& cat, const int round, const int grpNum, ERR* err)
  {
    ERR e;
    auto mg = getMatchGroup(cat, round, grpNum, &e);
    
    if (e == OK)
    {
      if (err != nullptr) *err = OK;
      return true;
    }
    
    if (err != nullptr) *err = e;
    return false;
  }

//----------------------------------------------------------------------------

  unique_ptr<Match> MatchMngr::createMatch(const MatchGroup &grp, ERR* err)
  {
    assert(err != nullptr);

    // we can only add matches to a group if the group
    // is in the config state
    if (grp.getState() != STAT_MG_CONFIG)
    {
      *err = MATCH_GROUP_NOT_CONFIGURALE_ANYMORE;
      return nullptr;
    }

    // Okay, parameters are valid
    // create a new match entry in the database
    QVariantList qvl;
    qvl << MA_GRP_REF << grp.getId();
    qvl << GENERIC_STATE_FIELD_NAME << STAT_MA_INCOMPLETE;
    int newId = matchTab.insertRow(qvl);

    // create a match group object for the new group and return a pointer
    // to this new object
    Match* ma = new Match(db, newId);
    *err = OK;
    return unique_ptr<Match>(ma);
  }

//----------------------------------------------------------------------------

  ERR MatchMngr::setPlayerPairsForMatch(const Match &ma, const PlayerPair &pp1, const PlayerPair &pp2)
  {
    // check if an assignment of the player pairs is okay
    ERR e = canAssignPlayerPairToMatch(ma, pp1);
    if (e != OK) return e;
    e = canAssignPlayerPairToMatch(ma, pp2);
    if (e != OK) return e;

    // make sure that both pairs are not identical
    if (pp1.getPairId() == pp2.getPairId())
    {
      return PLAYERS_IDENTICAL;
    }

    // assign the player pairs
    TabRow matchRow = matchTab[ma.getId()];
    matchRow.update(MA_PAIR1_REF, pp1.getPairId());
    matchRow.update(MA_PAIR2_REF, pp2.getPairId());

    return OK;
  }

//----------------------------------------------------------------------------

  ERR MatchMngr::canAssignPlayerPairToMatch(const Match &ma, const PlayerPair &pp) const
  {
    // Only allow changing / setting player pairs if we not yet fully configured
    if (ma.getState() != STAT_MA_INCOMPLETE) return MATCH_NOT_CONFIGURALE_ANYMORE;

    // make sure the player pair and the match belong to the same category
    Category requiredCat = ma.getCategory();
    auto ppCat = pp.getCategory(db);
    if (*ppCat != requiredCat) return PLAYER_NOT_IN_CATEGORY;

    // in case we're using group matches, make sure that the player's group
    // and the match's group are identical
    //
    //
    // Design decision:
    // We do accept "pairs with regular group number" in match groups with
    // "special group number". This case may occur after the transition from
    // a group match phase into finals because in the finals the pairs would still
    // have their group numbers assigned.
    //
    // So the check below is sufficient to cover all cases:
    //   * if mgGroupNumer is below or equal zero, we don't care (see above)
    //   * if it's greater than zero, the group numbers of player pair and
    //     match group must be identical
    auto mg = ma.getMatchGroup();
    int mgGroupNumer = mg.getGroupNumber();
    if ((mgGroupNumer > 0) && (pp.getPairsGroupNum(db) != mgGroupNumer))
    {
      return GROUP_NUMBER_MISMATCH;
    }

    // make sure the player pair has not already been assigned
    // to other matches in the same category and round
    MatchGroup myMatchGroup = ma.getMatchGroup();
    int roundNum = myMatchGroup.getRound();
    MatchGroupList mgl = getMatchGroupsForCat(requiredCat, roundNum);
    for (MatchGroup mg : mgl)
    {
      for (Match m : mg.getMatches())
      {
        // skip the match we want to assign the players to
        if (m == ma) continue;

        // check the first player pair of this match, if existing
        if (m.hasPlayerPair1())
        {
          int ppId = m.getPlayerPair1().getPairId();
          if (ppId == pp.getPairId()) return PLAYER_ALREADY_ASSIGNED_TO_OTHER_MATCH_IN_THE_SAME_ROUND_AND_CATEGORY;
        }

        // check the second  player pair of this match, if existing
        if (m.hasPlayerPair2())
        {
          int ppId = m.getPlayerPair2().getPairId();
          if (ppId == pp.getPairId()) return PLAYER_ALREADY_ASSIGNED_TO_OTHER_MATCH_IN_THE_SAME_ROUND_AND_CATEGORY;
        }
      }
    }

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


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


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
