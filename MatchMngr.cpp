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
    emit beginCreateMatch();
    int newId = matchTab.insertRow(qvl);
    fixSeqNumberAfterInsert(TAB_MATCH);
    emit endCreateMatch(matchTab.length() - 1); // the new sequence number is always the highest

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
    emit matchGroupStatusChanged(grp.getId(), grp.getSeqNum(), STAT_MG_CONFIG, STAT_MG_FROZEN);

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
        emit matchGroupStatusChanged(mg.getId(), mg.getSeqNum(), STAT_MG_SCHEDULED, STAT_MG_FINISHED);
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
          if (!((mg2Stat == STAT_MG_STAGED) || (mg2Stat == STAT_MG_SCHEDULED) || (mg2Stat == STAT_MG_FINISHED)))
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
        emit matchGroupStatusChanged(mg.getId(), mg.getSeqNum(), STAT_MG_FROZEN, STAT_MG_IDLE);
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
    ERR e = canStageMatchGroup(grp);
    if (e != OK) return e;

    // no further checks necessary. Any IDLE match group can be promoted

    // promote the group to STAGED and assign a sequence number
    int nextStageSeqNum = getMaxStageSeqNum() + 1;
    grp.setState(STAT_MG_STAGED);
    int grpId = grp.getId();
    TabRow r = groupTab[grpId];
    r.update(MG_STAGE_SEQ_NUM, nextStageSeqNum);
    emit matchGroupStatusChanged(grp.getId(), grp.getSeqNum(), STAT_MG_IDLE, STAT_MG_STAGED);

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

    // store and delete old stage sequence number
    int oldStageSeqNumber = grp.getStageSequenceNumber();
    int grpId = grp.getId();
    TabRow r = groupTab[grpId];
    r.update(MG_STAGE_SEQ_NUM, QVariant());

    emit matchGroupStatusChanged(grp.getId(), grp.getSeqNum(), STAT_MG_STAGED, STAT_MG_IDLE);

    // update all subsequent sequence numbers
    QString where = MG_STAGE_SEQ_NUM + " > " + QString::number(oldStageSeqNumber);
    auto it = groupTab.getRowsByWhereClause(where);
    while (!(it.isEnd()))
    {
      auto mg = MatchGroup(db, *it);
      int old = mg.getStageSequenceNumber();
      (*it).update(MG_STAGE_SEQ_NUM, old - 1);
      emit matchGroupStatusChanged(mg.getId(), mg.getSeqNum(), STAT_MG_STAGED, STAT_MG_STAGED);
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
      emit matchGroupStatusChanged(mg.getId(), mg.getSeqNum(), STAT_MG_IDLE, STAT_MG_FROZEN);

      ++it;
    }

    return OK;
  }

//----------------------------------------------------------------------------

  /**
   * Checks whether a match group can be Staged or not.
   * Currently this is only a check whether the match group is in state
   * IDLE or not
   *
   * @param grp the group to check for unstaging
   *
   * @return error code
   */
  ERR MatchMngr::canStageMatchGroup(const MatchGroup &grp)
  {
    if (grp.getState() != STAT_MG_IDLE)
    {
      return WRONG_STATE;
    }

    return OK;
  }

//----------------------------------------------------------------------------

  /**
   * Checks whether the necessary conditions are met to promote a match
   * to a higher state
   *
   * @param ma the match to check/promote
   */
  void MatchMngr::updateMatchStatus(const Match &ma) const
  {
    OBJ_STATE curState = ma.getState();

    // from INCOMPLETE to WAITING
    if (curState == STAT_MA_INCOMPLETE)
    {
      if (ma.hasBothPlayerPairs() && (ma.getMatchNumber() != MATCH_NUM_NOT_ASSIGNED))
      {
        ma.setState(STAT_MA_WAITING);
        curState = STAT_MA_WAITING;
        emit matchStatusChanged(ma.getId(), ma.getSeqNum(), STAT_MA_INCOMPLETE, STAT_MA_WAITING);
      }
    }

    // TODO: handle FUZZY (not yet implemented in the database)

    // from WAITING to READY or BUSY
    bool hasPredecessor = hasMandatoryPredecessor(ma);
    bool playersAvail = ma.hasAllPlayersIdle();
    if ((curState == STAT_MA_WAITING) && (!hasPredecessor))
    {
      curState = playersAvail ? STAT_MA_READY : STAT_MA_BUSY;
      ma.setState(curState);
      emit matchStatusChanged(ma.getId(), ma.getSeqNum(), STAT_MA_WAITING, curState);
    }

    // from READY to BUSY
    if ((curState == STAT_MA_READY) && !playersAvail)
    {
      ma.setState(STAT_MA_BUSY);
      curState = STAT_MA_BUSY;
      emit matchStatusChanged(ma.getId(), ma.getSeqNum(), STAT_MA_READY, STAT_MA_BUSY);
    }

    // from BUSY to READY
    if ((curState == STAT_MA_BUSY) && playersAvail)
    {
      ma.setState(STAT_MA_READY);
      curState = STAT_MA_READY;
      emit matchStatusChanged(ma.getId(), ma.getSeqNum(), STAT_MA_BUSY, STAT_MA_READY);
    }

    // RUNNING is handled separately
    // FINISHED is handled separately
    // POSTPONED is handled separately
  }

