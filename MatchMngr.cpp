/*
 *    This is QTournament, a badminton tournament management program.
 *    Copyright (C) 2014 - 2015  Volker Knollmann
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

#include <assert.h>

#include <QDateTime>

#include "DateAndTime.h"

#include "MatchMngr.h"
#include "Tournament.h"
#include "CatRoundStatus.h"

using namespace SqliteOverlay;

namespace QTournament {

  MatchMngr::MatchMngr(TournamentDB* _db)
    : TournamentDatabaseObjectManager(_db, TAB_MATCH), groupTab(db->getTab(TAB_MATCH_GROUP))
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
      int nOtherGroups = getMatchGroupsForCat(cat, round).size();
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
      WhereClause wc;
      wc.addIntCol(MG_GRP_NUM, "<=", 0);
      wc.addIntCol(MG_ROUND, round);
      wc.addIntCol(MG_CAT_REF, cat.getId());
      int nOtherGroups = groupTab->getMatchCountForWhereClause(wc);
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
    ColumnValueClause cvc;
    cvc.addIntCol(MG_CAT_REF, cat.getId());
    cvc.addIntCol(MG_ROUND, round);
    cvc.addIntCol(MG_GRP_NUM, grpNum);
    cvc.addIntCol(GENERIC_STATE_FIELD_NAME, static_cast<int>(STAT_MG_CONFIG));
    emit beginCreateMatchGroup();
    int newId = groupTab->insertRow(cvc);
    fixSeqNumberAfterInsert(groupTab);
    emit endCreateMatchGroup(groupTab->length() - 1); // the new sequence number is always the largest

    
    // create a match group object for the new group an return a pointer
    // to this new object
    MatchGroup* mg_raw = new MatchGroup(db, newId);
    *err = OK;
    return unique_ptr<MatchGroup>(mg_raw);
  }

  //----------------------------------------------------------------------------

  MatchGroupList MatchMngr::getMatchGroupsForCat(const Category& cat, int round) const
  {
    WhereClause wc;
    wc.addIntCol(MG_CAT_REF, cat.getId());
    if (round > 0)
    {
      wc.addIntCol(MG_ROUND, round);
    }

    return getObjectsByWhereClause<MatchGroup>(groupTab, wc);
  }
  
  //----------------------------------------------------------------------------

  MatchGroupList MatchMngr::getAllMatchGroups()
  {
    return getAllObjects<MatchGroup>(groupTab);
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
    
    WhereClause wc;
    wc.addIntCol(MG_CAT_REF, cat.getId());
    wc.addIntCol(MG_ROUND, round);
    wc.addIntCol(MG_GRP_NUM, grpNum);
    
    try
    {
      TabRow r = groupTab->getSingleRowByWhereClause(wc);
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
    
    if ((e == OK) && (mg != nullptr))
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
    ColumnValueClause cvc;
    cvc.addIntCol(MA_GRP_REF, grp.getId());
    cvc.addIntCol(GENERIC_STATE_FIELD_NAME, static_cast<int>(STAT_MA_INCOMPLETE));
    cvc.addIntCol(MA_PAIR1_SYMBOLIC_VAL, 0);   // default: no symbolic name
    cvc.addIntCol(MA_PAIR2_SYMBOLIC_VAL, 0);   // default: no symbolic name
    cvc.addIntCol(MA_WINNER_RANK, -1);         // default: no rank, no knock out
    cvc.addIntCol(MA_LOSER_RANK, -1);         // default: no rank, no knock out
    emit beginCreateMatch();
    int newId = tab->insertRow(cvc);
    fixSeqNumberAfterInsert();
    emit endCreateMatch(tab->length() - 1); // the new sequence number is always the highest

    // create a match group object for the new group and return a pointer
    // to this new object
    Match* ma = new Match(db, newId);
    *err = OK;
    return unique_ptr<Match>(ma);
  }

  //----------------------------------------------------------------------------

  ERR MatchMngr::setPlayerPairsForMatch(const Match &ma, const PlayerPair &pp1, const PlayerPair &pp2)
  {
    //
    // Initial note: we do not solve this function here by simply calling
    // setPlayerPairForMatch() twice. Because if we would do so, we could
    // leave the match in an invalid state if pp1 is valid (and thus assigned
    // to the match by the first call) and pp2 is not (and thus playerpair2 is
    // not set).
    //


    // make sure the player pair is valid (has a database entry)
    if (pp1.getPairId() < 1) return INVALID_PLAYER_PAIR;
    if (pp2.getPairId() < 1) return INVALID_PLAYER_PAIR;

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
    TabRow matchRow = tab->operator [](ma.getId());
    matchRow.update(MA_PAIR1_REF, pp1.getPairId());
    matchRow.update(MA_PAIR2_REF, pp2.getPairId());

    // potentially, the player pairs where all that was necessary
    // to actually promote the match to e.g., WAITING
    updateMatchStatus(ma);

    return OK;
  }

  //----------------------------------------------------------------------------

  ERR MatchMngr::setPlayerPairForMatch(const Match& ma, const PlayerPair& pp, int ppPos) const
  {
    // make sure the player pair is valid (has a database entry)
    if (pp.getPairId() < 1) return INVALID_PLAYER_PAIR;

    // check if an assignment of the player pairs is okay
    ERR e = canAssignPlayerPairToMatch(ma, pp);
    if (e != OK) return e;

    // assign the player pair
    TabRow matchRow = tab->operator [](ma.getId());
    if (ppPos == 1) matchRow.update(MA_PAIR1_REF, pp.getPairId());
    if (ppPos == 2) matchRow.update(MA_PAIR2_REF, pp.getPairId());

    return OK;
  }

  //----------------------------------------------------------------------------

  ERR MatchMngr::setSymbolicPlayerForMatch(const Match& fromMatch, const Match& toMatch, bool asWinner, int dstPlayerPosInMatch) const
  {
    // Only allow changing / setting players if we not yet fully configured
    if (toMatch.getState() != STAT_MA_INCOMPLETE) return MATCH_NOT_CONFIGURALE_ANYMORE;

    // fromMatch and toMatch must be in the same category
    int fromMatchCatId = fromMatch.getCategory().getId();
    int toMatchCatId = toMatch.getCategory().getId();
    if (fromMatchCatId != toMatchCatId)
    {
      return INVALID_MATCH_LINK;
    }

    // toMatch must be in a later round than fromMatch
    MatchGroup fromGroup = fromMatch.getMatchGroup();
    MatchGroup toGroup = toMatch.getMatchGroup();
    if (toGroup.getRound() <= fromGroup.getRound())
    {
      return INVALID_MATCH_LINK;
    }

    // okay, the link is valid
    TabRow toRow = tab->operator [](toMatch.getId());
    int dstId = asWinner ? fromMatch.getId() : -(fromMatch.getId());
    if (dstPlayerPosInMatch == 1)
    {
      toRow.update(MA_PAIR1_SYMBOLIC_VAL, dstId);
      toRow.updateToNull(MA_PAIR1_REF);
    }
    if (dstPlayerPosInMatch == 2)
    {
      toRow.update(MA_PAIR2_SYMBOLIC_VAL, dstId);
      toRow.updateToNull(MA_PAIR2_REF);
    }

    return OK;
  }

  //----------------------------------------------------------------------------

  ERR MatchMngr::setPlayerToUnused(const Match& ma, int unusedPlayerPos, int winnerRank) const
  {
    // Only allow changing / setting player pairs if we not yet fully configured
    if (ma.getState() != STAT_MA_INCOMPLETE) return MATCH_NOT_CONFIGURALE_ANYMORE;

    TabRow matchRow = tab->operator [](ma.getId());
    if (unusedPlayerPos == 1)
    {
      matchRow.updateToNull(MA_PAIR1_REF);
      matchRow.update(MA_PAIR1_SYMBOLIC_VAL, SYMBOLIC_ID_FOR_UNUSED_PLAYER_PAIR_IN_MATCH);
    }
    if (unusedPlayerPos == 2)
    {
      matchRow.updateToNull(MA_PAIR2_REF);
      matchRow.update(MA_PAIR2_SYMBOLIC_VAL, SYMBOLIC_ID_FOR_UNUSED_PLAYER_PAIR_IN_MATCH);
    }
    matchRow.update(MA_WINNER_RANK, winnerRank);

    return OK;
  }

  //----------------------------------------------------------------------------

  ERR MatchMngr::setRankForWinnerOrLoser(const Match& ma, bool isWinner, int rank) const
  {
    // Only allow changing / setting match data if we not yet fully configured
    if (ma.getState() != STAT_MA_INCOMPLETE) return MATCH_NOT_CONFIGURALE_ANYMORE;

    // TODO: check if rank is really valid

    if (isWinner)
    {
      ma.row.update(MA_WINNER_RANK, rank);
    } else {
      ma.row.update(MA_LOSER_RANK, rank);
    }

    return OK;
  }

  //----------------------------------------------------------------------------

  ERR MatchMngr::canAssignPlayerPairToMatch(const Match &ma, const PlayerPair &pp) const
  {
    // Only allow changing / setting player pairs if we not yet fully configured
    OBJ_STATE stat = ma.getState();
    if ((stat != STAT_MA_INCOMPLETE) && (stat != STAT_MA_FUZZY)) return MATCH_NOT_CONFIGURALE_ANYMORE;

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
      TabRow r = groupTab->getSingleRowByColumnValue(GENERIC_SEQNUM_FIELD_NAME, mgSeqNum);
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
    TabRow r = groupTab->operator [](grpId);
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
    if (groupTab->getMatchCountForColumnValue(GENERIC_STATE_FIELD_NAME, statId) < 1)
    {
      return 0;  // no staged match groups so far
    }

    // determine the max sequence number
    QString sql = "SELECT max(%1) FROM %2";
    sql = sql.arg(MG_STAGE_SEQ_NUM).arg(TAB_MATCH_GROUP);
    int result;
    int dbErr;
    bool isOk = db->execScalarQueryInt(sql.toUtf8().constData(), &result, &dbErr);
    if (!isOk)
    {
      return 0;  // shouldn't happen, but anyway...
    }

    return result;
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
    WhereClause wc;
    wc.addIntCol(MG_ROUND, round+1);
    wc.addIntCol(GENERIC_STATE_FIELD_NAME, static_cast<int>(STAT_MG_STAGED));
    wc.addIntCol(MG_CAT_REF, catId);
    if (groupTab->getMatchCountForWhereClause(wc) == 0)
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
    for (MatchGroup mg : getObjectsByWhereClause<MatchGroup>(groupTab, wc))
    {
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
    TabRow r = groupTab->operator [](grpId);
    r.updateToNull(MG_STAGE_SEQ_NUM);

    emit matchGroupStatusChanged(grp.getId(), grp.getSeqNum(), STAT_MG_STAGED, STAT_MG_IDLE);

    // update all subsequent sequence numbers
    WhereClause wc;
    wc.addIntCol(MG_STAGE_SEQ_NUM, ">", oldStageSeqNumber);
    for (MatchGroup mg : getObjectsByWhereClause<MatchGroup>(groupTab, wc))
    {
      int old = mg.getStageSequenceNumber();
      mg.row.update(MG_STAGE_SEQ_NUM, old - 1);
      emit matchGroupStatusChanged(mg.getId(), mg.getSeqNum(), STAT_MG_STAGED, STAT_MG_STAGED);
    }

    // demote other rounds from IDLE to FROZEN
    // check for a match group with "round + 1" in state IDLE
    int round = grp.getRound();
    int playersGroup = grp.getGroupNumber();
    int catId = grp.getCategory().getId();

    wc.clear();
    wc.addIntCol(MG_ROUND, round+1);
    wc.addIntCol(GENERIC_STATE_FIELD_NAME, static_cast<int>(STAT_MG_IDLE));
    wc.addIntCol(MG_CAT_REF, catId);
    for (MatchGroup mg : getObjectsByWhereClause<MatchGroup>(groupTab, wc))
    {
      // if our demoted match group and the IDLE match group are round-robin-groups,
      // the IDLE match group has only to be demoted if it matches the "grp's" players group
      //
      // or in inverse logic: if the player groups to not match, we don't have to
      // touch this group
      int otherPlayersGroup = mg.getGroupNumber();
      if ((playersGroup > 0) && (otherPlayersGroup > 0) && (playersGroup != otherPlayersGroup))
      {
        continue;
      }

      // in all other cases, the "IDLE" group has to be demoted to FROZEN
      mg.setState(STAT_MG_FROZEN);
      emit matchGroupStatusChanged(mg.getId(), mg.getSeqNum(), STAT_MG_IDLE, STAT_MG_FROZEN);
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

    // from INCOMPLETE to FUZZY
    if (curState == STAT_MA_INCOMPLETE)
    {
      bool isFuzzy1 = (ma.row.getInt(MA_PAIR1_SYMBOLIC_VAL) != 0);
      bool isFuzzy2 = (ma.row.getInt(MA_PAIR2_SYMBOLIC_VAL) != 0);
      bool hasMatchNumber = ma.getMatchNumber() > 0;

      // we shall never have a symbolic and a real player assignment at the same time
      assert((isFuzzy1 && ma.hasPlayerPair1()) == false);
      assert((isFuzzy2 && ma.hasPlayerPair2()) == false);

      if (isFuzzy1 || isFuzzy2 || hasMatchNumber)
      {
        ma.setState(STAT_MA_FUZZY);
        curState = STAT_MA_FUZZY;
        emit matchStatusChanged(ma.getId(), ma.getSeqNum(), STAT_MA_INCOMPLETE, STAT_MA_FUZZY);
      }
    }

    // transition from FUZZY to WAITING:
    // if we've resolved all symbolic references of a match, it may be promoted from
    // FUZZY at least to WAITING, maybe even to READY or BUSY
    if (curState == STAT_MA_FUZZY)
    {
      bool isFixed1 = ((ma.row.getInt(MA_PAIR1_SYMBOLIC_VAL) == 0) && (ma.row.getInt(MA_PAIR1_REF) > 0));
      bool isFixed2 = ((ma.row.getInt(MA_PAIR2_SYMBOLIC_VAL) == 0) && (ma.row.getInt(MA_PAIR2_REF) > 0));
      bool hasMatchNumber = ma.getMatchNumber() > 0;

      if (isFixed1 && isFixed2 && hasMatchNumber)
      {
        ma.setState(STAT_MA_WAITING);
        curState = STAT_MA_WAITING;
        emit matchStatusChanged(ma.getId(), ma.getSeqNum(), STAT_MA_FUZZY, STAT_MA_WAITING);
      }
    }

    // from WAITING to READY or BUSY
    bool hasPredecessor = hasUnfinishedMandatoryPredecessor(ma);
    auto tnmt = Tournament::getActiveTournament();
    bool playersAvail = ((tnmt->getPlayerMngr()->canAcquirePlayerPairsForMatch(ma)) == OK);
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
  bool MatchMngr::hasUnfinishedMandatoryPredecessor(const Match &ma) const
  {
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
    int requiredPrevRoundsPlayersGroup = cat.getGroupNumForPredecessorRound(mg.getGroupNumber());
    for (auto prevMg : getMatchGroupsForCat(cat, round-1))
    {
      int actualPrevRoundsPlayersGroup = prevMg.getGroupNumber();
      if ((requiredPrevRoundsPlayersGroup == ANY_PLAYERS_GROUP_NUMBER) && (actualPrevRoundsPlayersGroup < 0))
      {
        continue;  // we're looking for any round robins, but this a KO group or an iterative group
      }

      if ((requiredPrevRoundsPlayersGroup > 0) && (actualPrevRoundsPlayersGroup != requiredPrevRoundsPlayersGroup))
      {
        // we're looking for a specific players group in round robins,
        // but this a KO group or an iterative group or a wrong
        // players group
        continue;
      }

      if (((requiredPrevRoundsPlayersGroup == GROUP_NUM__SEMIFINAL) ||
           (requiredPrevRoundsPlayersGroup == GROUP_NUM__QUARTERFINAL) ||
           (requiredPrevRoundsPlayersGroup == GROUP_NUM__L16)) && (actualPrevRoundsPlayersGroup != requiredPrevRoundsPlayersGroup))
      {
        continue;  // wrong KO round
      }

      //
      // if we made it to this point, the match group is a mandatory predecessor
      //

      // in round robins, rounds are independent from each other. for this reason,
      // we may, for instance, start matches in round 3 before round 2 is finished.
      // the same assumption holds for elimination rounds
      MATCH_SYSTEM mSys = cat.getMatchSystem();
      if ((mSys == SINGLE_ELIM) || (mSys == RANKING) || (mSys == ROUND_ROBIN) || ((mSys == GROUPS_WITH_KO) && (mg.getGroupNumber() > 0)))
      {
        return false;
      }

      // Okay, the previous match group really has to be finished first.
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
        TabRow r = tab->operator [](matchId);
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
      TabRow r = groupTab->operator [](mg.getId());
      r.updateToNull(MG_STAGE_SEQ_NUM);  // delete the sequence number
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

    WhereClause wc;
    wc.addIntCol(MG_STAGE_SEQ_NUM, ">", 0);
    wc.setOrderColumn_Asc(MG_STAGE_SEQ_NUM);
    return getObjectsByWhereClause<MatchGroup>(groupTab, wc);
  }

  //----------------------------------------------------------------------------

  /**
   * Determines the highest currently assigned match number
   *
   * @return the highest assigned match number of all matches in all groups; 0 if no number has been assigned yet
   */
  int MatchMngr::getMaxMatchNum() const
  {
    // Is there any scheduled match at all?
    WhereClause wc;
    wc.addIntCol(MA_NUM, ">", 0);
    if (tab->getMatchCountForWhereClause(wc) < 1)
    {
      return 0;  // no assigned match numbers so far
    }

    // determine the max match number
    QString sql = "SELECT max(%1) FROM %2";
    sql = sql.arg(MA_NUM).arg(TAB_MATCH);
    int result;
    int dbErr;
    bool isOk = db->execScalarQueryInt(sql.toUtf8().constData(), &result, &dbErr);
    if (!isOk)
    {
      return 0;  // shouldn't happen, but anyway...
    }

    return result;
  }

  //----------------------------------------------------------------------------

  unique_ptr<Match> MatchMngr::getMatchBySeqNum(int maSeqNum) const
  {
    try {
      TabRow r = tab->getSingleRowByColumnValue(GENERIC_SEQNUM_FIELD_NAME, maSeqNum);
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

  unique_ptr<Match> MatchMngr::getMatchByMatchNum(int maNum) const
  {
    try {
      TabRow r = tab->getSingleRowByColumnValue(MA_NUM, maNum);
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
    WhereClause wc;
    wc.addIntCol(GENERIC_STATE_FIELD_NAME, reqState);
    wc.setOrderColumn_Asc(MA_NUM);
    if (tab->getMatchCountForWhereClause(wc) < 1)
    {
      return NO_MATCH_AVAIL;
    }
    TabRow matchRow = tab->getSingleRowByWhereClause(wc);

    ERR err;
    auto tnmt = Tournament::getActiveTournament();
    auto nextCourt = tnmt->getCourtMngr()->autoSelectNextUnusedCourt(&err, includeManualCourts);
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

    //
    // TODO: we shouldn't be able to start matches if the
    // associated category is a state other than PLAYING
    // or IDLE. In particular, we shouldn't call games
    // if we're in WAITING_FOR_SEEDING.
    //
    // Maybe this needs to be solved here or maybe we should
    // update the match status when we leave IDLE or PLAYING
    //

    // check the player's availability
    auto tnmt = Tournament::getActiveTournament();
    ERR e = tnmt->getPlayerMngr()->canAcquirePlayerPairsForMatch(ma);
    if (e != OK) return e;  // PLAYER_NOT_IDLE

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


    // EXTREMELY IMPORTANT:
    // Always maintain this sequence when updating object states:
    //   1) the match that's being called
    //   2) player status
    //   -) step (2) automatically triggers an update of applicable other matches to BUSY
    //   3) court
    //   4) other matches from WAITING to READY or BUSY, based on match sequence logic
    //   5) category states
    //
    // Since match groups don't distinguish between SCHEDULED and FINISHED, a match group
    // update is not necessary

    ColumnValueClause cvc;

    // assign the court
    cvc.addIntCol(MA_COURT_REF, court.getId());

    // copy the actual players to the database
    // TODO: implement substitute players etc. So far, we only copy
    // the contents of the player pairs blindly
    PlayerPair pp = ma.getPlayerPair1();
    cvc.addIntCol(MA_ACTUAL_PLAYER1A_REF, pp.getPlayer1().getId());
    if (pp.hasPlayer2())
    {
      cvc.addIntCol(MA_ACTUAL_PLAYER1B_REF, pp.getPlayer2().getId());
    }
    pp = ma.getPlayerPair2();
    cvc.addIntCol(MA_ACTUAL_PLAYER2A_REF, pp.getPlayer1().getId());
    if (pp.hasPlayer2())
    {
      cvc.addIntCol(MA_ACTUAL_PLAYER2B_REF, pp.getPlayer2().getId());
    }

    // update the match state
    cvc.addIntCol(GENERIC_STATE_FIELD_NAME, static_cast<int>(STAT_MA_RUNNING));

    // execute all updates at once
    TabRow matchRow = tab->operator [](ma.getId());
    matchRow.update(cvc);

    // tell the world that the match status has changed
    emit matchStatusChanged(ma.getId(), ma.getSeqNum(), STAT_MA_READY, STAT_MA_RUNNING);

    // update the player's status
    auto tnmt = Tournament::getActiveTournament();
    e = tnmt->getPlayerMngr()->acquirePlayerPairsForMatch(ma);
    assert(e == OK);  // must be, because the condition has been check by canAssignMatchToCourt()

    // now we finally acquire the court in the aftermath
    bool isOkay = tnmt->getCourtMngr()->acquireCourt(court);
    assert(isOkay);

    // update the category's state to "PLAYING", if necessary
    tnmt->getCatMngr()->updateCatStatusFromMatchStatus(ma.getCategory());

    // store the call time in the database
    matchRow.update(MA_START_TIME, UTCTimestamp());

    return OK;
  }

  //----------------------------------------------------------------------------

  unique_ptr<Court> MatchMngr::autoAssignMatchToNextAvailCourt(const Match &ma, ERR *err, bool includeManualCourts) const
  {
    ERR e;
    auto tnmt = Tournament::getActiveTournament();
    auto nextCourt = tnmt->getCourtMngr()->autoSelectNextUnusedCourt(&e, includeManualCourts);
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

  ERR MatchMngr::setMatchScoreAndFinalizeMatch(const Match &ma, const MatchScore &score, bool isWalkover) const
  {
    // check the match's state
    OBJ_STATE oldState = ma.getState();
    if ((!isWalkover) && (oldState != STAT_MA_RUNNING))
    {
      return MATCH_NOT_RUNNING;
    }
    if (isWalkover && (!(ma.isWalkoverPossible())))
    {
      return WRONG_STATE;
    }

    // check if the score is valid for the category settings
    Category cat = ma.getCategory();
    bool isDrawAllowed = cat.isDrawAllowedInRound(ma.getMatchGroup().getRound());
    int numWinGames = 2; // TODO: this needs to become a category parameter!
    if (!(score.isValidScore(numWinGames, isDrawAllowed)))
    {
      return INVALID_MATCH_RESULT_FOR_CATEGORY_SETTINGS;
    }

    // conserve the round status BEFORE we finalize the match.
    // we need this information later to detect whether we've finished
    // a round or not
    int lastFinishedRoundBeforeMatch = ma.getCategory().getRoundStatus().getFinishedRoundsCount();


    // EXTREMELY IMPORTANT:
    // Always maintain this sequence when updating object states:
    //   1) the match that's being finished
    //   2) player status
    //   -) step (2) automatically triggers an update of applicable other matches to BUSY
    //   3) court
    //   4) match group status to FINISH, based on match completion
    //   5) other matches from WAITING to READY or BUSY, based on match sequence logic
    //   6) category states


    // everything is fine, so write the result to the database
    // and update the match status
    int maId = ma.getId();
    int maSeqNum = ma.getSeqNum();
    ColumnValueClause cvc;
    cvc.addStringCol(MA_RESULT, score.toString().toUtf8().constData());
    cvc.addIntCol(GENERIC_STATE_FIELD_NAME, static_cast<int>(STAT_MA_FINISHED));
    TabRow matchRow = tab->operator [](maId);
    matchRow.update(cvc);

    emit matchResultUpdated(maId, maSeqNum);
    emit matchStatusChanged(maId, maSeqNum, oldState, STAT_MA_FINISHED);

    // if this was a regular, running match we need to release the court
    // and the players
    auto tnmt = Tournament::getActiveTournament();
    if (oldState == STAT_MA_RUNNING)
    {
      // release the players
      tnmt->getPlayerMngr()->releasePlayerPairsAfterMatch(ma);

      // release the court
      ERR e;
      auto pCourt = ma.getCourt(&e);
      assert(e == OK);
      bool isOkay = tnmt->getCourtMngr()->releaseCourt(*pCourt);
      assert(isOkay);
    }

    // update the match group
    updateAllMatchGroupStates(ma.getCategory());

    // update other matches in this category from WAITING to READY or BUSY, if applicable
    for (MatchGroup mg : getMatchGroupsForCat(ma.getCategory()))
    {
      for (Match otherMatch : mg.getMatches())
      {
        if (otherMatch.getState() != STAT_MA_WAITING) continue;
        updateMatchStatus(otherMatch);
      }
    }

    // in case some other match refers to this match with a symbolic name
    // (e.g., winner of match XYZ), resolve those symbolic names into
    // real player pairs
    resolveSymbolicNamesAfterFinishedMatch(ma);

    // update the category's state to "FINALIZED", if necessary
    tnmt->getCatMngr()->updateCatStatusFromMatchStatus(ma.getCategory());

    // store the finish time in the database
    if (oldState == STAT_MA_RUNNING)   // match was called normally, so we have a start time
    {
      matchRow.update(MA_FINISH_TIME, UTCTimestamp());
    }

    // get the round status AFTER the match and check whether
    // we'e just finished a round
    int lastFinishedRoundAfterMatch = ma.getCategory().getRoundStatus().getFinishedRoundsCount();
    if (lastFinishedRoundBeforeMatch != lastFinishedRoundAfterMatch)
    {
      // call the hook for finished rounds (e.g., for updating the ranking information
      // or for generating new matches)
      auto specialCat = ma.getCategory().convertToSpecializedObject();
      specialCat->onRoundCompleted(lastFinishedRoundAfterMatch);
      emit roundCompleted(ma.getCategory().getId(), lastFinishedRoundAfterMatch);
    }


    return OK;
  }

  //----------------------------------------------------------------------------

  ERR MatchMngr::walkover(const Match& ma, int playerNum) const
  {
    // for a walkover, the match must be in READY, WAITING, RUNNING or BUSY
    if (!(ma.isWalkoverPossible()))
    {
      return WRONG_STATE;
    }

    // the playerNum must be either 1 or 2
    if ((playerNum != 1) && (playerNum != 2))
    {
      return INVALID_PLAYER_PAIR;
    }

    // determine the game results
    int sc1 = (playerNum == 1) ? 21 : 0;
    int sc2 = (playerNum == 1) ? 0 : 21;

    // fake a match result
    int numWinGames = 2;    // TODO: this should become a category parameter
    GameScoreList gsl;
    for (int i=0; i < numWinGames; ++i)
    {
      gsl.append(*(GameScore::fromScore(sc1, sc2)));
    }
    auto ms = MatchScore::fromGameScoreListWithoutValidation(gsl);
    assert(ms != nullptr);

    return setMatchScoreAndFinalizeMatch(ma, *ms, true);
  }

  //----------------------------------------------------------------------------

  ERR MatchMngr::undoMatchCall(const Match& ma) const
  {
    if (ma.getState() != STAT_MA_RUNNING)
    {
      return MATCH_NOT_RUNNING;
    }

    // release the players first, because we need the entries
    // in MA_ACTUAL_PLAYER1A_REF etc.
    auto tnmt = Tournament::getActiveTournament();
    tnmt->getPlayerMngr()->releasePlayerPairsAfterMatch(ma);

    // store the court the match is running on
    ERR e;
    auto pCourt = ma.getCourt(&e);
    assert(e == OK);

    // reset the references to the court and the actual players
    ColumnValueClause cvc;
    cvc.addNullCol(MA_ACTUAL_PLAYER1A_REF);
    cvc.addNullCol(MA_ACTUAL_PLAYER1B_REF);
    cvc.addNullCol(MA_ACTUAL_PLAYER2A_REF);
    cvc.addNullCol(MA_ACTUAL_PLAYER2B_REF);
    cvc.addNullCol(MA_COURT_REF);

    // set the state back to READY
    cvc.addIntCol(GENERIC_STATE_FIELD_NAME, static_cast<int>(STAT_MA_READY));

    // apply all changes at once
    int maId = ma.getId();
    TabRow matchRow = tab->operator [](maId);
    matchRow.update(cvc);
    emit matchStatusChanged(maId, ma.getSeqNum(), STAT_MA_RUNNING, STAT_MA_READY);

    // release the court
    bool isOkay = tnmt->getCourtMngr()->releaseCourt(*pCourt);
    assert(isOkay);

    // update the match group
    updateAllMatchGroupStates(ma.getCategory());

    // update the category's state
    tnmt->getCatMngr()->updateCatStatusFromMatchStatus(ma.getCategory());

    // erase start time from database
    matchRow.updateToNull(MA_START_TIME);
    matchRow.updateToNull(MA_ADDITIONAL_CALL_TIMES);

    return OK;
  }

  //----------------------------------------------------------------------------

  unique_ptr<Match> MatchMngr::getMatchForCourt(const Court &court)
  {
    // search for matches in state RUNNING and assigned to the court
    WhereClause wc;
    wc.addIntCol(MA_COURT_REF, court.getId());
    wc.addIntCol(GENERIC_STATE_FIELD_NAME, static_cast<int>(STAT_MA_RUNNING));

    if (tab->getMatchCountForWhereClause(wc) != 1)
    {
      return nullptr;
    }

    TabRow r = tab->getSingleRowByWhereClause(wc);

    return unique_ptr<Match>(new Match(db, r));
  }

  //----------------------------------------------------------------------------

  unique_ptr<Match> MatchMngr::getMatch(int id) const
  {
    try
    {
      Match* ma = new Match(db, id);
      return unique_ptr<Match>(ma);
    }
    catch (std::exception e)
    {

    }
    return nullptr;
  }

  //----------------------------------------------------------------------------

  tuple<int, int, int> MatchMngr::getMatchStats() const
  {
    // get the total number of matches
    int nTotal = tab->length();

    // get the number of currently running matches
    int nRunning = tab->getMatchCountForColumnValue(GENERIC_STATE_FIELD_NAME, static_cast<int>(STAT_MA_RUNNING));

    // get the number of finished matches
    int nFinished = tab->getMatchCountForColumnValue(GENERIC_STATE_FIELD_NAME, static_cast<int>(STAT_MA_FINISHED));

    return make_tuple(nTotal, nFinished, nRunning);
  }

  //----------------------------------------------------------------------------

  void MatchMngr::onPlayerStatusChanged(int playerId, int playerSeqNum, OBJ_STATE fromState, OBJ_STATE toState)
  {
    // IMPORTANT:
    // This is the only place, where a transition from/to BUSY is managed
    //
    // IMPORTANT:
    // We only transition from READY to BUSY, never from any other state.
    // Otherwise we wouldn't to which state we should return to, after the
    // BUSY-condition is no longer applicable. Read: the only transition
    // back from BUSY is to READY!!


    // set matches that are READY to BUSY, if the necessary players become unavailable
    if (toState == STAT_PL_PLAYING)
    {
      for (Match ma : getObjectsByColumnValue<Match>(GENERIC_STATE_FIELD_NAME, static_cast<int>(STAT_MA_READY)))
      {
        PlayerList pl = ma.determineActualPlayers();
        for (Player p : pl)
        {
          if (p.getId() == playerId)
          {
            ma.row.update(GENERIC_STATE_FIELD_NAME, static_cast<int>(STAT_MA_BUSY));
            emit matchStatusChanged(ma.getId(), ma.getSeqNum(), STAT_MA_READY, STAT_MA_BUSY);
            break;  // no need to check other players for this match
          }
        }
      }
    }

    // switch matches from BUSY to READY, if all players are available again
    auto tnmt = Tournament::getActiveTournament();
    if (toState == STAT_PL_IDLE)
    {
      PlayerMngr* pm = tnmt->getPlayerMngr();
      for (Match ma : getObjectsByColumnValue<Match>(GENERIC_STATE_FIELD_NAME, static_cast<int>(STAT_MA_BUSY)))
      {
        if (pm->canAcquirePlayerPairsForMatch(ma) == OK)
        {
          ma.row.update(GENERIC_STATE_FIELD_NAME, static_cast<int>(STAT_MA_READY));
          emit matchStatusChanged(ma.getId(), ma.getSeqNum(), STAT_MA_BUSY, STAT_MA_READY);
        }
      }
    }
  }

  //----------------------------------------------------------------------------

  MatchList MatchMngr::getCurrentlyRunningMatches() const
  {
    return getObjectsByColumnValue<Match>(GENERIC_STATE_FIELD_NAME, static_cast<int>(STAT_MA_RUNNING));
  }

  //----------------------------------------------------------------------------

  MatchList MatchMngr::getFinishedMatches() const
  {
    return getObjectsByColumnValue<Match>(GENERIC_STATE_FIELD_NAME, static_cast<int>(STAT_MA_FINISHED));
  }

  //----------------------------------------------------------------------------

  MatchList MatchMngr::getMatchesForMatchGroup(const MatchGroup &grp) const
  {
    return getObjectsByColumnValue<Match>(MA_GRP_REF, grp.getId());
  }

  //----------------------------------------------------------------------------

  bool MatchMngr::hasMatchesInCategory(const Category &cat, int round) const
  {
    MatchGroupList mgl = getMatchGroupsForCat(cat, round);
    for (MatchGroup mg : mgl)
    {
      if (mg.getMatchCount() > 0) return true;
    }

    return false;
  }

  //----------------------------------------------------------------------------

  int MatchMngr::getHighestUsedRoundNumberInCategory(const Category& cat) const
  {
    // do we have match groups in this category at all?
    int grpCount = groupTab->getMatchCountForColumnValue(MG_CAT_REF, cat.getId());
    if (grpCount == 0) return 0;

    // query the highest used round number
    QString sql = "SELECT max(%1) FROM %2 WHERE %3 = %4";
    sql = sql.arg(MG_ROUND).arg(TAB_MATCH_GROUP);
    sql = sql.arg(MG_CAT_REF).arg(cat.getId());

    int result;
    int dbErr;
    bool isOk = db->execScalarQueryInt(sql.toUtf8().constData(), &result, &dbErr);
    if (!isOk)
    {
      return 0;  // shouldn't happen, but anyway...
    }

    return result;
  }

  //----------------------------------------------------------------------------

  unique_ptr<Match> MatchMngr::getMatchForPlayerPairAndRound(const PlayerPair &pp, int round) const
  {
    auto cat = pp.getCategory(db);
    if (cat == nullptr) return nullptr;

    for (MatchGroup mg : getMatchGroupsForCat(*cat, round))
    {
      QString where = "(%1 = %2 OR %3 = %2) AND %3 = %4";
      where = where.arg(MA_PAIR1_REF).arg(pp.getPairId()).arg(MA_PAIR2_REF);
      where = where.arg(MA_GRP_REF).arg(mg.getId());
      auto result = getSingleObjectByWhereClause<Match>(where.toUtf8().constData());
      if (result != nullptr) return result;
    }
    return nullptr;
  }

  //----------------------------------------------------------------------------

  void MatchMngr::resolveSymbolicNamesAfterFinishedMatch(const Match &ma) const
  {
    if (ma.getState() != STAT_MA_FINISHED)
    {
      return;
    }

    int matchId = ma.getId();
    auto winnerPair = ma.getWinner();
    auto loserPair = ma.getLoser();

    MatchList ml;
    if (winnerPair != nullptr)
    {
      // find all matches that use the winner of this match as player 1
      // and resolve their symbolic references
      ml = getObjectsByColumnValue<Match>(MA_PAIR1_SYMBOLIC_VAL, matchId);
      for (Match m : ml)
      {
        // we may only modify matches in state FUZZY or INCOMPLETE
        OBJ_STATE stat = m.getState();
        if ((stat != STAT_MA_FUZZY) && (stat != STAT_MA_INCOMPLETE)) continue;

        m.row.update(MA_PAIR1_REF, winnerPair->getPairId());  // set the reference to the winner
        m.row.update(MA_PAIR1_SYMBOLIC_VAL, 0);   // delete symbolic reference

        // emit a faked state change to trigger a display update of the
        // match in the match tab view
        emit matchStatusChanged(m.getId(), m.getSeqNum(), stat, stat);
      }
      // find all matches that use the winner of this match as player 2
      // and resolve their symbolic references
      ml = getObjectsByColumnValue<Match>(MA_PAIR2_SYMBOLIC_VAL, matchId);
      for (Match m : ml)
      {
        // we may only modify matches in state FUZZY or INCOMPLETE
        OBJ_STATE stat = m.getState();
        if ((stat != STAT_MA_FUZZY) && (stat != STAT_MA_INCOMPLETE)) continue;

        m.row.update(MA_PAIR2_REF, winnerPair->getPairId());  // set the reference to the winner
        m.row.update(MA_PAIR2_SYMBOLIC_VAL, 0);   // delete symbolic reference

        // emit a faked state change to trigger a display update of the
        // match in the match tab view
        emit matchStatusChanged(m.getId(), m.getSeqNum(), stat, stat);
      }
    }

    if (loserPair != nullptr)
    {
      // find all matches that use the loser of this match as player 1
      // and resolve their symbolic references
      ml = getObjectsByColumnValue<Match>(MA_PAIR1_SYMBOLIC_VAL, -matchId);
      for (Match m : ml)
      {
        // we may only modify matches in state FUZZY or INCOMPLETE
        OBJ_STATE stat = m.getState();
        if ((stat != STAT_MA_FUZZY) && (stat != STAT_MA_INCOMPLETE)) continue;

        m.row.update(MA_PAIR1_REF, loserPair->getPairId());  // set the reference to the winner
        m.row.update(MA_PAIR1_SYMBOLIC_VAL, 0);   // delete symbolic reference

        // emit a faked state change to trigger a display update of the
        // match in the match tab view
        emit matchStatusChanged(m.getId(), m.getSeqNum(), stat, stat);
      }
      // find all matches that use the loser of this match as player 2
      // and resolve their symbolic references
      ml = getObjectsByColumnValue<Match>(MA_PAIR2_SYMBOLIC_VAL, -matchId);
      for (Match m : ml)
      {
        // we may only modify matches in state FUZZY or INCOMPLETE
        OBJ_STATE stat = m.getState();
        if ((stat != STAT_MA_FUZZY) && (stat != STAT_MA_INCOMPLETE)) continue;

        m.row.update(MA_PAIR2_REF, loserPair->getPairId());  // set the reference to the winner
        m.row.update(MA_PAIR2_SYMBOLIC_VAL, 0);   // delete symbolic reference

        // emit a faked state change to trigger a display update of the
        // match in the match tab view
        emit matchStatusChanged(m.getId(), m.getSeqNum(), stat, stat);
      }
    }

    // if we resolved all symbolic references of a match, it may be promoted from
    // FUZZY at least to WAITING, maybe even to READY or BUSY
    QString where = QString("%1 = %2 AND %3 = 0 AND %4 = 0 AND %5 > 0 AND %6 > 0");
    where = where.arg(GENERIC_STATE_FIELD_NAME).arg(static_cast<int>(STAT_MA_FUZZY));
    where = where.arg(MA_PAIR1_SYMBOLIC_VAL).arg(MA_PAIR2_SYMBOLIC_VAL);
    where = where.arg(MA_PAIR1_REF).arg(MA_PAIR2_REF);
    for (Match m : getObjectsByWhereClause<Match>(where.toUtf8().constData()))
    {
      updateMatchStatus(m);
    }

  }

  //----------------------------------------------------------------------------


  //----------------------------------------------------------------------------


  //----------------------------------------------------------------------------


  //----------------------------------------------------------------------------


  //----------------------------------------------------------------------------


  //----------------------------------------------------------------------------


  //----------------------------------------------------------------------------


  //----------------------------------------------------------------------------


}
