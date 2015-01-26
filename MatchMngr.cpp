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
    emit beginCreateMatchGroup();
    int newId = groupTab.insertRow(qvl);
    fixSeqNumberAfterInsert(TAB_MATCH_GROUP);
    emit endCreateMatchGroup(groupTab.length() - 1); // the new sequence number is always the greatest

    
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

  /**
    Closes a match group so that no further matches can be added. This promotes
    the match group from CONFIG to FROZEN.

    Also call updateAllMatchGroups() in case the match group can be directly
    promoted further from FROZEN to IDLE

    \param grp is the match group to close

    \return error code
    */
  ERR MatchMngr::closeMatchGroup(const MatchGroup &grp)
  {
    // we can only close match groups that are in state CONFIG
    if (grp.getState() != STAT_MG_CONFIG) return MATCH_GROUP_ALREADY_CLOSED;

    // the match group should contain at least one match
    if (grp.getMatchCount() < 1) return MATCH_GROUP_EMPTY;

    // we can close the group unconditionally
    grp.setState(STAT_MG_FROZEN);
    // TODO: emit signal

    // call updateAllMatchGroupStates in case the group can be further promoted
    // to idle (which enables the group the be scheduled)
    updateAllMatchGroupStates(grp.getCategory());

    return OK;
  }

//----------------------------------------------------------------------------

  /**
    Checks if any match groups within a category can be promoted to a higher
    state. This can be, for example:
      * from SCHEDULED to FINISHED if all matches have been played; or
      * from FROZEN to IDLE of all predecessor rounds have been STAGED

    \param cat is the category to check the match groups for
    \return nothing (void)
    */
  void MatchMngr::updateAllMatchGroupStates(const Category &cat) const
  {
    MatchGroupList mgl = getMatchGroupsForCat(cat);

    // TODO: the following algorithm is not very efficient. Instead of
    // looping through all match groups using "for", a dedicated
    // SQL-statement with a suitable WHERE-clause would be better

    // transition from SCHEDULED to FINISHED
    for (auto mg : mgl)
    {
      if (mg.getState() != STAT_MG_SCHEDULED) continue;

      // check all matches in this scheduled category
      bool isfinished = true;
      for (auto match : mg.getMatches())
      {
        if (match.getState() != STAT_MA_FINISHED)
        {
          isfinished = false;
          break;
        }
      }
      if (isfinished)
      {
        mg.setState(STAT_MG_FINISHED);
        // TODO: emit signal
      }
    }

    // transition from FROZEN to IDLE
    for (auto mg : mgl)
    {
      if (mg.getState() != STAT_MG_FROZEN) continue;

      // Condition: all match groups of the same players group with lower
      // round numbers must be staged
      int round = mg.getRound();
      bool canPromote = true;
      for (int r=1; r < round; ++r)
      {
        MatchGroupList mglSubset = getMatchGroupsForCat(cat, r);
        for (auto mg2 : mglSubset)
        {
          int mgGroupNum = mg.getGroupNumber();
          int mg2GroupNum = mg2.getGroupNumber();

          // skip match groups associated with other player groups
          //
          // only applies to "real" player groups (group num > 0)
          if ((mgGroupNum > 0) && (mg2GroupNum > 0) && (mg2.getGroupNumber() != mg.getGroupNumber())) continue;

          OBJ_STATE mg2Stat = mg2.getState();
          if (!((mg2Stat == STAT_MG_STAGED) || (mg2Stat == STAT_MG_SCHEDULED) || (mg2Stat == STAT_MA_FINISHED)))
          {
            canPromote = false;
            break;
          }
        }

        if (!canPromote) break;  // no need to loop through other rounds
      }

      if (canPromote)
      {
        mg.setState(STAT_MG_IDLE);
        // TODO: emit signal
      }
    }

  }

//----------------------------------------------------------------------------

  unique_ptr<MatchGroup> MatchMngr::getMatchGroupBySeqNum(int mgSeqNum)
  {
    try {
      TabRow r = groupTab.getSingleRowByColumnValue(GENERIC_SEQNUM_FIELD_NAME, mgSeqNum);
      MatchGroup* mg_raw = new MatchGroup(db, r.getId());
      return unique_ptr<MatchGroup>(mg_raw);
    }
    catch (std::exception e)
    {
     return nullptr;  // null indicates error
    }
    return nullptr;
  }

//----------------------------------------------------------------------------

  /**
    Try to promote the match group from IDLE to STAGED.

    Also call updateAllMatchGroups() in case other match group can be
    subsequently promoted from FROZEN to IDLE

    \param grp is the match group to stage

    \return error code
    */
  ERR MatchMngr::stageMatchGroup(const MatchGroup &grp)
  {
    if (grp.getState() != STAT_MG_IDLE) return WRONG_STATE;

    // no further checks necessary. Any IDLE match group can be promoted

    // promote the group to STAGED and assign a sequence number
    int nextStageSeqNum = getMaxStageSeqNum() + 1;
    grp.setState(STAT_MG_STAGED);
    int grpId = grp.getId();
    TabRow r = groupTab[grpId];
    r.update(MG_STAGE_SEQ_NUM, nextStageSeqNum);
    // TODO:
    // emit signal

    // promote other groups from FROZEN to IDLE, if applicable
    updateAllMatchGroupStates(grp.getCategory());

    return OK;
  }