//----------------------------------------------------------------------------

  /**
   * Checks whether a match can potentially be called, given that all players are
   * available (which is not checked here).
   * Essentially this boils down to check if there any matches in earlier rounds
   * that have to be played first
   *
   * @param ma the match to check
   * @return true if earlier matches have to be played first, false otherwise
   */
  bool MatchMngr::hasMandatoryPredecessor(const Match &ma) const
  {
    auto mm = Tournament::getMatchMngr();
    auto mg = ma.getMatchGroup();

    int round = mg.getRound();
    if (round < 2)
    {
      // matches in round 1 can always be played
      return false;
    }

    // okay, our match is at least in round 2. Check for unplayed matches
    // in round-1
    auto cat = ma.getCategory();
    int prevRoundsPlayersGroup = cat.getGroupNumForPredecessorRound(mg.getGroupNumber());
    for (auto prevMg : getMatchGroupsForCat(cat, round-1))
    {
      int playersGroup = prevMg.getGroupNumber();
      if ((prevRoundsPlayersGroup == ANY_PLAYERS_GROUP_NUMBER) && (playersGroup < 0))
      {
        continue;  // we're looking for any round robins, but this a KO group or an iterative group
      }

      if ((prevRoundsPlayersGroup > 0) && (playersGroup != prevRoundsPlayersGroup))
      {
        // we're looking for a specific players group in round robins,
        // but this a KO group or an iterative group or a wrong
        // players group
        continue;
      }

      if (((prevRoundsPlayersGroup == GROUP_NUM__SEMIFINAL) ||
           (prevRoundsPlayersGroup == GROUP_NUM__QUARTERFINAL) ||
           (prevRoundsPlayersGroup == GROUP_NUM__L16)) && (playersGroup != prevRoundsPlayersGroup))
      {
        continue;  // wrong KO round
      }

      // if we made it to this point, the match group has to be finished befor
      // the match can be called
      if (prevMg.getState() != STAT_MG_FINISHED) return true;
    }

    return false;   // no match group found that has to be finished before the match
  }

//----------------------------------------------------------------------------

  /**
   * Assigns match numbers to all matches in all currently staged match groups and
   * clears the staging area
   */
  void MatchMngr::scheduleAllStagedMatchGroups() const
  {
    int nextMatchNumber = getMaxMatchNum() + 1;

    for (auto mg : getStagedMatchGroupsOrderedBySequence())
    {
      for (auto ma : mg.getMatches())
      {
        int matchId = ma.getId();
        TabRow r = matchTab[matchId];
        r.update(MA_NUM, nextMatchNumber);
        updateMatchStatus(ma);

        // Manually trigger (another) update, because assigning the match number
        // does not change the match state in all cases. So we need to have at
        // least this one trigger to tell everone that the data has changed
        emit matchStatusChanged(matchId, ma.getSeqNum(), ma.getState(), ma.getState());

        ++nextMatchNumber;
      }

      // update the match group's state
      mg.setState(STAT_MG_SCHEDULED);
      TabRow r = groupTab[mg.getId()];
      r.update(MG_STAGE_SEQ_NUM, QVariant());  // delete the sequence number
      emit matchGroupStatusChanged(mg.getId(), mg.getSeqNum(), STAT_MG_STAGED, STAT_MG_SCHEDULED);
    }
  }

