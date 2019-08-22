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
    : TournamentDatabaseObjectManager(_db, TabMatch), groupTab{db, TabMatchGroup, false}
  {
  }

  //----------------------------------------------------------------------------

  MatchGroupOrError MatchMngr::createMatchGroup(const Category& cat, const int round, const int grpNum)
  {
    // we can only create match groups, if the category configuration is stable
    // this means, we may not be in ObjState::CAT_Config or _FROZEN
    ObjState catState = cat.getState();
    if ((catState == ObjState::CAT_Config) || (catState == ObjState::CAT_Frozen))
    {
      return MatchGroupOrError{Error::CategoryStillConfigurable};
    }

    // check parameters for validity
    if (grpNum <= 0)
    {
      if ((grpNum != GroupNum_Final)
          && (grpNum != GroupNum_Semi)
          && (grpNum != GroupNum_Quarter)
          && (grpNum != GroupNum_L16)
          && (grpNum != GroupNum_Iteration))
      {
        return MatchGroupOrError{Error::InvalidGroupNum};
      }
    }

    if (round <= 0)
    {
      return MatchGroupOrError{Error::InvalidRound};
    }

    // ensure that we don't mix "normal" group numbers with "special" group numbers
    if (grpNum <= 0)
    {
      // in this case, no other match groups in this round may exist, i. e.
      // there can only be one match group for semi finals
      const auto otherGroups = getMatchGroupsForCat(cat, round);
      if (!otherGroups.empty())
      {
        return MatchGroupOrError{Error::InvalidGroupNum};
      }
    }
    if (grpNum > 0)
    {
      // in this case, no other match groups with group numbers
      // below zero (e. g. semi finals) may exist
      WhereClause wc;
      wc.addCol(MG_GrpNum, "<=", 0);
      wc.addCol(MG_Round, round);
      wc.addCol(MG_CatRef, cat.getId());
      int nOtherGroups = groupTab.getMatchCountForWhereClause(wc);
      if (nOtherGroups != 0)
      {
        return MatchGroupOrError{Error::InvalidGroupNum};
      }
    }

    // make sure the match group doesn't already exist
    auto mg = getMatchGroup(cat, round, grpNum);
    if (mg.err(Error::OK))    // match group exists
    {
      return MatchGroupOrError{Error::MatchGroupExists};
    }
    if (mg.err() != Error::NoSuchMatchGroup)   // catch any other error except "no such group"
    {
      return MatchGroupOrError{mg.err()};
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
    cvc.addCol(MG_CatRef, cat.getId());
    cvc.addCol(MG_Round, round);
    cvc.addCol(MG_GrpNum, grpNum);
    cvc.addCol(GenericStateFieldName, static_cast<int>(ObjState::MG_Config));
    cvc.addCol(GenericSeqnumFieldName, InvalidInitialSequenceNumber);  // will be fixed immediately; this is just for satisfying a not-NULL constraint

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
    wc.addCol(MG_CatRef, cat.getId());
    if (round > 0)
    {
      wc.addCol(MG_Round, round);
    }

    return SqliteOverlay::getObjectsByWhereClause<MatchGroup>(db, groupTab, wc);
  }
  
  //----------------------------------------------------------------------------

  MatchGroupList MatchMngr::getAllMatchGroups() const
  {
    return SqliteOverlay::getAllObjects<MatchGroup>(db, groupTab);
  }

  //----------------------------------------------------------------------------

  MatchGroupOrError MatchMngr::getMatchGroup(const Category& cat, const int round, const int grpNum)
  {
    // check round parameter for validity
    if (round <= 0)
    {
      return MatchGroupOrError{Error::InvalidRound};
    }
    
    // check group parameter for validity
    if (grpNum <= 0)
    {
      if ((grpNum != GroupNum_Final)
          && (grpNum != GroupNum_Semi)
          && (grpNum != GroupNum_Quarter)
          && (grpNum != GroupNum_L16)
          && (grpNum != GroupNum_Iteration))
      {
        return MatchGroupOrError{Error::InvalidGroupNum};
      }
    }
    
    // search for the match group in the database
    WhereClause wc;
    wc.addCol(MG_CatRef, cat.getId());
    wc.addCol(MG_Round, round);
    wc.addCol(MG_GrpNum, grpNum); 
    auto r = groupTab.getSingleRowByWhereClause2(wc);

    if (!r)
    {
      return MatchGroupOrError{Error::NoSuchMatchGroup};
    }

    return MatchGroupOrError{db, *r};
  }

  //----------------------------------------------------------------------------

  /*
  bool MatchMngr::hasMatchGroup(const Category& cat, const int round, const int grpNum, Error* err)
  {
    Error e;
    auto mg = getMatchGroup(cat, round, grpNum, &e);
    Sloppy::assignIfNotNull<Error>(err, e);
    return mg.has_value();
  }
  */

  //----------------------------------------------------------------------------

  MatchOrError MatchMngr::createMatch(const MatchGroup &grp)
  {
    // we can only add matches to a group if the group
    // is in the config state
    if (grp.is_NOT_InState(ObjState::MG_Config))
    {
      return MatchOrError{Error::MatchGroupNotConfiguraleAnymore};
    }

    // Okay, parameters are valid
    // create a new match entry in the database
    ColumnValueClause cvc;
    cvc.addCol(MA_GrpRef, grp.getId());
    cvc.addCol(GenericStateFieldName, static_cast<int>(ObjState::MA_Incomplete));
    cvc.addCol(MA_Pair1SymbolicVal, 0);   // default: no symbolic name
    cvc.addCol(MA_Pair2SymbolicVal, 0);   // default: no symbolic name
    cvc.addCol(MA_WinnerRank, -1);         // default: no rank, no knock out
    cvc.addCol(MA_LoserRank, -1);         // default: no rank, no knock out
    cvc.addCol(MA_RefereeMode, -1);        // -1: use current tournament default
    cvc.addCol(GenericSeqnumFieldName, InvalidInitialSequenceNumber);  // will be fixed immediately; this is just for satisfying a not-NULL constraint

    CentralSignalEmitter* cse = CentralSignalEmitter::getInstance();
    cse->beginCreateMatch();
    int newId = tab.insertRow(cvc);
    fixSeqNumberAfterInsert();
    cse->endCreateMatch(tab.length() - 1); // the new sequence number is always the highest

    // return a match object for the new match
    return MatchOrError{db, newId};
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
    // MAKE SURE THAT INCOMING LINKS TO MATCH GROUP FROM THE MatchSystem::Ranking TAB
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

  Error MatchMngr::setPlayerPairsForMatch(const Match &ma, const PlayerPair &pp1, const PlayerPair &pp2)
  {
    //
    // Initial note: we do not solve this function here by simply calling
    // setPlayerPairForMatch() twice. Because if we would do so, we could
    // leave the match in an invalid state if pp1 is valid (and thus assigned
    // to the match by the first call) and pp2 is not (and thus playerpair2 is
    // not set).
    //


    // make sure the player pair is valid (has a database entry)
    if (pp1.getPairId() < 1) return Error::InvalidPlayerPair;
    if (pp2.getPairId() < 1) return Error::InvalidPlayerPair;

    // check if an assignment of the player pairs is okay
    Error e = canAssignPlayerPairToMatch(ma, pp1);
    if (e != Error::OK) return e;
    e = canAssignPlayerPairToMatch(ma, pp2);
    if (e != Error::OK) return e;

    // make sure that both pairs are not identical
    if (pp1.getPairId() == pp2.getPairId())
    {
      return Error::PlayersIdentical;
    }

    // assign the player pairs
    ma.rowRef().update(MA_Pair1Ref, pp1.getPairId());
    ma.rowRef().update(MA_Pair2Ref, pp2.getPairId());

    // potentially, the player pairs where all that was necessary
    // to actually promote the match to e.g., WAITING
    updateMatchStatus(ma);

    return Error::OK;
  }

  //----------------------------------------------------------------------------

  Error MatchMngr::setPlayerPairForMatch(const Match& ma, const PlayerPair& pp, int ppPos) const
  {
    // make sure the player pair is valid (has a database entry)
    if (pp.getPairId() < 1) return Error::InvalidPlayerPair;

    // check if an assignment of the player pairs is okay
    Error e = canAssignPlayerPairToMatch(ma, pp);
    if (e != Error::OK) return e;

    // assign the player pair
    if (ppPos == 1) ma.rowRef().update(MA_Pair1Ref, pp.getPairId());
    if (ppPos == 2) ma.rowRef().update(MA_Pair2Ref, pp.getPairId());

    return Error::OK;
  }

  //----------------------------------------------------------------------------

  Error MatchMngr::setSymbolicPlayerForMatch(const Match& fromMatch, const Match& toMatch, bool asWinner, int dstPlayerPosInMatch) const
  {
    // Only allow changing / setting players if we not yet fully configured
    if (toMatch.is_NOT_InState(ObjState::MA_Incomplete)) return Error::MatchNotConfiguraleAnymore;

    // fromMatch and toMatch must be in the same category
    int fromMatchCatId = fromMatch.getCategory().getId();
    int toMatchCatId = toMatch.getCategory().getId();
    if (fromMatchCatId != toMatchCatId)
    {
      return Error::InvalidMatchLink;
    }

    // toMatch must be in a later round than fromMatch
    MatchGroup fromGroup = fromMatch.getMatchGroup();
    MatchGroup toGroup = toMatch.getMatchGroup();
    if (toGroup.getRound() <= fromGroup.getRound())
    {
      return Error::InvalidMatchLink;
    }

    // okay, the link is valid

    int dstId = asWinner ? fromMatch.getId() : -(fromMatch.getId());
    if (dstPlayerPosInMatch == 1)
    {
      toMatch.rowRef().update(MA_Pair1SymbolicVal, dstId);
      toMatch.rowRef().updateToNull(MA_Pair1Ref);
    }
    if (dstPlayerPosInMatch == 2)
    {
      toMatch.rowRef().update(MA_Pair2SymbolicVal, dstId);
      toMatch.rowRef().updateToNull(MA_Pair2Ref);
    }

    return Error::OK;
  }

  //----------------------------------------------------------------------------

  Error MatchMngr::setPlayerToUnused(const Match& ma, int unusedPlayerPos, int winnerRank) const
  {
    // Only allow changing / setting player pairs if we not yet fully configured
    if (ma.is_NOT_InState(ObjState::MA_Incomplete)) return Error::MatchNotConfiguraleAnymore;

    if (unusedPlayerPos == 1)
    {
      ma.rowRef().updateToNull(MA_Pair1Ref);
      ma.rowRef().update(MA_Pair1SymbolicVal, SymbolicIdForUnusedPlayerPairInMatch);
    }
    if (unusedPlayerPos == 2)
    {
      ma.rowRef().updateToNull(MA_Pair2Ref);
      ma.rowRef().update(MA_Pair2SymbolicVal, SymbolicIdForUnusedPlayerPairInMatch);
    }
    ma.rowRef().update(MA_WinnerRank, winnerRank);

    return Error::OK;
  }

  //----------------------------------------------------------------------------

  Error MatchMngr::setRankForWinnerOrLoser(const Match& ma, bool isWinner, int rank) const
  {
    // Only allow changing / setting match data if we not yet fully configured
    if (ma.is_NOT_InState(ObjState::MA_Incomplete)) return Error::MatchNotConfiguraleAnymore;

    // TODO: check if rank is really valid

    if (isWinner)
    {
      ma.rowRef().update(MA_WinnerRank, rank);
    } else {
      ma.rowRef().update(MA_LoserRank, rank);
    }

    return Error::OK;
  }

  //----------------------------------------------------------------------------

  Error MatchMngr::canAssignPlayerPairToMatch(const Match &ma, const PlayerPair &pp) const
  {
    // Only allow changing / setting player pairs if we not yet fully configured
    ObjState stat = ma.getState();
    if ((stat != ObjState::MA_Incomplete) && (stat != ObjState::MA_Fuzzy)) return Error::MatchNotConfiguraleAnymore;

    // make sure the player pair and the match belong to the same category
    Category requiredCat = ma.getCategory();
    auto ppCat = pp.getCategory(db);
    if (*ppCat != requiredCat) return Error::PlayerNotInCategory;

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
      return Error::GroupNumberMismatch;
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
          if (otherPairId == pp.getPairId()) return Error::PlayerAlreadyAssignedToOtherMatchInTheSameRoundAndCategory;
        }

        // check the second  player pair of this match, if existing
        if (otherMatch.hasPlayerPair2())
        {
          int otherPairId = otherMatch.getPlayerPair2().getPairId();
          if (otherPairId == pp.getPairId()) return Error::PlayerAlreadyAssignedToOtherMatchInTheSameRoundAndCategory;
        }
      }
    }

    return Error::OK;
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
  Error MatchMngr::closeMatchGroup(const MatchGroup &grp)
  {
    // we can only close match groups that are in state CONFIG
    if (grp.is_NOT_InState(ObjState::MG_Config)) return Error::MatchGroupAlreadyClosed;

    // the match group should contain at least one match
    if (grp.getMatchCount() < 1) return Error::MatchGroupEmpty;

    // we can close the group unconditionally
    grp.setState(ObjState::MG_Frozen);
    CentralSignalEmitter::getInstance()->matchGroupStatusChanged(grp.getId(), grp.getSeqNum(), ObjState::MG_Config, ObjState::MG_Frozen);

    // call updateAllMatchGroupStates in case the group can be further promoted
    // to idle (which enables the group the be scheduled)
    updateAllMatchGroupStates(grp.getCategory());

    return Error::OK;
  }

  //----------------------------------------------------------------------------

  Error MatchMngr::setRefereeMode(const Match& ma, RefereeMode newMode) const
  {
    // get the old mode
    RefereeMode oldMode = ma.get_RAW_RefereeMode();

    // is there anything to do at all?
    if (oldMode == newMode) return Error::OK;

    // only allow changes to the referee mode before the match has been called
    ObjState stat = ma.getState();
    if ((stat == ObjState::MA_Running) || (stat == ObjState::MA_Finished))
    {
      return Error::MatchNotConfiguraleAnymore;
    }

    // set the new mode
    ma.rowRef().update(MA_RefereeMode, static_cast<int>(newMode));

    // if we go to a more restrictive mode, delete any existing
    // referee assignments
    if ((ma.hasRefereeAssigned()) && (newMode != RefereeMode::AllPlayers))
    {
      ma.rowRef().updateToNull(MA_RefereeRef);
    }

    // fake a match-changed-event in order to trigger UI updates
    CentralSignalEmitter::getInstance()->matchStatusChanged(ma.getId(), ma.getSeqNum(), stat, stat);

    return Error::OK;
  }

  //----------------------------------------------------------------------------

  Error MatchMngr::assignReferee(const Match& ma, const Player& p, RefereeAction refAction) const
  {
    Error e = ma.canAssignReferee(refAction);
    if (e != Error::OK) return e;

    // don't allow assignments of players that actually
    // take part in the match
    PlayerPair pp1 = ma.getPlayerPair1();
    PlayerPair pp2 = ma.getPlayerPair2();
    if (pp1.getPlayer1() == p)
    {
      return Error::PlayerNotSuitable;
    }
    if (pp1.hasPlayer2() && (pp1.getPlayer2() == p))
    {
      return Error::PlayerNotSuitable;
    }
    if (pp2.getPlayer1() == p)
    {
      return Error::PlayerNotSuitable;
    }
    if (pp2.hasPlayer2() && (pp2.getPlayer2() == p))
    {
      return Error::PlayerNotSuitable;
    }

    // ensure that the player is IDLE when calling a match or
    // swapping the umpire
    if ((refAction != RefereeAction::PreAssign) && (p.is_NOT_InState(ObjState::PL_Idle)))
    {
      return Error::PlayerNotSuitable;
    }

    // store the currently assigned referee
    auto currentReferee = ma.getAssignedReferee();

    // okay, it is safe to assign the referee

    ma.rowRef().update(MA_RefereeRef, p.getId());

    // if we're swapping the umpire, we have to update the player states as well
    //
    // Note: when calling the match, the player states are updated by assignMatchToCourt()
    CentralSignalEmitter* cse = CentralSignalEmitter::getInstance();
    if (refAction == RefereeAction::Swap)
    {
      assert(currentReferee.has_value());
      currentReferee->setState(ObjState::PL_Idle);
      cse->playerStatusChanged(currentReferee->getId(), currentReferee->getSeqNum(), ObjState::PL_Referee, ObjState::PL_Idle);

      p.setState(ObjState::PL_Referee);
      cse->playerStatusChanged(p.getId(), p.getSeqNum(), ObjState::PL_Idle, ObjState::PL_Referee);
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
      std::string{GenericStateFieldName} + "=" + std::to_string(static_cast<int>(ObjState::MA_Ready)) +
      " OR " +
      std::string{GenericStateFieldName} + "=" + std::to_string(static_cast<int>(ObjState::MA_Busy))
    };
    for (const Match& otherMatch : getObjectsByWhereClause<Match>(where))
    {
      ObjState otherStat = otherMatch.getState();

      // upon match call, other matches can only switch from READY to BUSY
      // because we're allocating players
      if ((refAction == RefereeAction::MatchCall) && (otherStat == ObjState::MA_Ready))
      {
        updateMatchStatus(otherMatch);
      }

      // upon umpire swap, other matches can switch from READY to BUSY or from BUSY to READY
      // because we're allocating the new umpire and release the old umpire
      if ((refAction == RefereeAction::Swap) && ((otherStat == ObjState::MA_Ready) || (otherStat == ObjState::MA_Busy)))
      {
        updateMatchStatus(otherMatch);
      }
    }

    return Error::OK;
  }

  //----------------------------------------------------------------------------

  Error MatchMngr::removeReferee(const Match& ma) const
  {
    ObjState stat = ma.getState();
    if ((stat == ObjState::MA_Running) || (stat == ObjState::MA_Finished))
    {
      return Error::MatchNotConfiguraleAnymore;
    }

    ma.rowRef().updateToNull(MA_RefereeRef);

    // maybe the match status changes after the removal, because we're not
    // waiting anymore for a busy referee to become available
    updateMatchStatus(ma);

    // fake a match-changed-event in order to trigger UI updates
    CentralSignalEmitter::getInstance()->matchStatusChanged(ma.getId(), ma.getSeqNum(), stat, stat);

    return Error::OK;
  }

  //----------------------------------------------------------------------------

  Error MatchMngr::swapPlayer(const Match& ma, const PlayerPair& ppOld, const PlayerPair& ppNew) const
  {
    // the matches may not be "running" or "finished"
    ObjState stat = ma.getState();
    if ((stat == ObjState::MA_Running) || (stat == ObjState::MA_Finished)) return Error::WrongState;

    // the new player pair must belong to the
    // same category as the old one
    auto oldPairCat = ppOld.getCategory(db);
    if (!oldPairCat) return Error::InvalidPlayerPair;
    auto newPairCat = ppNew.getCategory(db);
    if (!newPairCat) return Error::InvalidPlayerPair;
    if (oldPairCat->getId() != newPairCat->getId()) return Error::InvalidPlayerPair;

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
      return Error::InvalidPlayerPair;   // ppOld is not part of the match
    }

    // if both pairs are identical, we're done
    if (ppOld.getPairId() == ppNew.getPairId()) return Error::OK;

    // start a transaction
    try
    {
      auto trans = db.startTransaction();

      // actually swap the players
      if (ppPos == 1) ma.rowRef().update(MA_Pair1Ref, ppNew.getPairId());
      if (ppPos == 2) ma.rowRef().update(MA_Pair2Ref, ppNew.getPairId());

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
      return Error::OK;
    }
    catch (SqliteOverlay::BusyException&)
    {
      return Error::DatabaseError;
    }
    catch (SqliteOverlay::GenericSqliteException&)
    {
      return Error::DatabaseError;
    }
  }

  //----------------------------------------------------------------------------

  Error MatchMngr::swapPlayers(const Match& ma1, const PlayerPair& ma1PlayerPair, const Match& ma2, const PlayerPair& ma2PlayerPair) const
  {
    // the matches may not be "running" or "finished"
    for (const Match& m : {ma1, ma2})
    {
      ObjState stat = m.getState();
      if ((stat == ObjState::MA_Running) || (stat == ObjState::MA_Finished)) return Error::WrongState;
    }

    // the matches must belong to the same category
    if (ma1.getCategory() != ma2.getCategory())
    {
      return Error::InvalidId;  // not the best matching error code, but anyway...
    }

    try
    {
      auto trans = db.startTransaction();

      // swap the players
      Error e = swapPlayer(ma1, ma1PlayerPair, ma2PlayerPair);
      if (e != Error::OK)
      {
        return e;  // triggers implicit rollback
      }

      e = swapPlayer(ma2, ma2PlayerPair, ma1PlayerPair);
      if (e != Error::OK)
      {
        return e;  // triggers implicit rollback
      }

      trans.commit();

      return Error::OK;
    }
    catch (SqliteOverlay::BusyException&)
    {
      return Error::DatabaseError;
    }
    catch (SqliteOverlay::GenericSqliteException&)
    {
      return Error::DatabaseError;
    }
  }

  //----------------------------------------------------------------------------

  std::string MatchMngr::getSyncString(const std::vector<int>& rows) const
  {
    std::vector<Sloppy::estring> cols = {"id", GenericStateFieldName, MA_GrpRef, MA_Num, MA_Pair1Ref, MA_Pair2Ref,
                          MA_ActualPlayer1aRef, MA_ActualPlayer1bRef, MA_ActualPlayer2aRef, MA_ActualPlayer2bRef,
                          MA_Result, MA_CourtRef, MA_StartTime, MA_AdditionalCallTimes, MA_FinishTime,
                           MA_Pair1SymbolicVal, MA_Pair2SymbolicVal, MA_WinnerRank, MA_LoserRank,
                           MA_RefereeMode, MA_RefereeRef};

    return db.getSyncStringForTable(TabMatch, cols, rows);
  }

  //----------------------------------------------------------------------------

  std::string MatchMngr::getSyncString_MatchGroups(std::vector<int> rows)
  {
    std::vector<Sloppy::estring> cols = {"id", MG_CatRef, GenericStateFieldName, MG_Round, MG_GrpNum};

    return db.getSyncStringForTable(TabMatchGroup, cols, rows);
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
    wc.addCol(MG_CatRef, cat.getId());
    wc.addCol(GenericStateFieldName, static_cast<int>(ObjState::MG_Scheduled));
    for (TabRowIterator it{db, TabMatchGroup, wc}; it.hasData(); ++it)
    {
      const MatchGroup mg{db, *it};

      // check all matches in this scheduled category
      bool isfinished = true;
      for (auto match : mg.getMatches())
      {
        if (match.is_NOT_InState(ObjState::MA_Finished))
        {
          isfinished = false;
          break;
        }
      }
      if (isfinished)
      {
        mg.setState(ObjState::MG_Finished);
        cse->matchGroupStatusChanged(mg.getId(), mg.getSeqNum(), ObjState::MG_Scheduled, ObjState::MG_Finished);
      }
    }

    // transition from FROZEN to IDLE
    wc.clear();
    wc.addCol(MG_CatRef, cat.getId());
    wc.addCol(GenericStateFieldName, static_cast<int>(ObjState::MG_Frozen));
    for (TabRowIterator it{db, TabMatchGroup, wc}; it.hasData(); ++it)
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
          if (!((mg2Stat == ObjState::MG_Staged) || (mg2Stat == ObjState::MG_Scheduled) || (mg2Stat == ObjState::MG_Finished)))
          {
            canPromote = false;
            break;
          }
        }

        if (!canPromote) break;  // no need to loop through other rounds
      }

      if (canPromote)
      {
        mg.setState(ObjState::MG_Idle);
        cse->matchGroupStatusChanged(mg.getId(), mg.getSeqNum(), ObjState::MG_Frozen, ObjState::MG_Idle);
      }
    }

  }

  //----------------------------------------------------------------------------

  std::optional<MatchGroup> MatchMngr::getMatchGroupBySeqNum(int mgSeqNum)
  {
    return SqliteOverlay::getSingleObjectByColumnValue<MatchGroup>(db, groupTab, GenericSeqnumFieldName, mgSeqNum);
  }

  //----------------------------------------------------------------------------

  /**
    Try to promote the match group from IDLE to STAGED.

    Also call updateAllMatchGroups() in case other match group can be
    subsequently promoted from FROZEN to IDLE

    \param grp is the match group to stage

    \return error code
    */
  Error MatchMngr::stageMatchGroup(const MatchGroup &grp)
  {
    Error e = canStageMatchGroup(grp);
    if (e != Error::OK) return e;

    // no further checks necessary. Any IDLE match group can be promoted

    // promote the group to STAGED and assign a sequence number
    int nextStageSeqNum = getMaxStageSeqNum() + 1;
    grp.setState(ObjState::MG_Staged);
    grp.rowRef().update(MG_StageSeqNum, nextStageSeqNum);
    CentralSignalEmitter::getInstance()->matchGroupStatusChanged(grp.getId(), grp.getSeqNum(), ObjState::MG_Idle, ObjState::MG_Staged);

    // promote other groups from FROZEN to IDLE, if applicable
    updateAllMatchGroupStates(grp.getCategory());

    return Error::OK;
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
    int statId = static_cast<int>(ObjState::MG_Staged);
    if (groupTab.getMatchCountForColumnValue(GenericStateFieldName, statId) < 1)
    {
      return 0;  // no staged match groups so far
    }

    // determine the max sequence number
    Sloppy::estring sql{"SELECT max(%1) FROM %2"};
    sql.arg(MG_StageSeqNum);
    sql.arg(TabMatchGroup);
    try
    {
      return db.execScalarQueryInt(sql);
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
  Error MatchMngr::canUnstageMatchGroup(const MatchGroup &grp)
  {
    // first precondition: match group has to be staged
    if (grp.is_NOT_InState(ObjState::MG_Staged))
    {
      return Error::MatchGroupNotUnstageable;
    }

    int round = grp.getRound();
    int playersGroup = grp.getGroupNumber();
    int catId = grp.getCategory().getId();

    // check for a match group with higher round number in the staging area
    WhereClause wc;
    wc.addCol(MG_Round, round + 1);
    wc.addCol(GenericStateFieldName, static_cast<int>(ObjState::MG_Staged));
    wc.addCol(MG_CatRef, catId);
    if (groupTab.getMatchCountForWhereClause(wc) == 0)
    {
      // there is no match group of this category and with a higher
      // round number in the staging area
      // ==> the match group can be demoted
      return Error::OK;
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
      return Error::MatchGroupNotUnstageable;
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
    for (MatchGroup mg : SqliteOverlay::getObjectsByWhereClause<MatchGroup>(db, groupTab, wc))
    {
      int nextGroupNumber = mg.getGroupNumber();
      if (nextGroupNumber < 0)
      {
        // next round is already in KO phase
        //
        // 2019-08-19: FIX, is that correct or too strict?
        return Error::MatchGroupNotUnstageable;
      }

      // the next round's group belongs to the same
      // players group ==> can't unstage
      if (nextGroupNumber == playersGroup)
      {
        return Error::MatchGroupNotUnstageable;
      }
    }

    return Error::OK;
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
  Error MatchMngr::unstageMatchGroup(const MatchGroup &grp)
  {
    // check all preconditions for a demotion
    Error e = canUnstageMatchGroup(grp);
    if (e != Error::OK) return e;

    // great, we can safely demote this match group to IDLE
    grp.setState(ObjState::MG_Idle);

    // store and delete old stage sequence number
    int oldStageSeqNumber = grp.getStageSequenceNumber();
    grp.rowRef().updateToNull(MG_StageSeqNum);

    CentralSignalEmitter* cse = CentralSignalEmitter::getInstance();

    cse->matchGroupStatusChanged(grp.getId(), grp.getSeqNum(), ObjState::MG_Staged, ObjState::MG_Idle);

    // update all subsequent sequence numbers
    WhereClause wc;
    wc.addCol(MG_StageSeqNum, ">", oldStageSeqNumber);
    for (const auto& mg : SqliteOverlay::getObjectsByWhereClause<MatchGroup>(db, groupTab, wc))
    {
      int old = mg.getStageSequenceNumber();
      mg.rowRef().update(MG_StageSeqNum, old - 1);
      cse->matchGroupStatusChanged(mg.getId(), mg.getSeqNum(), ObjState::MG_Staged, ObjState::MG_Staged);
    }

    // demote other groups from IDLE to FROZEN
    // check for a match group with "round + 1" in state IDLE
    int round = grp.getRound();
    int playersGroup = grp.getGroupNumber();
    int catId = grp.getCategory().getId();

    wc.clear();
    wc.addCol(MG_Round, round+1);
    wc.addCol(GenericStateFieldName, static_cast<int>(ObjState::MG_Idle));
    wc.addCol(MG_CatRef, catId);
    for (MatchGroup mg : SqliteOverlay::getObjectsByWhereClause<MatchGroup>(db, groupTab, wc))
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
      mg.setState(ObjState::MG_Frozen);
      cse->matchGroupStatusChanged(mg.getId(), mg.getSeqNum(), ObjState::MG_Idle, ObjState::MG_Frozen);
    }

    return Error::OK;
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
  Error MatchMngr::canStageMatchGroup(const MatchGroup &grp)
  {
    return (grp.isInState(ObjState::MG_Idle)) ? Error::OK : Error::WrongState;
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
    if (curState == ObjState::MA_Incomplete)
    {
      if (ma.hasBothPlayerPairs() && (ma.getMatchNumber() != MatchNumNotAssigned))
      {
        ma.setState(ObjState::MA_Waiting);
        curState = ObjState::MA_Waiting;
        cse->matchStatusChanged(ma.getId(), ma.getSeqNum(), ObjState::MA_Incomplete, ObjState::MA_Waiting);
      }
    }

    // from INCOMPLETE to FUZZY
    if (curState == ObjState::MA_Incomplete)
    {
      bool isFuzzy1 = (ma.rowRef().getInt(MA_Pair1SymbolicVal) != 0);
      bool isFuzzy2 = (ma.rowRef().getInt(MA_Pair2SymbolicVal) != 0);
      bool hasMatchNumber = ma.getMatchNumber() > 0;

      // we shall never have a symbolic and a real player assignment at the same time
      assert((isFuzzy1 && ma.hasPlayerPair1()) == false);
      assert((isFuzzy2 && ma.hasPlayerPair2()) == false);

      // 2019-08-19:
      // FIX: shouldn't this condition be: "(isFuzzy1 && isFuzzy2 && hasMatchNumber)"?
      //
      // The definition of ObjState::MA_Fuzzy sais: "Player names are defined by symbolic values (e.g., winner of match XYZ); match number is assigned"
      if (isFuzzy1 || isFuzzy2 || hasMatchNumber)
      {
        ma.setState(ObjState::MA_Fuzzy);
        curState = ObjState::MA_Fuzzy;
        cse->matchStatusChanged(ma.getId(), ma.getSeqNum(), ObjState::MA_Incomplete, ObjState::MA_Fuzzy);
      }
    }

    // transition from FUZZY to WAITING:
    // if we've resolved all symbolic references of a match, it may be promoted from
    // FUZZY at least to WAITING, maybe even to READY or BUSY
    if (curState == ObjState::MA_Fuzzy)
    {
      bool isFixed1 = ((ma.rowRef().getInt(MA_Pair1SymbolicVal) == 0) && (ma.rowRef().getInt(MA_Pair1Ref) > 0));
      bool isFixed2 = ((ma.rowRef().getInt(MA_Pair2SymbolicVal) == 0) && (ma.rowRef().getInt(MA_Pair2Ref) > 0));
      bool hasMatchNumber = ma.getMatchNumber() > 0;

      if (isFixed1 && isFixed2 && hasMatchNumber)
      {
        ma.setState(ObjState::MA_Waiting);
        curState = ObjState::MA_Waiting;
        cse->matchStatusChanged(ma.getId(), ma.getSeqNum(), ObjState::MA_Fuzzy, ObjState::MA_Waiting);
      }
    }

    // from WAITING to READY or BUSY
    PlayerMngr pm{db};
    bool playersAvail = ((pm.canAcquirePlayerPairsForMatch(ma)) == Error::OK);

    bool hasPredecessor = hasUnfinishedMandatoryPredecessor(ma);
    if ((curState == ObjState::MA_Waiting) && (!hasPredecessor))
    {
      curState = playersAvail ? ObjState::MA_Ready : ObjState::MA_Busy;
      ma.setState(curState);
      cse->matchStatusChanged(ma.getId(), ma.getSeqNum(), ObjState::MA_Waiting, curState);
    }

    // from READY to BUSY
    if ((curState == ObjState::MA_Ready) && !playersAvail)
    {
      ma.setState(ObjState::MA_Busy);
      curState = ObjState::MA_Busy;
      cse->matchStatusChanged(ma.getId(), ma.getSeqNum(), ObjState::MA_Ready, ObjState::MA_Busy);
    }

    // from BUSY to READY
    if ((curState == ObjState::MA_Busy) && playersAvail)
    {
      ma.setState(ObjState::MA_Ready);
      curState = ObjState::MA_Ready;
      cse->matchStatusChanged(ma.getId(), ma.getSeqNum(), ObjState::MA_Busy, ObjState::MA_Ready);
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
      if ((requiredPrevRoundsPlayersGroup == AnyPlayersGroupNumber) && (actualPrevRoundsPlayersGroup < 0))
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

      if (((requiredPrevRoundsPlayersGroup == GroupNum_Semi) ||
           (requiredPrevRoundsPlayersGroup == GroupNum_Quarter) ||
           (requiredPrevRoundsPlayersGroup == GroupNum_L16)) && (actualPrevRoundsPlayersGroup != requiredPrevRoundsPlayersGroup))
      {
        continue;  // wrong KO round
      }

      //
      // if we made it to this point, the match group is a mandatory predecessor
      //

      // in round robins, rounds are independent from each other. for this reason,
      // we may, for instance, start matches in round 3 before round 2 is finished.
      // the same assumption holds for elimination rounds
      MatchSystem mSys = cat.getMatchSystem();
      if ((mSys == MatchSystem::SingleElim) || (mSys == MatchSystem::Ranking) || (mSys == MatchSystem::RoundRobin) || ((mSys == MatchSystem::GroupsWithKO) && (mg.getGroupNumber() > 0)))
      {
        return false;
      }

      // Okay, the previous match group really has to be finished first.
      if (prevMg.is_NOT_InState(ObjState::MG_Finished)) return true;
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
        ma.rowRef().update(MA_Num, nextMatchNumber);
        updateMatchStatus(ma);

        // Manually trigger (another) update, because assigning the match number
        // does not change the match state in all cases. So we need to have at
        // least this one trigger to tell everone that the data has changed
        cse->matchStatusChanged(ma.getId(), ma.getSeqNum(), ma.getState(), ma.getState());

        ++nextMatchNumber;
      }

      // update the match group's state
      mg.setState(ObjState::MG_Scheduled);
      mg.rowRef().updateToNull(MG_StageSeqNum);  // delete the sequence number
      cse->matchGroupStatusChanged(mg.getId(), mg.getSeqNum(), ObjState::MG_Staged, ObjState::MG_Scheduled);
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
    wc.addCol(MG_StageSeqNum, ">", 0);
    wc.setOrderColumn_Asc(MG_StageSeqNum);
    return SqliteOverlay::getObjectsByWhereClause<MatchGroup>(db, groupTab, wc);
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
    wc.addCol(MA_Num, ">", 0);
    if (tab.getMatchCountForWhereClause(wc) < 1)
    {
      return 0;  // no assigned match numbers so far
    }
    */

    // determine the max match number
    Sloppy::estring sql = "SELECT max(%1) FROM %2";
    sql.arg(MA_Num);
    sql.arg(TabMatch);
    try
    {
      return db.execScalarQueryIntOrNull(sql).value_or(0);
    }
    catch (SqliteOverlay::NoDataException&)
    {
      return 0;
    }
  }

  //----------------------------------------------------------------------------

  std::optional<Match> MatchMngr::getMatchBySeqNum(int maSeqNum) const
  {
    return getSingleObjectByColumnValue<Match>(GenericSeqnumFieldName, maSeqNum);
  }

  //----------------------------------------------------------------------------

  std::optional<Match> MatchMngr::getMatchByMatchNum(int maNum) const
  {
    return getSingleObjectByColumnValue<Match>(MA_Num, maNum);
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
  Error MatchMngr::getNextViableMatchCourtPair(int *matchId, int *courtId, bool includeManualCourts) const
  {
    assert(matchId != nullptr);
    assert(courtId != nullptr);

    // default return values: error
    *matchId = -1;
    *courtId = -1;

    // find the next available match with the lowest match number
    int reqState = static_cast<int>(ObjState::MA_Ready);
    WhereClause wc;
    wc.addCol(GenericStateFieldName, reqState);
    wc.setOrderColumn_Asc(MA_Num);
    auto matchRow = tab.get2(wc);
    if (!matchRow)
    {
      return Error::NoMatchAvail;
    }

    CourtMngr cm{db};
    auto nextCourt = cm.autoSelectNextUnusedCourt(includeManualCourts);
    if (nextCourt)
    {
      *matchId = matchRow->id();
      *courtId = nextCourt->getId();
      return Error::OK;
    }

    return nextCourt.err();
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
  Error MatchMngr::canAssignMatchToCourt(const Match &ma, const Court& court) const
  {
    // check the match's state
    if (ma.is_NOT_InState(ObjState::MA_Ready))
    {
      return Error::MatchNotRunnable;
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
    Error e = pm.canAcquirePlayerPairsForMatch(ma);
    if (e != Error::OK) return e;  // Error::PlayerNotIdle

    // check if we have the necessary umpire, if required
    RefereeMode refMode = ma.get_EFFECTIVE_RefereeMode();
    if ((refMode != RefereeMode::None) && (refMode != RefereeMode::HandWritten))
    {
      auto referee = ma.getAssignedReferee();

      if (!referee) return Error::MatchNeedsReferee;

      // check if the assigned referee is available
      if (referee->is_NOT_InState(ObjState::PL_Idle)) return Error::RefereeNotIdle;
    }

    // check the court's availability
    ObjState stat = court.getState();
    if (stat == ObjState::CO_Avail)
    {
      return Error::OK;
    }
    if (stat == ObjState::CO_Disabled)
    {
      return Error::CourtDisabled;
    }

    return Error::CourtBusy;
  }

  //----------------------------------------------------------------------------

  Error MatchMngr::assignMatchToCourt(const Match &ma, const Court &court) const
  {
    Error e = canAssignMatchToCourt(ma, court);
    if (e != Error::OK) return e;

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
    cvc.addCol(MA_CourtRef, court.getId());

    // copy the actual players to the database
    // TODO: implement substitute players etc. So far, we only copy
    // the contents of the player pairs blindly
    PlayerPair pp = ma.getPlayerPair1();
    cvc.addCol(MA_ActualPlayer1aRef, pp.getPlayer1().getId());
    if (pp.hasPlayer2())
    {
      cvc.addCol(MA_ActualPlayer1bRef, pp.getPlayer2().getId());
    }
    pp = ma.getPlayerPair2();
    cvc.addCol(MA_ActualPlayer2aRef, pp.getPlayer1().getId());
    if (pp.hasPlayer2())
    {
      cvc.addCol(MA_ActualPlayer2bRef, pp.getPlayer2().getId());
    }

    // update the match state
    cvc.addCol(GenericStateFieldName, static_cast<int>(ObjState::MA_Running));

    // execute all updates at once
    auto trans = db.startTransaction();

    ma.rowRef().update(cvc);

    // tell the world that the match status has changed
    CentralSignalEmitter::getInstance()->matchStatusChanged(ma.getId(), ma.getSeqNum(), ObjState::MA_Ready, ObjState::MA_Running);

    // update the player's status
    PlayerMngr pm{db};
    e = pm.acquirePlayerPairsForMatch(ma);
    assert(e == Error::OK);  // must be, because the condition has been check by canAssignMatchToCourt()

    // acquire the referee, if any
    RefereeMode refMode = ma.get_EFFECTIVE_RefereeMode();
    if ((refMode != RefereeMode::None) && (refMode != RefereeMode::HandWritten))
    {
      auto referee = ma.getAssignedReferee();

      // the following assertion must hold,
      // because the conditions have been check by canAssignMatchToCourt()
      assert(referee);
      assert(referee->isInState(ObjState::PL_Idle));

      referee->setState(ObjState::PL_Referee);
    }

    // store the effective referee-mode at call time.
    //
    // This is necessary because cmdAssignRefereeToMatch wouldn't work
    // correctly if the referee mode is USE DEFAULT and we change the
    // default mode e.g. to RefereeMode::None after the match has been called. The
    // same applies to the CourtItemDelegate.
    //
    // Note: since we overwrite the mode in the match tab, the old mode
    // is not restored when undoing the match call.
    if (ma.get_RAW_RefereeMode() == RefereeMode::UseDefault)
    {
      // Note: we don't use setRefereeMode() here, because a few lines
      // above we've already changed the match state to RUNNING and
      // setRefereeMode() refuses to update matches in state RUNNING.
      // So we have to hard-code the mode change here
      auto cfg = SqliteOverlay::KeyValueTab{db, TabCfg};
      int tnmtDefaultRefereeModeId = cfg.getInt(CfgKey_DefaultRefereemode);
      ma.rowRef().update(MA_RefereeMode, tnmtDefaultRefereeModeId);
    }

    // now we finally acquire the court in the aftermath
    CourtMngr cm{db};
    bool isOkay = cm.acquireCourt(court);
    assert(isOkay);

    // update the category's state to "PLAYING", if necessary
    CatMngr catm{db};
    catm.updateCatStatusFromMatchStatus(ma.getCategory());

    // store the call time in the database
    ma.rowRef().update(MA_StartTime, UTCTimestamp());

    // check all matches that are currently "READY" because
    // due to the player allocation, some of them might have
    // become "BUSY"
    WhereClause wc;
    wc.addCol(GenericStateFieldName, static_cast<int>(ObjState::MA_Ready));
    for (TabRowIterator it{db, TabMatch, wc}; it.hasData(); ++it)
    {
      const Match otherMatch{db, *it};
      updateMatchStatus(otherMatch);
    }

    trans.commit();

    return Error::OK;
  }

  //----------------------------------------------------------------------------

  std::optional<Court> MatchMngr::autoAssignMatchToNextAvailCourt(const Match &ma, Error *err, bool includeManualCourts) const
  {
    assert(err != nullptr);

    CourtMngr cm{db};
    auto nextCourt = cm.autoSelectNextUnusedCourt(includeManualCourts);
    if (nextCourt)
    {
      *err = assignMatchToCourt(ma, *nextCourt);
      return (*err == Error::OK) ? *nextCourt : std::optional<Court>{};
    }

    // return the error resulting from the court selection
    *err = nextCourt.err();
    return {};
  }

  //----------------------------------------------------------------------------

  Error MatchMngr::setMatchScoreAndFinalizeMatch(const Match &ma, const MatchScore &score, bool isWalkover) const
  {
    // check the match's state
    ObjState oldState = ma.getState();
    if ((!isWalkover) && (oldState != ObjState::MA_Running))
    {
      return Error::MatchNotRunning;
    }
    if (isWalkover && (!(ma.isWalkoverPossible())))
    {
      return Error::WrongState;
    }

    // check if the score is valid for the category settings
    Category cat = ma.getCategory();
    bool isDrawAllowed = cat.isDrawAllowedInRound(ma.getMatchGroup().getRound());
    int numWinGames = 2; // TODO: this needs to become a category parameter!
    if (!(score.isValidScore(numWinGames, isDrawAllowed)))
    {
      return Error::InvalidMatchResultForCategorySettings;
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
      auto trans = db.startTransaction();

      // store score and FINISH status
      ColumnValueClause cvc;
      cvc.addCol(MA_Result, score.toString().toUtf8().constData());
      cvc.addCol(GenericStateFieldName, static_cast<int>(ObjState::MA_Finished));

      // store the finish time in the database, but only if this is not
      // a walkover and only if the match was started regularly
      if ((oldState == ObjState::MA_Running) && !isWalkover)   // match was called normally, so we have a start time
      {
        UTCTimestamp now;
        cvc.addCol(MA_FinishTime, &now);
      }

      // apply the update
      ma.rowRef().update(cvc);

      // let the world know what has happened
      int maId = ma.getId();
      int maSeqNum = ma.getSeqNum();
      CentralSignalEmitter* cse = CentralSignalEmitter::getInstance();
      cse->matchResultUpdated(maId, maSeqNum);
      cse->matchStatusChanged(maId, maSeqNum, oldState, ObjState::MA_Finished);

      // if this was a regular, running match we need to release the court
      // and the players
      PlayerMngr pm{db};
      CourtMngr cm{db};
      if (oldState == ObjState::MA_Running)
      {
        // release the players
        Error err = pm.releasePlayerPairsAfterMatch(ma);
        if (err != Error::OK) return err;

        // release the umpire, if any
        auto referee = ma.getAssignedReferee();
        if (referee && (referee->isInState(ObjState::PL_Referee)))
        {
          referee->setState(ObjState::PL_Idle);
          pm.increaseRefereeCountForPlayer(*referee);
        }

        // release the court
        auto court = ma.getCourt(&err);
        if (err != Error::OK) return err;
        bool isOkay = cm.releaseCourt(*court);
        if (!isOkay) return Error::DatabaseError;
      }

      // update the match group
      updateAllMatchGroupStates(ma.getCategory());

      // update other matches in this category from WAITING to READY or BUSY, if applicable
      for (MatchGroup mg : getMatchGroupsForCat(ma.getCategory()))
      {
        for (const Match& otherMatch : getMatchesForMatchGroup(mg))
        {
          if (otherMatch.is_NOT_InState(ObjState::MA_Waiting)) continue;
          updateMatchStatus(otherMatch);
        }
      }

      // in case some other match refers to this match with a symbolic name
      // (e.g., winner of match XYZ), resolve those symbolic names into
      // real player pairs
      resolveSymbolicNamesAfterFinishedMatch(ma);

      // update the category's state to "KO_Start::FinalIZED", if necessary
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
      for (const Match& otherMatch : getObjectsByColumnValue<Match>(GenericStateFieldName, static_cast<int>(ObjState::MA_Busy)))
      {
        updateMatchStatus(otherMatch);
      }

      // commit all changes
      trans.commit();

      return Error::OK;
    }
    catch (BusyException&)
    {
      return Error::DatabaseError;
    }
    catch (GenericSqliteException&)
    {
      return Error::DatabaseError;
    }
  }

  //----------------------------------------------------------------------------

  Error MatchMngr::updateMatchScore(const Match& ma, const MatchScore& newScore, bool winnerLoserChangePermitted) const
  {
    //
    // IMPORTANT:
    // This method __blindly__ sets the match score to whatever is provided by
    // the caller.
    // It is the caller's responsibility to ensure that a modification of the
    // match score is possible at all
    //

    // only modify finished matches
    if (ma.is_NOT_InState(ObjState::MA_Finished)) return Error::WrongState;

    // make sure the score itself is valid
    Category cat = ma.getCategory();
    bool isDrawAllowed = cat.isDrawAllowedInRound(ma.getMatchGroup().getRound());
    int numWinGames = 2; // TODO: this needs to become a category parameter!
    if (!(newScore.isValidScore(numWinGames, isDrawAllowed)))
    {
      return Error::InvalidMatchResultForCategorySettings;
    }

    // if no change of the winner/loser is permitted, compare
    // the new score with the old one
    if (!winnerLoserChangePermitted)
    {
      auto oldScore = ma.getScore();

      if (oldScore->getWinner() != newScore.getWinner())
      {
        // well, the error code doesn't match exactly... but we just take what's there
        return Error::InconsistentMatchResultString;
      }
    }

    // everything is fine, so write the result to the database
    ma.rowRef().update(MA_Result, newScore.toString().toUtf8().constData());

    CentralSignalEmitter* cse = CentralSignalEmitter::getInstance();
    cse->matchResultUpdated(ma.getId(), ma.getSeqNum());

    return Error::OK;
  }

  //----------------------------------------------------------------------------

  Error MatchMngr::walkover(const Match& ma, int winningPlayerNum) const
  {
    // for a walkover, the match must be in READY, WAITING, RUNNING or BUSY
    if (!(ma.isWalkoverPossible()))
    {
      return Error::WrongState;
    }

    // the playerNum must be either 1 or 2
    if ((winningPlayerNum != 1) && (winningPlayerNum != 2))
    {
      return Error::InvalidPlayerPair;
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

  Error MatchMngr::undoMatchCall(const Match& ma) const
  {
    if (ma.is_NOT_InState(ObjState::MA_Running))
    {
      return Error::MatchNotRunning;
    }

    // release the players first, because we need the entries
    // in MA_ActualPlayer1aRef etc.
    PlayerMngr pm{db};
    pm.releasePlayerPairsAfterMatch(ma);

    // release the umpire, if any
    auto referee = ma.getAssignedReferee();
    if (referee && (referee->isInState(ObjState::PL_Referee)))
    {
      referee->setState(ObjState::PL_Idle);
    }

    // store the court the match is running on
    Error e;
    auto court = ma.getCourt(&e);
    assert(e == Error::OK);

    // reset the references to the court and the actual players
    ColumnValueClause cvc;
    cvc.addNullCol(MA_ActualPlayer1aRef);
    cvc.addNullCol(MA_ActualPlayer1bRef);
    cvc.addNullCol(MA_ActualPlayer2aRef);
    cvc.addNullCol(MA_ActualPlayer2bRef);
    cvc.addNullCol(MA_CourtRef);

    // set the state back to READY
    cvc.addCol(GenericStateFieldName, static_cast<int>(ObjState::MA_Ready));

    // apply all changes at once
    ma.rowRef().update(cvc);
    CentralSignalEmitter::getInstance()->matchStatusChanged(ma.getId(), ma.getSeqNum(), ObjState::MA_Running, ObjState::MA_Ready);

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
    ma.rowRef().updateToNull(MA_StartTime);
    ma.rowRef().updateToNull(MA_AdditionalCallTimes);

    // check all matches that are currently "BUSY" because
    // due to the player release, some of them might have
    // become "READY"
    for (const Match& otherMatch : getObjectsByColumnValue<Match>(GenericStateFieldName, static_cast<int>(ObjState::MA_Busy)))
    {
      updateMatchStatus(otherMatch);
    }

    return Error::OK;
  }

  //----------------------------------------------------------------------------

  std::optional<Match> MatchMngr::getMatchForCourt(const Court& court)
  {
    // search for matches in state RUNNING and assigned to the court
    WhereClause wc;
    wc.addCol(MA_CourtRef, court.getId());
    wc.addCol(GenericStateFieldName, static_cast<int>(ObjState::MA_Running));

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
    int nRunning = tab.getMatchCountForColumnValue(GenericStateFieldName, static_cast<int>(ObjState::MA_Running));

    // get the number of finished matches
    int nFinished = tab.getMatchCountForColumnValue(GenericStateFieldName, static_cast<int>(ObjState::MA_Finished));

    // get the number of scheduled matches
    WhereClause wc;
    wc.addCol(MA_Num, ">", 0);
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
    if (toState == ObjState::PL_Playing)
    {
      for (const Match& ma : getObjectsByColumnValue<Match>(GenericStateFieldName, static_cast<int>(ObjState::MA_Ready)))
      {
        const PlayerList pl = ma.determineActualPlayers();
        for (const Player& p : pl)
        {
          if (p.getId() == playerId)
          {
            ma.rowRef().update(GenericStateFieldName, static_cast<int>(ObjState::MA_Busy));
            cse->matchStatusChanged(ma.getId(), ma.getSeqNum(), ObjState::MA_Ready, ObjState::MA_Busy);
            break;  // no need to check other players for this match
          }
        }
      }
    }

    // switch matches from BUSY to READY, if all players are available again
    if (toState == ObjState::PL_Idle)
    {
      PlayerMngr pm{db};
      for (Match ma : getObjectsByColumnValue<Match>(GenericStateFieldName, static_cast<int>(ObjState::MA_Busy)))
      {
        if (pm.canAcquirePlayerPairsForMatch(ma) == Error::OK)
        {
          ma.rowRef().update(GenericStateFieldName, static_cast<int>(ObjState::MA_Ready));
          cse->matchStatusChanged(ma.getId(), ma.getSeqNum(), ObjState::MA_Busy, ObjState::MA_Ready);
        }
      }
    }
  }

  //----------------------------------------------------------------------------

  MatchList MatchMngr::getCurrentlyRunningMatches() const
  {
    return getObjectsByColumnValue<Match>(GenericStateFieldName, static_cast<int>(ObjState::MA_Running));
  }

  //----------------------------------------------------------------------------

  MatchList MatchMngr::getFinishedMatches() const
  {
    return getObjectsByColumnValue<Match>(GenericStateFieldName, static_cast<int>(ObjState::MA_Finished));
  }

  //----------------------------------------------------------------------------

  MatchList MatchMngr::getMatchesForMatchGroup(const MatchGroup &grp) const
  {
    return getObjectsByColumnValue<Match>(MA_GrpRef, grp.getId());
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
    int grpCount = groupTab.getMatchCountForColumnValue(MG_CatRef, cat.getId());
    if (grpCount == 0) return 0;
    */

    // query the highest used round number
    Sloppy::estring sql = "SELECT max(%1) FROM %2 WHERE %3 = %4";
    sql.arg(MG_Round);
    sql.arg(TabMatchGroup);
    sql.arg(MG_CatRef);
    sql.arg(cat.getId());

    try
    {
      return db.execScalarQueryIntOrNull(sql).value_or(0);
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
      where.arg(MA_Pair1Ref);
      where.arg(pp.getPairId());
      where.arg(MA_Pair2Ref);
      where.arg(MA_GrpRef);
      where.arg(mg.getId());

      auto result = getSingleObjectByWhereClause<Match>(where);
      if (result) return result;
    }
    return {};
  }

  //----------------------------------------------------------------------------

  void MatchMngr::resolveSymbolicNamesAfterFinishedMatch(const Match &ma) const
  {
    if (ma.is_NOT_InState(ObjState::MA_Finished))
    {
      return;
    }

    int matchId = ma.getId();
    auto winnerPair = ma.getWinner();
    auto loserPair = ma.getLoser();

    static const std::vector<std::tuple<std::string, std::string>> colPairs{
      {MA_Pair1SymbolicVal, MA_Pair1Ref},
      {MA_Pair2SymbolicVal, MA_Pair2Ref},
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
          if ((stat != ObjState::MA_Fuzzy) && (stat != ObjState::MA_Incomplete)) continue;

          m.rowRef().update(pairRefColName, pairId);  // set the reference to the winner / loser
          m.rowRef().update(symbolColName, 0);   // delete symbolic reference

          // emit a faked state change to trigger a display update of the
          // match in the match tab view
          cse->matchStatusChanged(m.getId(), m.getSeqNum(), stat, stat);
        }
      }
    }

    // if we resolved all symbolic references of a match, it may be promoted from
    // FUZZY at least to WAITING, maybe even to READY or BUSY
    WhereClause wc;
    wc.addCol(GenericStateFieldName, static_cast<int>(ObjState::MA_Fuzzy));
    wc.addCol(MA_Pair1SymbolicVal, 0);
    wc.addCol(MA_Pair2SymbolicVal, 0);
    wc.addCol(MA_Pair1Ref, ">", 0);
    wc.addCol(MA_Pair2Ref, ">", 0);
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