//----------------------------------------------------------------------------

  /**
   * Determines the highest currently assigned sequence number in the staging area
   *
   * @return the highest assigned sequence number of all staged match groups or 0 if no group is staged
   */
  int MatchMngr::getMaxStageSeqNum() const
  {
    // Is there any staged match group at all?
    int statId = static_cast<int>(STAT_MG_STAGED);
    if (groupTab.getMatchCountForColumnValue(GENERIC_STATE_FIELD_NAME, statId) < 1)
    {
      return 0;  // no staged match groups so far
    }

    // determine the max sequence number
    QString sql = "SELECT max(" + MG_STAGE_SEQ_NUM + ") FROM " + TAB_MATCH_GROUP;
    QVariant result = db->execScalarQuery(sql);
    if ( (!(result.isValid())) || (result.isNull()))
    {
      return 0;  // shouldn't happen, but anyway...
    }

    return result.toInt();
  }

//----------------------------------------------------------------------------

  /**
   * Checks whether a match group can be unstaged or not. In order to demote
   * a match group from STAGED to IDLE, it must have the highest round number
   * of all other match groups in this category and (possibly) in the same
   * players group.
   *
   * @param grp the group to check for unstaging
   *
   * @return error code
   */
  ERR MatchMngr::canUnstageMatchGroup(const MatchGroup &grp)
  {
    // first precondition: match group has to be staged
    if (grp.getState() != STAT_MG_STAGED)
    {
      return MATCH_GROUP_NOT_UNSTAGEABLE;
    }

    int round = grp.getRound();
    int playersGroup = grp.getGroupNumber();
    int catId = grp.getCategory().getId();

    // check for a match group with "round + 1" in the staging area
    QVariantList qvl;
    qvl << MG_ROUND << round+1;
    qvl << GENERIC_STATE_FIELD_NAME << static_cast<int>(STAT_MG_STAGED);
    qvl << MG_CAT_REF << catId;
    if (groupTab.getMatchCountForColumnValue(qvl) == 0)
    {
      // there is no match group of this category and with a higher
      // round number in the staging area
      // ==> the match group can be demoted
      return OK;
    }

    //
    // obviously there is at least one match group of this category
    // and with round number round+1 staged
    //

    // now compare the players groups to properly check the transition
    // between round robin rounds and KO-rounds
    //
    if (playersGroup < 0)
    {
      // this group is already in KO phase, so the next one
      // must be as well. So the fact that there is a group
      // with "round+1" already staged is a no-go for unstaging
      // this group
      return MATCH_GROUP_NOT_UNSTAGEABLE;
    }

    // if we made it to this point, at least the current
    // match group "grp" is still in round robin phase
    //
    // let's see if all staged match groups with "round+1" are
    // in different players groups. If yes, then it's okay
    // to unstage this match group
    //
    // if the match group(s) with "round+1" are of the
    // same players group or already in KO phase, we can't
    // demote this match group
    auto it = groupTab.getRowsByColumnValue(qvl);
    while (!(it.isEnd()))
    {
      auto mg = MatchGroup(db, *it);
      int nextGroupNumber = mg.getGroupNumber();
      if (nextGroupNumber < 0)
      {
        // next round is already in KO phase
        return MATCH_GROUP_NOT_UNSTAGEABLE;
      }

      // the next round's group belongs to the same
      // players group ==> can't unstage
      if (nextGroupNumber == playersGroup)
      {
        return MATCH_GROUP_NOT_UNSTAGEABLE;
      }
      ++it;
    }

    return OK;
  }

//----------------------------------------------------------------------------

  /**
   * Tries to demote a match group from STAGED to IDLE. If this demotion is
   * possible, other match groups will automatically be demoted from IDLE
   * to FROZEN, if applicable
   *
   * @param grp the group to demote
   *
   * @return error code
   */
  ERR MatchMngr::unstageMatchGroup(const MatchGroup &grp)
  {
    // check all preconditions for a demotion
    ERR e = canUnstageMatchGroup(grp);
    if (e != OK) return e;

    // great, we can safely demote this match group to IDLE
    grp.setState(STAT_MG_IDLE);
    // TODO: emit signal

    // store and delete old stage sequence number
    int oldStageSeqNumber = grp.getStageSequenceNumber();
    int grpId = grp.getId();
    TabRow r = groupTab[grpId];
    r.update(MG_STAGE_SEQ_NUM, QVariant());
    // TODO: emit signal

    // update all subsequent sequence numbers
    QString where = MG_STAGE_SEQ_NUM + " > " + QString::number(oldStageSeqNumber);
    auto it = groupTab.getRowsByWhereClause(where);
    while (!(it.isEnd()))
    {
      auto mg = MatchGroup(db, *it);
      int old = mg.getStageSequenceNumber();
      (*it).update(MG_STAGE_SEQ_NUM, old - 1);
      // TODO: emit signal
      ++it;
    }

    // demote other rounds from IDLE to FROZEN
    // check for a match group with "round + 1" in state IDLE
    int round = grp.getRound();
    int playersGroup = grp.getGroupNumber();
    int catId = grp.getCategory().getId();

    QVariantList qvl;
    qvl << MG_ROUND << round+1;
    qvl << GENERIC_STATE_FIELD_NAME << static_cast<int>(STAT_MG_IDLE);
    qvl << MG_CAT_REF << catId;
    it = groupTab.getRowsByColumnValue(qvl);
    while (!(it.isEnd()))
    {
      auto mg = MatchGroup(db, *it);

      // if our demoted match group and the IDLE match group are round-robin-groups,
      // the IDLE match group has only to be demoted if it matches the "grp's" players group
      //
      // or in inverse logic: if the player groups to not match, we don't have to
      // touch this group
      int otherPlayersGroup = mg.getGroupNumber();
      if ((playersGroup > 0) && (otherPlayersGroup > 0) && (playersGroup != otherPlayersGroup))
      {
        ++it;
        continue;
      }

      // in all other cases, the "IDLE" group has to be demoted to FROZEN
      mg.setState(STAT_MG_FROZEN);
      // TODO: emit signal

      ++it;
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


}