//----------------------------------------------------------------------------

  /**
   * Retrieves a list of all currently staged match groups. The
   * earliest-to-be-played group is at index 0
   *
   * @return a list of match groups, sorted by stage sequence number
   */
  MatchGroupList MatchMngr::getStagedMatchGroupsOrderedBySequence() const
  {
    MatchGroupList result;

    QString where = MG_STAGE_SEQ_NUM + " > 0 ORDER BY " + MG_STAGE_SEQ_NUM + " ASC";
    auto it = groupTab.getRowsByWhereClause(where);
    while (!(it.isEnd()))
    {
      MatchGroup mg(db, *it);
      result.append(mg);
      ++it;
    }

    return result;
  }

//----------------------------------------------------------------------------

  /**
   * Determines the highest currently assigned match number
   *
   * @return the highest assigned match number of all matches in all groups; 0 if no number has been assigned yet
   */
  int MatchMngr::getMaxMatchNum() const
  {
    // Is there any staged match group at all?
    QString where = MA_NUM + " > 0";
    if (matchTab.getMatchCountForWhereClause(where) < 1)
    {
      return 0;  // no assigned match numbers so far
    }

    // determine the max match number
    QString sql = "SELECT max(" + MA_NUM + ") FROM " + TAB_MATCH;
    QVariant result = db->execScalarQuery(sql);
    if ( (!(result.isValid())) || (result.isNull()))
    {
      return 0;  // shouldn't happen, but anyway...
    }

    return result.toInt();
  }

//----------------------------------------------------------------------------

  unique_ptr<Match> MatchMngr::getMatchBySeqNum(int maSeqNum) const
  {
    try {
      TabRow r = matchTab.getSingleRowByColumnValue(GENERIC_SEQNUM_FIELD_NAME, maSeqNum);
      Match* ma_raw = new Match(db, r.getId());
      return unique_ptr<Match>(ma_raw);
    }
    catch (std::exception e)
    {
     return nullptr;  // null indicates error
    }
    return nullptr;
  }

//----------------------------------------------------------------------------

  /**
   * Determines the next callable match and the next free court. Can be used for
   * automatically calling the next match, e.g., after a previous match is finished
   *
   * @param matchNum will contain the database ID of the next callable match
   * @param courtNum will contain the database ID of the next free court
   * @param includeManualCourts should be set to true if the search for free courts shall include courts with manual match assignment
   *
   * @return the identified match and court number and an error code
   */
  ERR MatchMngr::getNextViableMatchCourtPair(int *matchId, int *courtId, bool includeManualCourts) const
  {
    // default return values: error
    *matchId = -1;
    *courtId = -1;

    // find the next available match with the lowest match number
    int reqState = static_cast<int>(STAT_MA_READY);
    QString where = GENERIC_STATE_FIELD_NAME + " = " + QString::number(reqState);
    where += " ORDER BY " + MA_NUM + " ASC";
    if (matchTab.getMatchCountForWhereClause(where) < 1)
    {
      return NO_MATCH_AVAIL;
    }
    TabRow matchRow = matchTab.getSingleRowByWhereClause(where);

    ERR err;
    auto nextCourt = Tournament::getCourtMngr()->autoSelectNextUnusedCourt(&err, includeManualCourts);
    if (err == OK)
    {
      *matchId = matchRow.getId();
      *courtId = nextCourt->getId();
      return OK;
    }

    return err;
  }

//----------------------------------------------------------------------------

  /**
   * Determines whether it is okay to start a specific match on a specific court
   *
   * @param ma the match to start
   * @param court the court to start it on
   *
   * @return error code
   */
  ERR MatchMngr::canAssignMatchToCourt(const Match &ma, const Court& court) const
  {
    // check the match's state
    if (ma.getState() != STAT_MA_READY)
    {
      return MATCH_NOT_RUNNABLE;
    }

    // check the court's availability
    OBJ_STATE stat = court.getState();
    if (stat == STAT_CO_AVAIL)
    {
      return OK;
    }
    if (stat == STAT_CO_DISABLED)
    {
      return COURT_DISABLED;
    }

    return COURT_BUSY;
  }

