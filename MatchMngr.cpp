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

#include <assert.h>

#include <QDateTime>

#include <Sloppy/DateTime/DateAndTime.h>
#include <SqliteOverlay/Transaction.h>

#include "MatchMngr.h"
#include "CatRoundStatus.h"
#include "CentralSignalEmitter.h"
#include "PlayerMngr.h"
#include "CourtMngr.h"
#include "CatMngr.h"
#include <SqliteOverlay/KeyValueTab.h>
#include "HelperFunc.h"

using namespace SqliteOverlay;

namespace QTournament {

  MatchMngr::MatchMngr(const TournamentDB& _db)
    : TournamentDatabaseObjectManager(_db, TAB_MATCH), groupTab{db, TAB_MATCH_GROUP, false}
  {
  }

  //----------------------------------------------------------------------------

  std::optional<MatchGroup> MatchMngr::createMatchGroup(const Category& cat, const int round, const int grpNum, ERR *err)
  {
    assert(err != nullptr);

    // we can only create match groups, if the category configuration is stable
    // this means, we may not be in ObjState::CAT_CONFIG or _FROZEN
    ObjState catState = cat.getState();
    if ((catState == ObjState::CAT_CONFIG) || (catState == ObjState::CAT_FROZEN))
    {
      *err = ERR::CATEGORY_STILL_CONFIGURABLE;
      return {};
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
        *err = ERR::INVALID_GROUP_NUM;
        return {};
      }
    }

    if (round <= 0)
    {
      *err = ERR::INVALID_ROUND;
      return {};
    }

    // ensure that we don't mix "normal" group numbers with "special" group numbers
    if (grpNum <= 0)
    {
      // in this case, no other match groups in this round may exist, i. e.
      // there can only be one match group for semi finals
      const auto otherGroups = getMatchGroupsForCat(cat, round);
      if (!otherGroups.empty())
      {
        *err = ERR::INVALID_GROUP_NUM;
        return {};
      }
    }
    if (grpNum > 0)
    {
      // in this case, no other match groups with group numbers
      // below zero (e. g. semi finals) may exist
      WhereClause wc;
      wc.addCol(MG_GRP_NUM, "<=", 0);
      wc.addCol(MG_ROUND, round);
      wc.addCol(MG_CAT_REF, cat.getId());
      int nOtherGroups = groupTab.getMatchCountForWhereClause(wc);
      if (nOtherGroups != 0)
      {
        *err = ERR::INVALID_GROUP_NUM;
        return {};
      }
    }

    // make sure the match group doesn't already exist
    ERR e;
    auto mg = getMatchGroup(cat, round, grpNum, &e);
    if (e == ERR::OK)    // match group exists
    {
      *err = ERR::MATCH_GROUP_EXISTS;
      return {};
    }
    if (e != ERR::NO_SUCH_MATCH_GROUP)   // catch any other error except "no such group"
    {
      *err = e;
      return {};
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
    cvc.addCol(MG_CAT_REF, cat.getId());
    cvc.addCol(MG_ROUND, round);
    cvc.addCol(MG_GRP_NUM, grpNum);
    cvc.addCol(GENERIC_STATE_FIELD_NAME, static_cast<int>(ObjState::MG_CONFIG));

    CentralSignalEmitter* cse = CentralSignalEmitter::getInstance();
    cse->beginCreateMatchGroup();
    int newId = groupTab.insertRow(cvc);
    fixSeqNumberAfterInsert(groupTab);
    cse->endCreateMatchGroup(groupTab.length() - 1); // the new sequence number is always the largest

    
    // return a match group object for the new group
    return MatchGroup{db, newId};
  }

  //----------------------------------------------------------------------------

  MatchGroupList MatchMngr::getMatchGroupsForCat(const Category& cat, int round) const
  {
    WhereClause wc;
    wc.addCol(MG_CAT_REF, cat.getId());
    if (round > 0)
    {
      wc.addCol(MG_ROUND, round);
    }

    return getObjectsByWhereClause<MatchGroup>(groupTab, wc);
  }
  
  //----------------------------------------------------------------------------

  MatchGroupList MatchMngr::getAllMatchGroups() const
  {
    return getAllObjects<MatchGroup>(groupTab);
  }

  //----------------------------------------------------------------------------

  std::optional<MatchGroup> MatchMngr::getMatchGroup(const Category& cat, const int round, const int grpNum, ERR *err)
  {
    assert(err != nullptr);

    // check round parameter for validity
    if (round <= 0)
    {
      *err = ERR::INVALID_ROUND;
      return {};
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
        *err = ERR::INVALID_GROUP_NUM;
        return {};
      }
    }
    
    // search for the match group in the database
    WhereClause wc;
    wc.addCol(MG_CAT_REF, cat.getId());
    wc.addCol(MG_ROUND, round);
    wc.addCol(MG_GRP_NUM, grpNum); 
    auto r = groupTab.getSingleRowByWhereClause2(wc);

    if (!r)
    {
      *err = ERR::NO_SUCH_MATCH_GROUP;
      return {};
    }