//----------------------------------------------------------------------------

  ERR MatchMngr::assignMatchToCourt(const Match &ma, const Court &court) const
  {
    ERR e = canAssignMatchToCourt(ma, court);
    if (e != OK) return e;

    // NORMALLY, we should first acquire the court and then assign the
    // match to this court. BUT acquiring triggers an update of the
    // associate court views and we want the update to show the match
    // details, too. So we first assign the match and formally acquire the
    // court afterwards. This way, the match is already linked to the court
    // when all views are updated.

    QVariantList qvl;

    // assign the court
    qvl << MA_COURT_REF << court.getId();

    // copy the actual players to the database
    // TODO: implement substitute players etc. So far, we only copy
    // the contents of the player pairs blindly
    PlayerPair pp = ma.getPlayerPair1();
    qvl << MA_ACTUAL_PLAYER1A_REF << pp.getPlayer1().getId();
    if (pp.hasPlayer2())
    {
      qvl << MA_ACTUAL_PLAYER1B_REF << pp.getPlayer2().getId();
    }
    pp = ma.getPlayerPair2();
    qvl << MA_ACTUAL_PLAYER2A_REF << pp.getPlayer1().getId();
    if (pp.hasPlayer2())
    {
      qvl << MA_ACTUAL_PLAYER2B_REF << pp.getPlayer2().getId();
    }

    // update the match state
    qvl << GENERIC_STATE_FIELD_NAME << static_cast<int>(STAT_MA_RUNNING);

    // execute all updates at once
    TabRow matchRow = matchTab[ma.getId()];
    matchRow.update(qvl);

    // tell the world that the match status has changed
    emit matchStatusChanged(ma.getId(), ma.getSeqNum(), STAT_MA_READY, STAT_MA_RUNNING);

    // now we finally acquire the court in the aftermath
    bool isOkay = Tournament::getCourtMngr()->acquireCourt(court);
    assert(isOkay);

    // update the category's state to "PLAYING", if necessary
    Tournament::getCatMngr()->updateCatStatusFromMatchStatus(ma.getCategory());

    // TODO: add call time to database

    return OK;
  }

//----------------------------------------------------------------------------

  unique_ptr<Court> MatchMngr::autoAssignMatchToNextAvailCourt(const Match &ma, ERR *err, bool includeManualCourts) const
  {
    ERR e;
    auto nextCourt = Tournament::getCourtMngr()->autoSelectNextUnusedCourt(&e, includeManualCourts);
    if (nextCourt != nullptr)
    {
      *err = assignMatchToCourt(ma, *nextCourt);
      return (*err == OK) ? move(nextCourt) : nullptr;
    }

    // return the error resulting from the court selection
    *err = e;
    return nullptr;
  }

//----------------------------------------------------------------------------

  ERR MatchMngr::setMatchScoreAndFinalizeMatch(const Match &ma, const MatchScore &score)
  {
    // check the match's state
    if (ma.getState() != STAT_MA_RUNNING)
    {
      return MATCH_NOT_RUNNING;
    }

    // check if the score is valid for the category settings
    Category cat = ma.getCategory();
    bool isDrawAllowed = cat.getParameter_bool(ALLOW_DRAW);
    int numWinGames = 2; // TODO: this needs to become a category parameter!
    if (!(score.isValidScore(numWinGames, isDrawAllowed)))
    {
      return INVALID_MATCH_RESULT_FOR_CATEGORY_SETTINGS;
    }

    // everything is fine, so write the result to the database
    // and update the match status
    int maId = ma.getId();
    int maSeqNum = ma.getSeqNum();
    QVariantList qvl;
    qvl << MA_RESULT << score.toString();
    qvl << GENERIC_STATE_FIELD_NAME << static_cast<int>(STAT_MA_FINISHED);
    TabRow matchRow = matchTab[maId];
    matchRow.update(qvl);

    emit matchResultUpdated(maId, maSeqNum);
    emit matchStatusChanged(maId, maSeqNum, STAT_MA_RUNNING, STAT_MA_FINISHED);

    // release the court
    ERR e;
    auto pCourt = ma.getCourt(&e);
    assert(e == OK);
    bool isOkay = Tournament::getCourtMngr()->releaseCourt(*pCourt);
    assert(isOkay);

    // update the category's state to "FINALIZED", if necessary
    Tournament::getCatMngr()->updateCatStatusFromMatchStatus(ma.getCategory());

    // TODO: add finish time to database

    return OK;
  }

//----------------------------------------------------------------------------

  unique_ptr<Match> MatchMngr::getMatchForCourt(const Court &court)
  {
    // search for matches in state RUNNING and assigned to the court
    QVariantList qvl;
    qvl << MA_COURT_REF << court.getId();
    qvl << GENERIC_STATE_FIELD_NAME << static_cast<int>(STAT_MA_RUNNING);

    if (matchTab.getMatchCountForColumnValue(qvl) != 1)
    {
      return nullptr;
    }

    TabRow r = matchTab.getSingleRowByColumnValue(qvl);

    return unique_ptr<Match>(new Match(db, r));
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


}