    *err = ERR::OK;
    return MatchGroup{db, *r};
  }

  //----------------------------------------------------------------------------

  bool MatchMngr::hasMatchGroup(const Category& cat, const int round, const int grpNum, ERR* err)
  {
    ERR e;
    auto mg = getMatchGroup(cat, round, grpNum, &e);
    Sloppy::assignIfNotNull<ERR>(err, e);
    return mg.has_value();
  }

  //----------------------------------------------------------------------------

  std::optional<Match> MatchMngr::createMatch(const MatchGroup &grp, ERR* err)
  {
    assert(err != nullptr);

    // we can only add matches to a group if the group
    // is in the config state
    if (grp.getState() != ObjState::MG_CONFIG)
    {
      *err = ERR::MATCH_GROUP_NOT_CONFIGURALE_ANYMORE;
      return {};
    }

    // Okay, parameters are valid
    // create a new match entry in the database
    ColumnValueClause cvc;
    cvc.addCol(MA_GRP_REF, grp.getId());
    cvc.addCol(GENERIC_STATE_FIELD_NAME, static_cast<int>(ObjState::MA_INCOMPLETE));
    cvc.addCol(MA_PAIR1_SYMBOLIC_VAL, 0);   // default: no symbolic name
    cvc.addCol(MA_PAIR2_SYMBOLIC_VAL, 0);   // default: no symbolic name
    cvc.addCol(MA_WINNER_RANK, -1);         // default: no rank, no knock out
    cvc.addCol(MA_LOSER_RANK, -1);         // default: no rank, no knock out
    cvc.addCol(MA_REFEREE_MODE, -1);        // -1: use current tournament default

    CentralSignalEmitter* cse = CentralSignalEmitter::getInstance();
    cse->beginCreateMatch();
    int newId = tab.insertRow(cvc);
    fixSeqNumberAfterInsert();
    cse->endCreateMatch(tab.length() - 1); // the new sequence number is always the highest

    // return a match object for the new match
    return Match(db, newId);
  }

  //----------------------------------------------------------------------------

  void MatchMngr::deleteMatchGroupAndMatch(const MatchGroup& mg) const
  {
    //
    // USE WITH EXTREME CARE!!
    //
    // This is a bad-hack convenience function. It does not perform any
    // further checking and DOES NOT EMIT ANY SIGNALS!!
    //
    // It assumes that you notify all models of the change yourself.
    //
    // The purpose of this function is to clean-up code for the
    // deletion of running categories or the "shortening" of Swiss Ladder
    // categores after a deadlock.
    //
    // MAKE SURE THAT INCOMING LINKS TO MATCH GROUP FROM THE RANKING TAB
    // HAVE BEEN DELETED BEFORE!!
    //


    auto matchesInGroup = mg.getMatches();
    for (const Match& ma : matchesInGroup)
    {
      int deletedSeqNum = ma.getSeqNum();
      tab.deleteRowsByColumnValue("id", ma.getId());
      fixSeqNumberAfterDelete(tab, deletedSeqNum);
    }

    // delete the group itself.
    int deletedSeqNum = mg.getSeqNum();
    groupTab.deleteRowsByColumnValue("id", mg.getId());
    fixSeqNumberAfterDelete(groupTab, deletedSeqNum);
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
    if (pp1.getPairId() < 1) return ERR::INVALID_PLAYER_PAIR;
    if (pp2.getPairId() < 1) return ERR::INVALID_PLAYER_PAIR;

    // check if an assignment of the player pairs is okay
    ERR e = canAssignPlayerPairToMatch(ma, pp1);
    if (e != ERR::OK) return e;
    e = canAssignPlayerPairToMatch(ma, pp2);
    if (e != ERR::OK) return e;

    // make sure that both pairs are not identical
    if (pp1.getPairId() == pp2.getPairId())
    {
      return ERR::PLAYERS_IDENTICAL;
    }

    // assign the player pairs
    ma.row.update(MA_PAIR1_REF, pp1.getPairId());
    ma.row.update(MA_PAIR2_REF, pp2.getPairId());

    // potentially, the player pairs where all that was necessary
    // to actually promote the match to e.g., WAITING
    updateMatchStatus(ma);

    return ERR::OK;
  }

  //----------------------------------------------------------------------------

  ERR MatchMngr::setPlayerPairForMatch(const Match& ma, const PlayerPair& pp, int ppPos) const
  {
    // make sure the player pair is valid (has a database entry)
    if (pp.getPairId() < 1) return ERR::INVALID_PLAYER_PAIR;

    // check if an assignment of the player pairs is okay
    ERR e = canAssignPlayerPairToMatch(ma, pp);
    if (e != ERR::OK) return e;

    // assign the player pair
    if (ppPos == 1) ma.row.update(MA_PAIR1_REF, pp.getPairId());
    if (ppPos == 2) ma.row.update(MA_PAIR2_REF, pp.getPairId());

    return ERR::OK;
  }

  //----------------------------------------------------------------------------

  ERR MatchMngr::setSymbolicPlayerForMatch(const Match& fromMatch, const Match& toMatch, bool asWinner, int dstPlayerPosInMatch) const
  {
    // Only allow changing / setting players if we not yet fully configured
    if (toMatch.getState() != ObjState::MA_INCOMPLETE) return ERR::MATCH_NOT_CONFIGURALE_ANYMORE;

    // fromMatch and toMatch must be in the same category
    int fromMatchCatId = fromMatch.getCategory().getId();
    int toMatchCatId = toMatch.getCategory().getId();
    if (fromMatchCatId != toMatchCatId)
    {
      return ERR::INVALID_MATCH_LINK;
    }

    // toMatch must be in a later round than fromMatch
    MatchGroup fromGroup = fromMatch.getMatchGroup();
    MatchGroup toGroup = toMatch.getMatchGroup();
    if (toGroup.getRound() <= fromGroup.getRound())
    {
      return ERR::INVALID_MATCH_LINK;
    }

    // okay, the link is valid

    int dstId = asWinner ? fromMatch.getId() : -(fromMatch.getId());
    if (dstPlayerPosInMatch == 1)
    {
      toMatch.row.update(MA_PAIR1_SYMBOLIC_VAL, dstId);
      toMatch.row.updateToNull(MA_PAIR1_REF);
    }
    if (dstPlayerPosInMatch == 2)
    {
      toMatch.row.update(MA_PAIR2_SYMBOLIC_VAL, dstId);
      toMatch.row.updateToNull(MA_PAIR2_REF);
    }

    return ERR::OK;
  }

  //----------------------------------------------------------------------------

  ERR MatchMngr::setPlayerToUnused(const Match& ma, int unusedPlayerPos, int winnerRank) const
  {
    // Only allow changing / setting player pairs if we not yet fully configured
    if (ma.getState() != ObjState::MA_INCOMPLETE) return ERR::MATCH_NOT_CONFIGURALE_ANYMORE;

    if (unusedPlayerPos == 1)
    {
      ma.row.updateToNull(MA_PAIR1_REF);
      ma.row.update(MA_PAIR1_SYMBOLIC_VAL, SYMBOLIC_ID_FOR_UNUSED_PLAYER_PAIR_IN_MATCH);
    }
    if (unusedPlayerPos == 2)
    {
      ma.row.updateToNull(MA_PAIR2_REF);
      ma.row.update(MA_PAIR2_SYMBOLIC_VAL, SYMBOLIC_ID_FOR_UNUSED_PLAYER_PAIR_IN_MATCH);
    }
    ma.row.update(MA_WINNER_RANK, winnerRank);

    return ERR::OK;
  }

  //----------------------------------------------------------------------------

  ERR MatchMngr::setRankForWinnerOrLoser(const Match& ma, bool isWinner, int rank) const
  {
    // Only allow changing / setting match data if we not yet fully configured
    if (ma.getState() != ObjState::MA_INCOMPLETE) return ERR::MATCH_NOT_CONFIGURALE_ANYMORE;

    // TODO: check if rank is really valid

    if (isWinner)
    {
      ma.row.update(MA_WINNER_RANK, rank);
    } else {
      ma.row.update(MA_LOSER_RANK, rank);
    }

    return ERR::OK;
  }

  //----------------------------------------------------------------------------

  ERR MatchMngr::canAssignPlayerPairToMatch(const Match &ma, const PlayerPair &pp) const
  {
    // Only allow changing / setting player pairs if we not yet fully configured
    ObjState stat = ma.getState();
    if ((stat != ObjState::MA_INCOMPLETE) && (stat != ObjState::MA_FUZZY)) return ERR::MATCH_NOT_CONFIGURALE_ANYMORE;

    // make sure the player pair and the match belong to the same category
    Category requiredCat = ma.getCategory();
    auto ppCat = pp.getCategory(db);
    if (*ppCat != requiredCat) return ERR::PLAYER_NOT_IN_CATEGORY;

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
    //   * if mgGroupNumber is below or equal zero, we don't care (see above)
    //   * if it's greater than zero, the group numbers of player pair and
    //     match group must be identical
    auto mg = ma.getMatchGroup();
    int mgGroupNumber = mg.getGroupNumber();
    if ((mgGroupNumber > 0) && (pp.getPairsGroupNum() != mgGroupNumber))
    {
      return ERR::GROUP_NUMBER_MISMATCH;
    }

    // make sure the player pair has not already been assigned
    // to other matches in the same category and round
    MatchGroup myMatchGroup = ma.getMatchGroup();
    int roundNum = myMatchGroup.getRound();
    MatchGroupList mgl = getMatchGroupsForCat(requiredCat, roundNum);
    for (const MatchGroup& mg : mgl)
    {
      for (Match otherMatch : mg.getMatches())
      {
        // skip the match we want to assign the players to
        if (otherMatch == ma) continue;

        // check the first player pair of this match, if existing
        if (otherMatch.hasPlayerPair1())
        {
          int otherPairId = otherMatch.getPlayerPair1().getPairId();
          if (otherPairId == pp.getPairId()) return ERR::PLAYER_ALREADY_ASSIGNED_TO_OTHER_MATCH_IN_THE_SAME_ROUND_AND_CATEGORY;
        }

        // check the second  player pair of this match, if existing
        if (otherMatch.hasPlayerPair2())
        {
          int otherPairId = otherMatch.getPlayerPair2().getPairId();
          if (otherPairId == pp.getPairId()) return ERR::PLAYER_ALREADY_ASSIGNED_TO_OTHER_MATCH_IN_THE_SAME_ROUND_AND_CATEGORY;
        }
      }
    }

    return ERR::OK;
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
    if (grp.getState() != ObjState::MG_CONFIG) return ERR::MATCH_GROUP_ALREADY_CLOSED;

    // the match group should contain at least one match
    if (grp.getMatchCount() < 1) return ERR::MATCH_GROUP_EMPTY;

    // we can close the group unconditionally
    grp.setState(ObjState::MG_FROZEN);
    CentralSignalEmitter::getInstance()->matchGroupStatusChanged(grp.getId(), grp.getSeqNum(), ObjState::MG_CONFIG, ObjState::MG_FROZEN);

    // call updateAllMatchGroupStates in case the group can be further promoted
    // to idle (which enables the group the be scheduled)
    updateAllMatchGroupStates(grp.getCategory());

    return ERR::OK;
  }

  //----------------------------------------------------------------------------

  ERR MatchMngr::setRefereeMode(const Match& ma, REFEREE_MODE newMode) const
  {
    // get the old mode
    REFEREE_MODE oldMode = ma.get_RAW_RefereeMode();

    // is there anything to do at all?
    if (oldMode == newMode) return ERR::OK;

    // only allow changes to the referee mode before the match has been called
    ObjState stat = ma.getState();
    if ((stat == ObjState::MA_RUNNING) || (stat == ObjState::MA_FINISHED))
    {
      return ERR::MATCH_NOT_CONFIGURALE_ANYMORE;
    }

    // set the new mode
    ma.row.update(MA_REFEREE_MODE, static_cast<int>(newMode));

    // if we go to a more restrictive mode, delete any existing
    // referee assignments
    if ((ma.hasRefereeAssigned()) && (newMode != REFEREE_MODE::ALL_PLAYERS))
    {
      ma.row.updateToNull(MA_REFEREE_REF);
    }

    // fake a match-changed-event in order to trigger UI updates
    CentralSignalEmitter::getInstance()->matchStatusChanged(ma.getId(), ma.getSeqNum(), stat, stat);

    return ERR::OK;
  }

  //----------------------------------------------------------------------------

  ERR MatchMngr::assignReferee(const Match& ma, const Player& p, REFEREE_ACTION refAction) const
  {
    ERR e = ma.canAssignReferee(refAction);
    if (e != ERR::OK) return e;

    // don't allow assignments of players that actually
    // take part in the match
    PlayerPair pp1 = ma.getPlayerPair1();
    PlayerPair pp2 = ma.getPlayerPair2();
    if (pp1.getPlayer1() == p)
    {
      return ERR::PLAYER_NOT_SUITABLE;
    }
    if (pp1.hasPlayer2() && (pp1.getPlayer2() == p))
    {
      return ERR::PLAYER_NOT_SUITABLE;
    }
    if (pp2.getPlayer1() == p)
    {
      return ERR::PLAYER_NOT_SUITABLE;
    }
    if (pp2.hasPlayer2() && (pp2.getPlayer2() == p))
    {
      return ERR::PLAYER_NOT_SUITABLE;
    }

    // ensure that the player is IDLE when calling a match or
    // swapping the umpire
    if ((refAction != REFEREE_ACTION::PRE_ASSIGN) && (p.getState() != ObjState::PL_IDLE))
    {
      return ERR::PLAYER_NOT_SUITABLE;
    }

    // store the currently assigned referee
    auto currentReferee = ma.getAssignedReferee();

    // okay, it is safe to assign the referee

    ma.row.update(MA_REFEREE_REF, p.getId());

    // if we're swapping the umpire, we have to update the player states as well
    //
    // Note: when calling the match, the player states are updated by assignMatchToCourt()
    CentralSignalEmitter* cse = CentralSignalEmitter::getInstance();
    if (refAction == REFEREE_ACTION::SWAP)
    {
      assert(currentReferee.has_value());
      currentReferee->setState(ObjState::PL_IDLE);
      cse->playerStatusChanged(currentReferee->getId(), currentReferee->getSeqNum(), ObjState::PL_REFEREE, ObjState::PL_IDLE);

      p.setState(ObjState::PL_REFEREE);
      cse->playerStatusChanged(p.getId(), p.getSeqNum(), ObjState::PL_IDLE, ObjState::PL_REFEREE);
    }

    // maybe the match status changes after the assignment, because we're now
    // waiting for a busy referee to become available
    updateMatchStatus(ma);

    // fake a match-changed-event in order to trigger UI updates
    ObjState stat = ma.getState();
    cse->matchStatusChanged(ma.getId(), ma.getSeqNum(), stat, stat);

    // in case we're calling a match or swapping the umpire:
    //
    // check all matches that are currently "READY" or "BUSY" because
    // due to the player allocation, some of them might have
    // become "BUSY" or "READY"
    static const std::string where{
      std::string{GENERIC_STATE_FIELD_NAME} + "=" + std::to_string(static_cast<int>(ObjState::MA_READY)) +
      " OR " +
      std::string{GENERIC_STATE_FIELD_NAME} + "=" + std::to_string(static_cast<int>(ObjState::MA_BUSY))
    };
    for (const Match& otherMatch : getObjectsByWhereClause<Match>(where))
    {
      ObjState otherStat = otherMatch.getState();

      // upon match call, other matches can only switch from READY to BUSY
      // because we're allocating players
      if ((refAction == REFEREE_ACTION::MATCH_CALL) && (otherStat == ObjState::MA_READY))
      {
        updateMatchStatus(otherMatch);
      }

      // upon umpire swap, other matches can switch from READY to BUSY or from BUSY to READY
      // because we're allocating the new umpire and release the old umpire
      if ((refAction == REFEREE_ACTION::SWAP) && ((otherStat == ObjState::MA_READY) || (otherStat == ObjState::MA_BUSY)))
      {
        updateMatchStatus(otherMatch);
      }
    }

    return ERR::OK;
  }

  //----------------------------------------------------------------------------

  ERR MatchMngr::removeReferee(const Match& ma) const
  {
    ObjState stat = ma.getState();
    if ((stat == ObjState::MA_RUNNING) || (stat == ObjState::MA_FINISHED))
    {
      return ERR::MATCH_NOT_CONFIGURALE_ANYMORE;
    }

    ma.row.updateToNull(MA_REFEREE_REF);

    // maybe the match status changes after the removal, because we're not
    // waiting anymore for a busy referee to become available
    updateMatchStatus(ma);

    // fake a match-changed-event in order to trigger UI updates
    CentralSignalEmitter::getInstance()->matchStatusChanged(ma.getId(), ma.getSeqNum(), stat, stat);

    return ERR::OK;
  }

  //----------------------------------------------------------------------------

  ERR MatchMngr::swapPlayer(const Match& ma, const PlayerPair& ppOld, const PlayerPair& ppNew) const
  {
    // the matches may not be "running" or "finished"
    ObjState stat = ma.getState();
    if ((stat == ObjState::MA_RUNNING) || (stat == ObjState::MA_FINISHED)) return ERR::WRONG_STATE;

    // the new player pair must belong to the
    // same category as the old one
    auto oldPairCat = ppOld.getCategory(db);
    if (!oldPairCat) return ERR::INVALID_PLAYER_PAIR;
    auto newPairCat = ppNew.getCategory(db);
    if (!newPairCat) return ERR::INVALID_PLAYER_PAIR;
    if (oldPairCat->getId() != newPairCat->getId()) return ERR::INVALID_PLAYER_PAIR;

    /*
     * 2019-08-19:
     *
     * FIX: don't we need a check that ppNew is not already assigned
     * to some other match? At least in the same round?
     *
     * Would that fit to "swapPlayers()" below? That's one of the callers
     * of this function.
     *
     */

    // find the position of the old player pair in the match
    int ppPos = 0;
    if (ma.hasPlayerPair1())
    {
      PlayerPair pp = ma.getPlayerPair1();
      if (pp.getPairId() == ppOld.getPairId()) ppPos = 1;
    }
    if (ma.hasPlayerPair2())
    {
      PlayerPair pp = ma.getPlayerPair2();
      if (pp.getPairId() == ppOld.getPairId()) ppPos = 2;
    }
    if (ppPos == 0)
    {
      return ERR::INVALID_PLAYER_PAIR;   // ppOld is not part of the match
    }

    // if both pairs are identical, we're done
    if (ppOld.getPairId() == ppNew.getPairId()) return ERR::OK;

    // start a transaction
    try
    {
      auto trans = db.get().startTransaction();

      // actually swap the players
      if (ppPos == 1) ma.row.update(MA_PAIR1_REF, ppNew.getPairId());
      if (ppPos == 2) ma.row.update(MA_PAIR2_REF, ppNew.getPairId());

      // make sure that the newly assigned player
      // is not already foreseen as a referee
      auto ref = ma.getAssignedReferee();
      if (ref)
      {
        Player p = ppNew.getPlayer1();
        if (p.getId() == ref->getId()) removeReferee(ma);
        if (ppNew.hasPlayer2())
        {
          Player p = ppNew.getPlayer2();
          if (p.getId() == ref->getId()) removeReferee(ma);
        }
      }

      // update the match status because ready / busy might
      // have changed due to the player swap
      updateMatchStatus(ma);

      trans.commit();
      return ERR::OK;
    }
    catch (SqliteOverlay::BusyException&)
    {
      return ERR::DATABASE_ERROR;
    }
    catch (SqliteOverlay::GenericSqliteException&)
    {
      return ERR::DATABASE_ERROR;
    }
  }

  //----------------------------------------------------------------------------

  ERR MatchMngr::swapPlayers(const Match& ma1, const PlayerPair& ma1PlayerPair, const Match& ma2, const PlayerPair& ma2PlayerPair) const
  {
    // the matches may not be "running" or "finished"
    for (const Match& m : {ma1, ma2})
    {
      ObjState stat = m.getState();
      if ((stat == ObjState::MA_RUNNING) || (stat == ObjState::MA_FINISHED)) return ERR::WRONG_STATE;
    }

    // the matches must belong to the same category
    if (ma1.getCategory() != ma2.getCategory())
    {
      return ERR::INVALID_ID;  // not the best matching error code, but anyway...
    }

    try
    {
      auto trans = db.get().startTransaction();

      // swap the players
      ERR e = swapPlayer(ma1, ma1PlayerPair, ma2PlayerPair);
      if (e != ERR::OK)
      {
        return e;  // triggers implicit rollback
      }

      e = swapPlayer(ma2, ma2PlayerPair, ma1PlayerPair);
      if (e != ERR::OK)
      {
        return e;  // triggers implicit rollback
      }

      trans.commit();

      return ERR::OK;
    }
    catch (SqliteOverlay::BusyException&)
    {
      return ERR::DATABASE_ERROR;
    }
    catch (SqliteOverlay::GenericSqliteException&)
    {
      return ERR::DATABASE_ERROR;
    }
  }

  //----------------------------------------------------------------------------

  std::string MatchMngr::getSyncString(const std::vector<int>& rows) const
  {
    std::vector<Sloppy::estring> cols = {"id", GENERIC_STATE_FIELD_NAME, MA_GRP_REF, MA_NUM, MA_PAIR1_REF, MA_PAIR2_REF,
                          MA_ACTUAL_PLAYER1A_REF, MA_ACTUAL_PLAYER1B_REF, MA_ACTUAL_PLAYER2A_REF, MA_ACTUAL_PLAYER2B_REF,
                          MA_RESULT, MA_COURT_REF, MA_START_TIME, MA_ADDITIONAL_CALL_TIMES, MA_FINISH_TIME,
                           MA_PAIR1_SYMBOLIC_VAL, MA_PAIR2_SYMBOLIC_VAL, MA_WINNER_RANK, MA_LOSER_RANK,
                           MA_REFEREE_MODE, MA_REFEREE_REF};

    return db.get().getSyncStringForTable(TAB_MATCH, cols, rows);
  }

  //----------------------------------------------------------------------------

  std::string MatchMngr::getSyncString_MatchGroups(std::vector<int> rows)
  {
    std::vector<Sloppy::estring> cols = {"id", MG_CAT_REF, GENERIC_STATE_FIELD_NAME, MG_ROUND, MG_GRP_NUM};

    return db.get().getSyncStringForTable(TAB_MATCH_GROUP, cols, rows);
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
  void MatchMngr::updateAllMatchGroupStates(const Category& cat) const
  {
    CentralSignalEmitter* cse = CentralSignalEmitter::getInstance();

    // transition from SCHEDULED to FINISHED
    WhereClause wc;
    wc.addCol(MG_CAT_REF, cat.getId());
    wc.addCol(GENERIC_STATE_FIELD_NAME, static_cast<int>(ObjState::MG_SCHEDULED));
    for (TabRowIterator it{db, TAB_MATCH_GROUP, wc}; it.hasData(); ++it)
    {
      const MatchGroup mg{db, *it};

      // check all matches in this scheduled category
      bool isfinished = true;
      for (auto match : mg.getMatches())
      {
        if (match.getState() != ObjState::MA_FINISHED)
        {
          isfinished = false;
          break;
        }
      }
      if (isfinished)
      {
        mg.setState(ObjState::MG_FINISHED);
        cse->matchGroupStatusChanged(mg.getId(), mg.getSeqNum(), ObjState::MG_SCHEDULED, ObjState::MG_FINISHED);
      }
    }

    // transition from FROZEN to IDLE
    wc.clear();
    wc.addCol(MG_CAT_REF, cat.getId());
    wc.addCol(GENERIC_STATE_FIELD_NAME, static_cast<int>(ObjState::MG_FROZEN));
    for (TabRowIterator it{db, TAB_MATCH_GROUP, wc}; it.hasData(); ++it)
    {
      const MatchGroup mg{db, *it};

      // Condition: all match groups of the same players group with lower
      // round numbers must be staged
      int round = mg.getRound();
      int mgGroupNum = mg.getGroupNumber();

      bool canPromote = true;
      for (int r=1; r < round; ++r)
      {
        MatchGroupList mglSubset = getMatchGroupsForCat(cat, r);
        for (const auto& mg2 : mglSubset)
        {
          int mg2GroupNum = mg2.getGroupNumber();

          // skip match groups associated with other player groups
          //
          // only applies to "real" player groups (group num > 0)
          if ((mgGroupNum > 0) && (mg2GroupNum > 0) && (mg2.getGroupNumber() != mg.getGroupNumber())) continue;

          ObjState mg2Stat = mg2.getState();
          if (!((mg2Stat == ObjState::MG_STAGED) || (mg2Stat == ObjState::MG_SCHEDULED) || (mg2Stat == ObjState::MG_FINISHED)))
          {
            canPromote = false;
            break;
          }
        }

        if (!canPromote) break;  // no need to loop through other rounds
      }

      if (canPromote)
      {
        mg.setState(ObjState::MG_IDLE);
        cse->matchGroupStatusChanged(mg.getId(), mg.getSeqNum(), ObjState::MG_FROZEN, ObjState::MG_IDLE);
      }
    }

  }

  //----------------------------------------------------------------------------

  std::optional<MatchGroup> MatchMngr::getMatchGroupBySeqNum(int mgSeqNum)
  {
    return getSingleObjectByColumnValue<MatchGroup>(groupTab, GENERIC_SEQNUM_FIELD_NAME, mgSeqNum);
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
    if (e != ERR::OK) return e;

    // no further checks necessary. Any IDLE match group can be promoted

    // promote the group to STAGED and assign a sequence number
    int nextStageSeqNum = getMaxStageSeqNum() + 1;
    grp.setState(ObjState::MG_STAGED);
    grp.row.update(MG_STAGE_SEQ_NUM, nextStageSeqNum);
    CentralSignalEmitter::getInstance()->matchGroupStatusChanged(grp.getId(), grp.getSeqNum(), ObjState::MG_IDLE, ObjState::MG_STAGED);

    // promote other groups from FROZEN to IDLE, if applicable
    updateAllMatchGroupStates(grp.getCategory());

    return ERR::OK;
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
    int statId = static_cast<int>(ObjState::MG_STAGED);
    if (groupTab.getMatchCountForColumnValue(GENERIC_STATE_FIELD_NAME, statId) < 1)
    {
      return 0;  // no staged match groups so far
    }

    // determine the max sequence number
    Sloppy::estring sql{"SELECT max(%1) FROM %2"};
    sql.arg(MG_STAGE_SEQ_NUM);
    sql.arg(TAB_MATCH_GROUP);
    try
    {
      return db.get().execScalarQueryInt(sql);
    }
    catch (...)
    {
      return 0;  // shouldn't happen, but anyway...
    }
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
    if (grp.getState() != ObjState::MG_STAGED)
    {
      return ERR::MATCH_GROUP_NOT_UNSTAGEABLE;
    }

    int round = grp.getRound();
    int playersGroup = grp.getGroupNumber();
    int catId = grp.getCategory().getId();

    // check for a match group with higher round number in the staging area
    WhereClause wc;
    wc.addCol(MG_ROUND, round + 1);
    wc.addCol(GENERIC_STATE_FIELD_NAME, static_cast<int>(ObjState::MG_STAGED));
    wc.addCol(MG_CAT_REF, catId);
    if (groupTab.getMatchCountForWhereClause(wc) == 0)
    {
      // there is no match group of this category and with a higher
      // round number in the staging area
      // ==> the match group can be demoted
      return ERR::OK;
    }

    //
    // obviously there is at least one match group of this category
    // and with a higher round number staged
    //

    // now compare the players groups to properly check the transition
    // between round robin rounds and KO-rounds
    //
    if (playersGroup < 0)
    {
      // this group is already in KO phase, so the one in the higher round
      // must be as well. So the fact that there is a group
      // with a higher round number already staged is a no-go for unstaging
      // this group
      return ERR::MATCH_GROUP_NOT_UNSTAGEABLE;
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
        //
        // 2019-08-19: FIX, is that correct or too strict?
        return ERR::MATCH_GROUP_NOT_UNSTAGEABLE;
      }

      // the next round's group belongs to the same
      // players group ==> can't unstage
      if (nextGroupNumber == playersGroup)
      {
        return ERR::MATCH_GROUP_NOT_UNSTAGEABLE;
      }
    }

    return ERR::OK;
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
    if (e != ERR::OK) return e;

    // great, we can safely demote this match group to IDLE
    grp.setState(ObjState::MG_IDLE);

    // store and delete old stage sequence number
    int oldStageSeqNumber = grp.getStageSequenceNumber();
    grp.row.updateToNull(MG_STAGE_SEQ_NUM);

    CentralSignalEmitter* cse = CentralSignalEmitter::getInstance();

    cse->matchGroupStatusChanged(grp.getId(), grp.getSeqNum(), ObjState::MG_STAGED, ObjState::MG_IDLE);

    // update all subsequent sequence numbers
    WhereClause wc;
    wc.addCol(MG_STAGE_SEQ_NUM, ">", oldStageSeqNumber);
    for (const auto& mg : getObjectsByWhereClause<MatchGroup>(groupTab, wc))
    {
      int old = mg.getStageSequenceNumber();
      mg.row.update(MG_STAGE_SEQ_NUM, old - 1);
      cse->matchGroupStatusChanged(mg.getId(), mg.getSeqNum(), ObjState::MG_STAGED, ObjState::MG_STAGED);
    }

    // demote other groups from IDLE to FROZEN
    // check for a match group with "round + 1" in state IDLE
    int round = grp.getRound();
    int playersGroup = grp.getGroupNumber();
    int catId = grp.getCategory().getId();

    wc.clear();
    wc.addCol(MG_ROUND, round+1);
    wc.addCol(GENERIC_STATE_FIELD_NAME, static_cast<int>(ObjState::MG_IDLE));
    wc.addCol(MG_CAT_REF, catId);
    for (MatchGroup mg : getObjectsByWhereClause<MatchGroup>(groupTab, wc))
    {
      // if our demoted match group and the IDLE match group are round-robin-groups,
      // the IDLE match group has only to be demoted if it matches the "grp's" players group
      //
      // or in inverse logic: if the player groups do not match, we don't have to
      // touch this group
      int otherPlayersGroup = mg.getGroupNumber();
      if ((playersGroup > 0) && (otherPlayersGroup > 0) && (playersGroup != otherPlayersGroup))
      {
        continue;
      }

      // in all other cases, the "IDLE" group has to be demoted to FROZEN
      mg.setState(ObjState::MG_FROZEN);
      cse->matchGroupStatusChanged(mg.getId(), mg.getSeqNum(), ObjState::MG_IDLE, ObjState::MG_FROZEN);
    }

    return ERR::OK;
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
    return (grp.getState() == ObjState::MG_IDLE) ? ERR::OK : ERR::WRONG_STATE;
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
    ObjState curState = ma.getState();

    CentralSignalEmitter* cse = CentralSignalEmitter::getInstance();

    // from INCOMPLETE to WAITING
    if (curState == ObjState::MA_INCOMPLETE)
    {
      if (ma.hasBothPlayerPairs() && (ma.getMatchNumber() != MATCH_NUM_NOT_ASSIGNED))
      {
        ma.setState(ObjState::MA_WAITING);
        curState = ObjState::MA_WAITING;
        cse->matchStatusChanged(ma.getId(), ma.getSeqNum(), ObjState::MA_INCOMPLETE, ObjState::MA_WAITING);
      }
    }

    // from INCOMPLETE to FUZZY
    if (curState == ObjState::MA_INCOMPLETE)
    {
      bool isFuzzy1 = (ma.row.getInt(MA_PAIR1_SYMBOLIC_VAL) != 0);
      bool isFuzzy2 = (ma.row.getInt(MA_PAIR2_SYMBOLIC_VAL) != 0);
      bool hasMatchNumber = ma.getMatchNumber() > 0;

      // we shall never have a symbolic and a real player assignment at the same time
      assert((isFuzzy1 && ma.hasPlayerPair1()) == false);
      assert((isFuzzy2 && ma.hasPlayerPair2()) == false);

      // 2019-08-19:
      // FIX: shouldn't this condition be: "(isFuzzy1 && isFuzzy2 && hasMatchNumber)"?
      //
      // The definition of ObjState::MA_FUZZY sais: "Player names are defined by symbolic values (e.g., winner of match XYZ); match number is assigned"
      if (isFuzzy1 || isFuzzy2 || hasMatchNumber)
      {
        ma.setState(ObjState::MA_FUZZY);
        curState = ObjState::MA_FUZZY;
        cse->matchStatusChanged(ma.getId(), ma.getSeqNum(), ObjState::MA_INCOMPLETE, ObjState::MA_FUZZY);
      }
    }

    // transition from FUZZY to WAITING:
    // if we've resolved all symbolic references of a match, it may be promoted from
    // FUZZY at least to WAITING, maybe even to READY or BUSY
    if (curState == ObjState::MA_FUZZY)
    {
      bool isFixed1 = ((ma.row.getInt(MA_PAIR1_SYMBOLIC_VAL) == 0) && (ma.row.getInt(MA_PAIR1_REF) > 0));
      bool isFixed2 = ((ma.row.getInt(MA_PAIR2_SYMBOLIC_VAL) == 0) && (ma.row.getInt(MA_PAIR2_REF) > 0));
      bool hasMatchNumber = ma.getMatchNumber() > 0;

      if (isFixed1 && isFixed2 && hasMatchNumber)
      {
        ma.setState(ObjState::MA_WAITING);
        curState = ObjState::MA_WAITING;
        cse->matchStatusChanged(ma.getId(), ma.getSeqNum(), ObjState::MA_FUZZY, ObjState::MA_WAITING);
      }
    }

    // from WAITING to READY or BUSY
    PlayerMngr pm{db};
    bool playersAvail = ((pm.canAcquirePlayerPairsForMatch(ma)) == ERR::OK);

    bool hasPredecessor = hasUnfinishedMandatoryPredecessor(ma);
    if ((curState == ObjState::MA_WAITING) && (!hasPredecessor))
    {
      curState = playersAvail ? ObjState::MA_READY : ObjState::MA_BUSY;
      ma.setState(curState);
      cse->matchStatusChanged(ma.getId(), ma.getSeqNum(), ObjState::MA_WAITING, curState);
    }

    // from READY to BUSY
    if ((curState == ObjState::MA_READY) && !playersAvail)
    {
      ma.setState(ObjState::MA_BUSY);
      curState = ObjState::MA_BUSY;
      cse->matchStatusChanged(ma.getId(), ma.getSeqNum(), ObjState::MA_READY, ObjState::MA_BUSY);
    }

    // from BUSY to READY
    if ((curState == ObjState::MA_BUSY) && playersAvail)
    {
      ma.setState(ObjState::MA_READY);
      curState = ObjState::MA_READY;
      cse->matchStatusChanged(ma.getId(), ma.getSeqNum(), ObjState::MA_BUSY, ObjState::MA_READY);
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
      if (prevMg.getState() != ObjState::MG_FINISHED) return true;
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

    CentralSignalEmitter* cse = CentralSignalEmitter::getInstance();

    for (auto mg : getStagedMatchGroupsOrderedBySequence())
    {
      for (auto ma : mg.getMatches())
      {
        ma.row.update(MA_NUM, nextMatchNumber);
        updateMatchStatus(ma);

        // Manually trigger (another) update, because assigning the match number
        // does not change the match state in all cases. So we need to have at
        // least this one trigger to tell everone that the data has changed
        cse->matchStatusChanged(ma.getId(), ma.getSeqNum(), ma.getState(), ma.getState());

        ++nextMatchNumber;
      }

      // update the match group's state
      mg.setState(ObjState::MG_SCHEDULED);
      mg.row.updateToNull(MG_STAGE_SEQ_NUM);  // delete the sequence number
      cse->matchGroupStatusChanged(mg.getId(), mg.getSeqNum(), ObjState::MG_STAGED, ObjState::MG_SCHEDULED);
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
    WhereClause wc;
    wc.addCol(MG_STAGE_SEQ_NUM, ">", 0);
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
    /*
    // Is there any scheduled match at all?
    WhereClause wc;
    wc.addCol(MA_NUM, ">", 0);
    if (tab.getMatchCountForWhereClause(wc) < 1)
    {
      return 0;  // no assigned match numbers so far
    }
    */

    // determine the max match number
    Sloppy::estring sql = "SELECT max(%1) FROM %2";
    sql.arg(MA_NUM);
    sql.arg(TAB_MATCH);
    try
    {
      return db.get().execScalarQueryIntOrNull(sql).value_or(0);
    }
    catch (SqliteOverlay::NoDataException&)
    {
      return 0;
    }
  }

  //----------------------------------------------------------------------------

  std::optional<Match> MatchMngr::getMatchBySeqNum(int maSeqNum) const
  {
    return getSingleObjectByColumnValue<Match>(GENERIC_SEQNUM_FIELD_NAME, maSeqNum);
  }

  //----------------------------------------------------------------------------

  std::optional<Match> MatchMngr::getMatchByMatchNum(int maNum) const
  {
    return getSingleObjectByColumnValue<Match>(MA_NUM, maNum);
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
    assert(matchId != nullptr);
    assert(courtId != nullptr);

    // default return values: error
    *matchId = -1;
    *courtId = -1;

    // find the next available match with the lowest match number
    int reqState = static_cast<int>(ObjState::MA_READY);
    WhereClause wc;
    wc.addCol(GENERIC_STATE_FIELD_NAME, reqState);
    wc.setOrderColumn_Asc(MA_NUM);
    auto matchRow = tab.get2(wc);
    if (!matchRow)
    {
      return ERR::NO_MATCH_AVAIL;
    }

    ERR err;
    CourtMngr cm{db};
    auto nextCourt = cm.autoSelectNextUnusedCourt(&err, includeManualCourts);
    if (err == ERR::OK)
    {
      *matchId = matchRow->id();
      *courtId = nextCourt->getId();
      return ERR::OK;
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
    if (ma.getState() != ObjState::MA_READY)
    {
      return ERR::MATCH_NOT_RUNNABLE;
    }

    //
    // TODO: we shouldn't be able to start matches if the
    // associated category is in a state other than PLAYING
    // or IDLE. In particular, we shouldn't call games
    // if we're in WAITING_FOR_SEEDING.
    //
    // Maybe this needs to be solved here or maybe we should
    // update the match status when we leave IDLE or PLAYING
    //

    // check the player's availability
    PlayerMngr pm{db};
    ERR e = pm.canAcquirePlayerPairsForMatch(ma);
    if (e != ERR::OK) return e;  // ERR::PLAYER_NOT_IDLE

    // check if we have the necessary umpire, if required
    REFEREE_MODE refMode = ma.get_EFFECTIVE_RefereeMode();
    if ((refMode != REFEREE_MODE::NONE) && (refMode != REFEREE_MODE::HANDWRITTEN))
    {
      auto referee = ma.getAssignedReferee();

      if (!referee) return ERR::MATCH_NEEDS_REFEREE;

      // check if the assigned referee is available
      if (referee->getState() != ObjState::PL_IDLE) return ERR::REFEREE_NOT_IDLE;
    }

    // check the court's availability
    ObjState stat = court.getState();
    if (stat == ObjState::CO_AVAIL)
    {
      return ERR::OK;
    }
    if (stat == ObjState::CO_DISABLED)
    {
      return ERR::COURT_DISABLED;
    }

    return ERR::COURT_BUSY;
  }

  //----------------------------------------------------------------------------

  ERR MatchMngr::assignMatchToCourt(const Match &ma, const Court &court) const
  {
    ERR e = canAssignMatchToCourt(ma, court);
    if (e != ERR::OK) return e;

    // NORMALLY, we should first acquire the court and then assign the
    // match to this court. BUT acquiring triggers an update of the
    // associate court views and we want the update to show the match
    // details, too. So we first assign the match and formally acquire the
    // court afterwards. This way, the match is already linked to the court
    // when all views are updated.


    // EXTREMELY IMPORTANT:
    // Always adhere to this sequence when updating object states:
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
    cvc.addCol(MA_COURT_REF, court.getId());

    // copy the actual players to the database
    // TODO: implement substitute players etc. So far, we only copy
    // the contents of the player pairs blindly
    PlayerPair pp = ma.getPlayerPair1();
    cvc.addCol(MA_ACTUAL_PLAYER1A_REF, pp.getPlayer1().getId());
    if (pp.hasPlayer2())
    {
      cvc.addCol(MA_ACTUAL_PLAYER1B_REF, pp.getPlayer2().getId());
    }
    pp = ma.getPlayerPair2();
    cvc.addCol(MA_ACTUAL_PLAYER2A_REF, pp.getPlayer1().getId());
    if (pp.hasPlayer2())
    {
      cvc.addCol(MA_ACTUAL_PLAYER2B_REF, pp.getPlayer2().getId());
    }

    // update the match state
    cvc.addCol(GENERIC_STATE_FIELD_NAME, static_cast<int>(ObjState::MA_RUNNING));

    // execute all updates at once
    auto trans = db.get().startTransaction();

    ma.row.update(cvc);

    // tell the world that the match status has changed
    CentralSignalEmitter::getInstance()->matchStatusChanged(ma.getId(), ma.getSeqNum(), ObjState::MA_READY, ObjState::MA_RUNNING);

    // update the player's status
    PlayerMngr pm{db};
    e = pm.acquirePlayerPairsForMatch(ma);
    assert(e == ERR::OK);  // must be, because the condition has been check by canAssignMatchToCourt()

    // acquire the referee, if any
    REFEREE_MODE refMode = ma.get_EFFECTIVE_RefereeMode();
    if ((refMode != REFEREE_MODE::NONE) && (refMode != REFEREE_MODE::HANDWRITTEN))
    {
      auto referee = ma.getAssignedReferee();

      // the following assertion must hold,
      // because the conditions have been check by canAssignMatchToCourt()
      assert(referee);
      assert(referee->getState() == ObjState::PL_IDLE);

      referee->setState(ObjState::PL_REFEREE);
    }

    // store the effective referee-mode at call time.
    //
    // This is necessary because cmdAssignRefereeToMatch wouldn't work
    // correctly if the referee mode is USE DEFAULT and we change the
    // default mode e.g. to NONE after the match has been called. The
    // same applies to the CourtItemDelegate.
    //
    // Note: since we overwrite the mode in the match tab, the old mode
    // is not restored when undoing the match call.
    if (ma.get_RAW_RefereeMode() == REFEREE_MODE::USE_DEFAULT)
    {
      // Note: we don't use setRefereeMode() here, because a few lines
      // above we've already changed the match state to RUNNING and
      // setRefereeMode() refuses to update matches in state RUNNING.
      // So we have to hard-code the mode change here
      auto cfg = SqliteOverlay::KeyValueTab{db.get(), TAB_CFG};
      int tnmtDefaultRefereeModeId = cfg.getInt(CFG_KEY_DEFAULT_REFEREE_MODE);
      ma.row.update(MA_REFEREE_MODE, tnmtDefaultRefereeModeId);
    }

    // now we finally acquire the court in the aftermath
    CourtMngr cm{db};
    bool isOkay = cm.acquireCourt(court);
    assert(isOkay);

    // update the category's state to "PLAYING", if necessary
    CatMngr catm{db};
    catm.updateCatStatusFromMatchStatus(ma.getCategory());

    // store the call time in the database
    ma.row.update(MA_START_TIME, UTCTimestamp());

    // check all matches that are currently "READY" because
    // due to the player allocation, some of them might have
    // become "BUSY"
    WhereClause wc;
    wc.addCol(GENERIC_STATE_FIELD_NAME, static_cast<int>(ObjState::MA_READY));
    for (TabRowIterator it{db, TAB_MATCH, wc}; it.hasData(); ++it)
    {
      const Match otherMatch{db, *it};
      updateMatchStatus(otherMatch);
    }

    trans.commit();

    return ERR::OK;
  }

  //----------------------------------------------------------------------------

  std::optional<Court> MatchMngr::autoAssignMatchToNextAvailCourt(const Match &ma, ERR *err, bool includeManualCourts) const
  {
    assert(err != nullptr);

    ERR e;
    CourtMngr cm{db};
    auto nextCourt = cm.autoSelectNextUnusedCourt(&e, includeManualCourts);
    if (nextCourt)
    {
      *err = assignMatchToCourt(ma, *nextCourt);
      return (*err == ERR::OK) ? *nextCourt : std::optional<Court>{};
    }

    // return the error resulting from the court selection
    *err = e;
    return {};
  }

  //----------------------------------------------------------------------------

  ERR MatchMngr::setMatchScoreAndFinalizeMatch(const Match &ma, const MatchScore &score, bool isWalkover) const
  {
    // check the match's state
    ObjState oldState = ma.getState();
    if ((!isWalkover) && (oldState != ObjState::MA_RUNNING))
    {
      return ERR::MATCH_NOT_RUNNING;
    }
    if (isWalkover && (!(ma.isWalkoverPossible())))
    {
      return ERR::WRONG_STATE;
    }

    // check if the score is valid for the category settings
    Category cat = ma.getCategory();
    bool isDrawAllowed = cat.isDrawAllowedInRound(ma.getMatchGroup().getRound());
    int numWinGames = 2; // TODO: this needs to become a category parameter!
    if (!(score.isValidScore(numWinGames, isDrawAllowed)))
    {
      return ERR::INVALID_MATCH_RESULT_FOR_CATEGORY_SETTINGS;
    }

    // conserve the round status BEFORE we finalize the match.
    // we need this information later to detect whether we've finished
    // a round or not
    int lastFinishedRoundBeforeMatch = ma.getCategory().getRoundStatus().getFinishedRoundsCount();


    // EXTREMELY IMPORTANT:
    // Always adhere to this sequence when updating object states:
    //   1) the match that's being finished
    //   2) player status
    //   -) step (2) automatically triggers an update of applicable other matches to BUSY
    //   3) court
    //   4) match group status to FINISH, based on match completion
    //   5) other matches from WAITING to READY or BUSY, based on match sequence logic
    //   6) category states


    // everything is fine, so write the result to the database
    // and update the match status

    // wrap all changes in one giant commit
    try
    {
      auto trans = db.get().startTransaction();

      // store score and FINISH status
      ColumnValueClause cvc;
      cvc.addCol(MA_RESULT, score.toString().toUtf8().constData());
      cvc.addCol(GENERIC_STATE_FIELD_NAME, static_cast<int>(ObjState::MA_FINISHED));

      // store the finish time in the database, but only if this is not
      // a walkover and only if the match was started regularly
      if ((oldState == ObjState::MA_RUNNING) && !isWalkover)   // match was called normally, so we have a start time
      {
        UTCTimestamp now;
        cvc.addCol(MA_FINISH_TIME, &now);
      }

      // apply the update
      ma.row.update(cvc);

      // let the world know what has happened
      int maId = ma.getId();
      int maSeqNum = ma.getSeqNum();
      CentralSignalEmitter* cse = CentralSignalEmitter::getInstance();
      cse->matchResultUpdated(maId, maSeqNum);
      cse->matchStatusChanged(maId, maSeqNum, oldState, ObjState::MA_FINISHED);

      // if this was a regular, running match we need to release the court
      // and the players
      PlayerMngr pm{db};
      CourtMngr cm{db};
      if (oldState == ObjState::MA_RUNNING)
      {
        // release the players
        ERR err = pm.releasePlayerPairsAfterMatch(ma);
        if (err != ERR::OK) return err;

        // release the umpire, if any
        auto referee = ma.getAssignedReferee();
        if (referee && (referee->getState() == ObjState::PL_REFEREE))
        {
          referee->setState(ObjState::PL_IDLE);
          pm.increaseRefereeCountForPlayer(*referee);
        }

        // release the court
        auto court = ma.getCourt(&err);
        if (err != ERR::OK) return err;
        bool isOkay = cm.releaseCourt(*court);
        if (!isOkay) return ERR::DATABASE_ERROR;
      }

      // update the match group
      updateAllMatchGroupStates(ma.getCategory());

      // update other matches in this category from WAITING to READY or BUSY, if applicable
      for (MatchGroup mg : getMatchGroupsForCat(ma.getCategory()))
      {
        for (const Match& otherMatch : getMatchesForMatchGroup(mg))
        {
          if (otherMatch.getState() != ObjState::MA_WAITING) continue;
          updateMatchStatus(otherMatch);
        }
      }

      // in case some other match refers to this match with a symbolic name
      // (e.g., winner of match XYZ), resolve those symbolic names into
      // real player pairs
      resolveSymbolicNamesAfterFinishedMatch(ma);

      // update the category's state to "FINALIZED", if necessary
      CatMngr catm{db};
      catm.updateCatStatusFromMatchStatus(cat);

      // get the round status AFTER the match and check whether
      // we've just finished a round
      int lastFinishedRoundAfterMatch = ma.getCategory().getRoundStatus().getFinishedRoundsCount();
      if (lastFinishedRoundBeforeMatch != lastFinishedRoundAfterMatch)
      {
        // call the hook for finished rounds (e.g., for updating the ranking information
        // or for generating new matches)
        auto specialCat = ma.getCategory().convertToSpecializedObject();
        specialCat->onRoundCompleted(lastFinishedRoundAfterMatch);
        cse->roundCompleted(ma.getCategory().getId(), lastFinishedRoundAfterMatch);
      }

      // check all matches that are currently "BUSY" because
      // due to the player release, some of them might have
      // become "READY"
      for (const Match& otherMatch : getObjectsByColumnValue<Match>(GENERIC_STATE_FIELD_NAME, static_cast<int>(ObjState::MA_BUSY)))
      {
        updateMatchStatus(otherMatch);
      }

      // commit all changes
      trans.commit();

      return ERR::OK;
    }
    catch (BusyException&)
    {
      return ERR::DATABASE_ERROR;
    }
    catch (GenericSqliteException&)
    {
      return ERR::DATABASE_ERROR;
    }
  }

  //----------------------------------------------------------------------------

  ERR MatchMngr::updateMatchScore(const Match& ma, const MatchScore& newScore, bool winnerLoserChangePermitted) const
  {
    //
    // IMPORTANT:
    // This method __blindly__ sets the match score to whatever is provided by
    // the caller.
    // It is the caller's responsibility to ensure that a modification of the
    // match score is possible at all
    //

    // only modify finished matches
    if (ma.getState() != ObjState::MA_FINISHED) return ERR::WRONG_STATE;

    // make sure the score itself is valid
    Category cat = ma.getCategory();
    bool isDrawAllowed = cat.isDrawAllowedInRound(ma.getMatchGroup().getRound());
    int numWinGames = 2; // TODO: this needs to become a category parameter!
    if (!(newScore.isValidScore(numWinGames, isDrawAllowed)))
    {
      return ERR::INVALID_MATCH_RESULT_FOR_CATEGORY_SETTINGS;
    }

    // if no change of the winner/loser is permitted, compare
    // the new score with the old one
    if (!winnerLoserChangePermitted)
    {
      auto oldScore = ma.getScore();

      if (oldScore->getWinner() != newScore.getWinner())
      {
        // well, the error code doesn't match exactly... but we just take what's there
        return ERR::INCONSISTENT_MATCH_RESULT_STRING;
      }
    }

    // everything is fine, so write the result to the database
    ma.row.update(MA_RESULT, newScore.toString().toUtf8().constData());

    CentralSignalEmitter* cse = CentralSignalEmitter::getInstance();
    cse->matchResultUpdated(ma.getId(), ma.getSeqNum());

    return ERR::OK;
  }

  //----------------------------------------------------------------------------

  ERR MatchMngr::walkover(const Match& ma, int winningPlayerNum) const
  {
    // for a walkover, the match must be in READY, WAITING, RUNNING or BUSY
    if (!(ma.isWalkoverPossible()))
    {
      return ERR::WRONG_STATE;
    }

    // the playerNum must be either 1 or 2
    if ((winningPlayerNum != 1) && (winningPlayerNum != 2))
    {
      return ERR::INVALID_PLAYER_PAIR;
    }

    // determine the game results
    int sc1 = (winningPlayerNum == 1) ? 21 : 0;
    int sc2 = (winningPlayerNum == 1) ? 0 : 21;

    // fake a match result
    int numWinGames = 2;    // TODO: this should become a category parameter
    GameScoreList gsl;
    for (int i=0; i < numWinGames; ++i)
    {
      gsl.append(*(GameScore::fromScore(sc1, sc2)));
    }
    auto ms = MatchScore::fromGameScoreListWithoutValidation(gsl);
    assert(ms);

    return setMatchScoreAndFinalizeMatch(ma, *ms, true);
  }

  //----------------------------------------------------------------------------

  ERR MatchMngr::undoMatchCall(const Match& ma) const
  {
    if (ma.getState() != ObjState::MA_RUNNING)
    {
      return ERR::MATCH_NOT_RUNNING;
    }

    // release the players first, because we need the entries
    // in MA_ACTUAL_PLAYER1A_REF etc.
    PlayerMngr pm{db};
    pm.releasePlayerPairsAfterMatch(ma);

    // release the umpire, if any
    auto referee = ma.getAssignedReferee();
    if (referee  && (referee->getState() == ObjState::PL_REFEREE))
    {
      referee->setState(ObjState::PL_IDLE);
    }

    // store the court the match is running on
    ERR e;
    auto court = ma.getCourt(&e);
    assert(e == ERR::OK);

    // reset the references to the court and the actual players
    ColumnValueClause cvc;
    cvc.addNullCol(MA_ACTUAL_PLAYER1A_REF);
    cvc.addNullCol(MA_ACTUAL_PLAYER1B_REF);
    cvc.addNullCol(MA_ACTUAL_PLAYER2A_REF);
    cvc.addNullCol(MA_ACTUAL_PLAYER2B_REF);
    cvc.addNullCol(MA_COURT_REF);

    // set the state back to READY
    cvc.addCol(GENERIC_STATE_FIELD_NAME, static_cast<int>(ObjState::MA_READY));

    // apply all changes at once
    ma.row.update(cvc);
    CentralSignalEmitter::getInstance()->matchStatusChanged(ma.getId(), ma.getSeqNum(), ObjState::MA_RUNNING, ObjState::MA_READY);

    // release the court
    CourtMngr cm{db};
    bool isOkay = cm.releaseCourt(*court);
    assert(isOkay);

    // update the match group
    updateAllMatchGroupStates(ma.getCategory());

    // update the category's state
    CatMngr catm{db};
    catm.updateCatStatusFromMatchStatus(ma.getCategory());

    // erase start time from database
    ma.row.updateToNull(MA_START_TIME);
    ma.row.updateToNull(MA_ADDITIONAL_CALL_TIMES);

    // check all matches that are currently "BUSY" because
    // due to the player release, some of them might have
    // become "READY"
    for (const Match& otherMatch : getObjectsByColumnValue<Match>(GENERIC_STATE_FIELD_NAME, static_cast<int>(ObjState::MA_BUSY)))
    {
      updateMatchStatus(otherMatch);
    }

    return ERR::OK;
  }

  //----------------------------------------------------------------------------

  std::optional<Match> MatchMngr::getMatchForCourt(const Court& court)
  {
    // search for matches in state RUNNING and assigned to the court
    WhereClause wc;
    wc.addCol(MA_COURT_REF, court.getId());
    wc.addCol(GENERIC_STATE_FIELD_NAME, static_cast<int>(ObjState::MA_RUNNING));

    return getSingleObjectByWhereClause<Match>(wc);
  }

  //----------------------------------------------------------------------------

  std::optional<Match> MatchMngr::getMatch(int id) const
  {
    return getSingleObjectByColumnValue<Match>("id", id);
  }

  //----------------------------------------------------------------------------

  std::tuple<int, int, int, int> MatchMngr::getMatchStats() const
  {
    // get the total number of matches
    int nTotal = tab.length();

    // get the number of currently running matches
    int nRunning = tab.getMatchCountForColumnValue(GENERIC_STATE_FIELD_NAME, static_cast<int>(ObjState::MA_RUNNING));

    // get the number of finished matches
    int nFinished = tab.getMatchCountForColumnValue(GENERIC_STATE_FIELD_NAME, static_cast<int>(ObjState::MA_FINISHED));

    // get the number of scheduled matches
    WhereClause wc;
    wc.addCol(MA_NUM, ">", 0);
    int nScheduled = tab.getMatchCountForWhereClause(wc) - nRunning - nFinished;

    return std::tuple{nTotal, nScheduled, nRunning, nFinished};
  }

  //----------------------------------------------------------------------------

  void MatchMngr::onPlayerStatusChanged(int playerId, int playerSeqNum, ObjState fromState, ObjState toState)
  {
    // IMPORTANT:
    // This is the only place, where a transition from/to BUSY is managed
    //
    // IMPORTANT:
    // We only transition from READY to BUSY, never from any other state.
    // Otherwise we wouldn't know to which state we should return to, after the
    // BUSY-condition is no longer applicable. Read: the only transition
    // back from BUSY is to READY!!

    /*
     * 2019-08-19: FIX: is this signal handler really necessary?
     * Doesn't it break with the overall architecture? Is it a
     * left-over from the times when we had the xManagers as global singletons?
     */

    CentralSignalEmitter* cse = CentralSignalEmitter::getInstance();

    // set matches that are READY to BUSY, if the necessary players become unavailable
    if (toState == ObjState::PL_PLAYING)
    {
      for (const Match& ma : getObjectsByColumnValue<Match>(GENERIC_STATE_FIELD_NAME, static_cast<int>(ObjState::MA_READY)))
      {
        const PlayerList pl = ma.determineActualPlayers();
        for (const Player& p : pl)
        {
          if (p.getId() == playerId)
          {
            ma.row.update(GENERIC_STATE_FIELD_NAME, static_cast<int>(ObjState::MA_BUSY));
            cse->matchStatusChanged(ma.getId(), ma.getSeqNum(), ObjState::MA_READY, ObjState::MA_BUSY);
            break;  // no need to check other players for this match
          }
        }
      }
    }

    // switch matches from BUSY to READY, if all players are available again
    if (toState == ObjState::PL_IDLE)
    {
      PlayerMngr pm{db};
      for (Match ma : getObjectsByColumnValue<Match>(GENERIC_STATE_FIELD_NAME, static_cast<int>(ObjState::MA_BUSY)))
      {
        if (pm.canAcquirePlayerPairsForMatch(ma) == ERR::OK)
        {
          ma.row.update(GENERIC_STATE_FIELD_NAME, static_cast<int>(ObjState::MA_READY));
          cse->matchStatusChanged(ma.getId(), ma.getSeqNum(), ObjState::MA_BUSY, ObjState::MA_READY);
        }
      }
    }
  }

  //----------------------------------------------------------------------------

  MatchList MatchMngr::getCurrentlyRunningMatches() const
  {
    return getObjectsByColumnValue<Match>(GENERIC_STATE_FIELD_NAME, static_cast<int>(ObjState::MA_RUNNING));
  }

  //----------------------------------------------------------------------------

  MatchList MatchMngr::getFinishedMatches() const
  {
    return getObjectsByColumnValue<Match>(GENERIC_STATE_FIELD_NAME, static_cast<int>(ObjState::MA_FINISHED));
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
    /*
    // do we have match groups in this category at all?
    int grpCount = groupTab.getMatchCountForColumnValue(MG_CAT_REF, cat.getId());
    if (grpCount == 0) return 0;
    */

    // query the highest used round number
    Sloppy::estring sql = "SELECT max(%1) FROM %2 WHERE %3 = %4";
    sql.arg(MG_ROUND);
    sql.arg(TAB_MATCH_GROUP);
    sql.arg(MG_CAT_REF);
    sql.arg(cat.getId());

    try
    {
      return db.get().execScalarQueryIntOrNull(sql).value_or(0);
    }
    catch (NoDataException&)
    {
      return 0;
    }
  }

  //----------------------------------------------------------------------------

  std::optional<Match> MatchMngr::getMatchForPlayerPairAndRound(const PlayerPair &pp, int round) const
  {
    auto cat = pp.getCategory(db);
    if (!cat) return {};

    for (MatchGroup mg : getMatchGroupsForCat(*cat, round))
    {
      Sloppy::estring where = "(%1 = %2 OR %3 = %2) AND %4 = %5";
      where.arg(MA_PAIR1_REF);
      where.arg(pp.getPairId());
      where.arg(MA_PAIR2_REF);
      where.arg(MA_GRP_REF);
      where.arg(mg.getId());

      auto result = getSingleObjectByWhereClause<Match>(where);
      if (result) return result;
    }
    return {};
  }

  //----------------------------------------------------------------------------

  void MatchMngr::resolveSymbolicNamesAfterFinishedMatch(const Match &ma) const
  {
    if (ma.getState() != ObjState::MA_FINISHED)
    {
      return;
    }

    int matchId = ma.getId();
    auto winnerPair = ma.getWinner();
    auto loserPair = ma.getLoser();

    static const std::vector<std::tuple<std::string, std::string>> colPairs{
      {MA_PAIR1_SYMBOLIC_VAL, MA_PAIR1_REF},
      {MA_PAIR2_SYMBOLIC_VAL, MA_PAIR2_REF},
    };
    std::vector<std::tuple<int, int>> symbolicValue2PairId;
    if (winnerPair) symbolicValue2PairId.push_back(std::tuple{matchId, winnerPair->getPairId()});
    if (loserPair) symbolicValue2PairId.push_back(std::tuple{-matchId, loserPair->getPairId()});

    CentralSignalEmitter* cse = CentralSignalEmitter::getInstance();

    for (const auto& [symbolColName, pairRefColName] : colPairs)
    {
      for (const auto& [symbolicValue, pairId] : symbolicValue2PairId)
      {
        for (const Match& m : getObjectsByColumnValue<Match>(symbolColName, symbolicValue))
        {
          // we may only modify matches in state FUZZY or INCOMPLETE
          ObjState stat = m.getState();
          if ((stat != ObjState::MA_FUZZY) && (stat != ObjState::MA_INCOMPLETE)) continue;

          m.row.update(pairRefColName, pairId);  // set the reference to the winner / loser
          m.row.update(symbolColName, 0);   // delete symbolic reference

          // emit a faked state change to trigger a display update of the
          // match in the match tab view
          cse->matchStatusChanged(m.getId(), m.getSeqNum(), stat, stat);
        }
      }
    }

    // if we resolved all symbolic references of a match, it may be promoted from
    // FUZZY at least to WAITING, maybe even to READY or BUSY
    WhereClause wc;
    wc.addCol(GENERIC_STATE_FIELD_NAME, static_cast<int>(ObjState::MA_FUZZY));
    wc.addCol(MA_PAIR1_SYMBOLIC_VAL, 0);
    wc.addCol(MA_PAIR2_SYMBOLIC_VAL, 0);
    wc.addCol(MA_PAIR1_REF, ">", 0);
    wc.addCol(MA_PAIR2_REF, ">", 0);
    for (Match m : getObjectsByWhereClause<Match>(wc))
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
