/* 
 * File:   tstGenericDBObject.cpp
 * Author: volker
 * 
 * Created on March 2, 2014, 3:46 PM
 */

#include <stdexcept>

#include "tstMatchMngr.h"
#include "BasicTestClass.h"
#include "GenericDatabaseObject.h"
#include "TournamentDB.h"
#include "TournamentDataDefs.h"
#include "MatchMngr.h"
#include "TournamentErrorCodes.h"
#include "DbTab.h"
#include "Tournament.h"

using namespace QTournament;
using namespace dbOverlay;

//----------------------------------------------------------------------------

void tstMatchMngr::testCreateNewGroup()
{
  printStartMsg("tstMatchMngr::testCreateNewGroup");
  
  TournamentDB* db = getScenario02(true);
  Tournament t(getSqliteFileName());
  MatchMngr* mm = Tournament::getMatchMngr();
  
  // try invalid category states
  Category ms = Tournament::getCatMngr()->getCategoryById(1);
  ERR e;
  CPPUNIT_ASSERT(ms.getState() == STAT_CAT_Config);
  auto mg = mm->createMatchGroup(ms, 2, 3, &e);
  CPPUNIT_ASSERT(e == CategoryStillConfigurable);
  CPPUNIT_ASSERT(mg == nullptr);
  TabRow catRow = (*db)[TabCategory][1];
  catRow.update(GenericStateFieldName, static_cast<int>(STAT_CAT_Frozen));
  CPPUNIT_ASSERT(ms.getState() == STAT_CAT_Frozen);
  mg = mm->createMatchGroup(ms, 2, 3, &e);
  CPPUNIT_ASSERT(e == CategoryStillConfigurable);
  CPPUNIT_ASSERT(mg == nullptr);

  // fake a valid category state
  catRow.update(GenericStateFieldName, static_cast<int>(STAT_CAT_Idle));
  CPPUNIT_ASSERT(ms.getState() == STAT_CAT_Idle);

  // try empty or invalid parameters
  mg = mm->createMatchGroup(ms, -5, 1, &e);
  CPPUNIT_ASSERT(e == InvalidRound);
  CPPUNIT_ASSERT(mg == nullptr);

  mg = mm->createMatchGroup(ms, 0, 1, &e);
  CPPUNIT_ASSERT(e == InvalidRound);
  CPPUNIT_ASSERT(mg == nullptr);

  mg = mm->createMatchGroup(ms, 1, -42, &e);
  CPPUNIT_ASSERT(e == InvalidGroupNum);
  CPPUNIT_ASSERT(mg == nullptr);

  mg = mm->createMatchGroup(ms, 1, 0, &e);
  CPPUNIT_ASSERT(e == InvalidGroupNum);
  CPPUNIT_ASSERT(mg == nullptr);
  
  // make sure nothing has been inserted so far
  CPPUNIT_ASSERT((*db)[TabMatch_GROUP].length() == 0);
  
  // create a valid group with a real (positive) group number
  mg = mm->createMatchGroup(ms, 2, 3, &e);
  CPPUNIT_ASSERT(e == OK);
  CPPUNIT_ASSERT(mg != nullptr);
  
  // create the same group twice
  auto mg2 = mm->createMatchGroup(ms, 2, 3, &e);
  CPPUNIT_ASSERT(e == MatchGroupExists);
  CPPUNIT_ASSERT(mg2 == nullptr);
  
  // check database entries
  CPPUNIT_ASSERT((*db)[TabMatch_GROUP].length() == 1);
  CPPUNIT_ASSERT((*db)[TabMatch_GROUP][1][MG_CatRef].toInt() == 1);
  CPPUNIT_ASSERT((*db)[TabMatch_GROUP][1][MG_Round].toInt() == 2);
  CPPUNIT_ASSERT((*db)[TabMatch_GROUP][1][MG_GrpNum].toInt() == 3);
  CPPUNIT_ASSERT(mg->getState() == STAT_MG_Config);
  
  // create a match group with a special value for the group number
  mg2 = mm->createMatchGroup(ms, 4, GroupNum_L16, &e);
  CPPUNIT_ASSERT(e == OK);
  CPPUNIT_ASSERT(mg2 != 0);
  CPPUNIT_ASSERT((*db)[TabMatch_GROUP].length() == 2);
  CPPUNIT_ASSERT((*db)[TabMatch_GROUP][2][MG_CatRef].toInt() == 1);
  CPPUNIT_ASSERT((*db)[TabMatch_GROUP][2][MG_Round].toInt() == 4);
  CPPUNIT_ASSERT((*db)[TabMatch_GROUP][2][MG_GrpNum].toInt() == -8);
  CPPUNIT_ASSERT(mg2->getState() == STAT_MG_Config);
  
  delete db;
  printEndMsg();
}

//----------------------------------------------------------------------------

void tstMatchMngr::testHasGroup()
{
  printStartMsg("tstMatchMngr::testHasGroup");
  
  TournamentDB* db = getScenario02(true);
  Tournament t(getSqliteFileName());
  MatchMngr* mm = Tournament::getMatchMngr();
  
  // try empty or invalid parameters
  Category ms = Tournament::getCatMngr()->getCategoryById(1);
  ERR e;
  CPPUNIT_ASSERT(mm->hasMatchGroup(ms, -5, 1, &e) == false);
  CPPUNIT_ASSERT(e == InvalidRound);

  CPPUNIT_ASSERT(mm->hasMatchGroup(ms, 0, 1, &e) == false);
  CPPUNIT_ASSERT(e == InvalidRound);

  CPPUNIT_ASSERT(mm->hasMatchGroup(ms, 1, -42, &e) == false);
  CPPUNIT_ASSERT(e == InvalidGroupNum);

  CPPUNIT_ASSERT(mm->hasMatchGroup(ms, 1, 0, &e) == false);
  CPPUNIT_ASSERT(e == InvalidGroupNum);
  
  // try empty or invalid parameters without
  // pointer to an error code
  CPPUNIT_ASSERT(mm->hasMatchGroup(ms, -5, 1) == false);
  CPPUNIT_ASSERT(mm->hasMatchGroup(ms, 0, 1) == false);
  CPPUNIT_ASSERT(mm->hasMatchGroup(ms, 1, -42) == false);
  CPPUNIT_ASSERT(mm->hasMatchGroup(ms, 1, 0) == false);
  
  // fake a valid category state
  TabRow catRow = (*db)[TabCategory][1];
  catRow.update(GenericStateFieldName, static_cast<int>(STAT_CAT_Idle));
  CPPUNIT_ASSERT(ms.getState() == STAT_CAT_Idle);

  // create a valid group with a real (positive) group number
  auto mg = mm->createMatchGroup(ms, 2, 3, &e);
  CPPUNIT_ASSERT(e == OK);
  CPPUNIT_ASSERT(mg != nullptr);

  // look up this group
  CPPUNIT_ASSERT(mm->hasMatchGroup(ms, 2, 3, &e) == true);
  CPPUNIT_ASSERT(e == OK);
  CPPUNIT_ASSERT(mm->hasMatchGroup(ms, 2, 3) == true);
  
  // look up a non-existing group with valid parameters
  CPPUNIT_ASSERT(mm->hasMatchGroup(ms, 3, 3, &e) == false);
  CPPUNIT_ASSERT(e == NoSuchMatchGroup);
  CPPUNIT_ASSERT(mm->hasMatchGroup(ms, 3, 3) == false);
  
  delete db;
  printEndMsg();
}

//----------------------------------------------------------------------------
    
void tstMatchMngr::testGetGroup()
{
  printStartMsg("tstMatchMngr::testHasGroup");
  
  TournamentDB* db = getScenario02(true);
  Tournament t(getSqliteFileName());
  MatchMngr* mm = Tournament::getMatchMngr();
  Category ms = Tournament::getCatMngr()->getCategoryById(1);
  
  // don't repeat all the test for INVALID_XXX here, because internally
  // it's the same code for hasMatchGroup (see above) and getMatchGroup()
  // that's being executed for the checks
  
  // so we only check here whether the right object is returned
  
  // fake a valid category state
  TabRow catRow = (*db)[TabCategory][1];
  catRow.update(GenericStateFieldName, static_cast<int>(STAT_CAT_Idle));
  CPPUNIT_ASSERT(ms.getState() == STAT_CAT_Idle);

  // create a valid group with a real (positive) group number
  ERR e;
  auto mg = mm->createMatchGroup(ms, 2, 3, &e);
  CPPUNIT_ASSERT(e == OK);
  CPPUNIT_ASSERT(mg != nullptr);

  auto mg2 = mm->getMatchGroup(ms, 2, 3, &e);
  CPPUNIT_ASSERT(e == OK);
  CPPUNIT_ASSERT(mg2 != nullptr);
  CPPUNIT_ASSERT((*mg2) == (*mg));
  
  CPPUNIT_ASSERT(mg2 != nullptr);
  mg2 = mm->getMatchGroup(ms, 3, 3, &e);
  CPPUNIT_ASSERT(e == NoSuchMatchGroup);
  CPPUNIT_ASSERT(mg2 == nullptr);
  
  delete db;
  printEndMsg();
}

//----------------------------------------------------------------------------

void tstMatchMngr::testCreateNewMatch()
{
  printStartMsg("tstMatchMngr::testCreateNewMatch");

  TournamentDB* db = getScenario02(true);
  Tournament t(getSqliteFileName());
  MatchMngr* mm = Tournament::getMatchMngr();
  Category ms = Tournament::getCatMngr()->getCategoryById(1);

  // fake a valid category state
  TabRow catRow = (*db)[TabCategory][1];
  catRow.update(GenericStateFieldName, static_cast<int>(STAT_CAT_Idle));
  CPPUNIT_ASSERT(ms.getState() == STAT_CAT_Idle);

  // create a new match group
  ERR e;
  auto grp = mm->createMatchGroup(ms, 2, 3, &e);
  CPPUNIT_ASSERT(e == OK);
  CPPUNIT_ASSERT(grp != nullptr);

  // create a new match in this group
  auto ma = mm->createMatch(*grp, &e);
  CPPUNIT_ASSERT(e == OK);
  CPPUNIT_ASSERT(ma != nullptr);

  // check database entries
  CPPUNIT_ASSERT((*db)[TabMatch].length() == 1);
  CPPUNIT_ASSERT((*db)[TabMatch][1][MA_GrpRef].toInt() == 1);
  CPPUNIT_ASSERT(ma->getState() == STAT_MA_Incomplete);

  // fake a new match group state
  (*db)[TabMatch_GROUP][1].update(GenericStateFieldName, static_cast<int>(STAT_MG_Frozen));
  CPPUNIT_ASSERT(grp->getState() == STAT_MG_Frozen);

  // try to create a new match in a frozen match group
  auto ma2 = mm->createMatch(*grp, &e);
  CPPUNIT_ASSERT(e == MatchGroupNotConfiguraleAnymore);
  CPPUNIT_ASSERT(ma2 == nullptr);

  delete db;
  printEndMsg();
}

//----------------------------------------------------------------------------

void tstMatchMngr::testCanAssignPlayerPairToMatch()
{
  printStartMsg("tstMatchMngr::testCanAssignPlayerPairToMatch");

  TournamentDB* db = getScenario03(true);
  Tournament t(getSqliteFileName());
  MatchMngr* mm = Tournament::getMatchMngr();
  Category ms = Tournament::getCatMngr()->getCategoryById(1);
  Category ls = Tournament::getCatMngr()->getCategoryById(3);
  Category mx = Tournament::getCatMngr()->getCategoryById(5);

  // fake / create a few player pairs
  auto tabPairs = db->getTab(TabPairs);
  auto catMngr = Tournament::getCatMngr();
  Player m1 = Tournament::getPlayerMngr()->getPlayer("f", "l1");
  Player f1 = Tournament::getPlayerMngr()->getPlayer("f", "l2");
  Player m2 = Tournament::getPlayerMngr()->getPlayer("f", "l3");
  Player m3 = Tournament::getPlayerMngr()->getPlayer("f", "l5");
  CPPUNIT_ASSERT(catMngr->pairPlayers(mx, m1, f1) == OK);
  PlayerPair mxPair{m1, f1, 1};
  CPPUNIT_ASSERT(mxPair.isConsistent(db));

  // leave out Pairs_Player2Ref to assign a NULL value in the database
  QVariantList qvl;
  qvl << Pairs_CatRef << 1; // ms
  qvl << Pairs_GrpNum << 3;
  qvl << Pairs_Player1Ref << m1.getId();
  tabPairs.insertRow(qvl);
  PlayerPair msPair1{m1, 2};
  CPPUNIT_ASSERT(msPair1.isConsistent(db));

  qvl.clear();
  qvl << Pairs_CatRef << 1; // ms
  qvl << Pairs_GrpNum << GroupNum_NotAssigned;
  qvl << Pairs_Player1Ref << m2.getId();
  tabPairs.insertRow(qvl);
  PlayerPair msPair2{m2, 3};
  CPPUNIT_ASSERT(msPair2.isConsistent(db));

  qvl.clear();
  qvl << Pairs_CatRef << 1; // ms
  qvl << Pairs_GrpNum << 3;
  qvl << Pairs_Player1Ref << m3.getId();
  tabPairs.insertRow(qvl);
  PlayerPair msPair3{m3, 4};
  CPPUNIT_ASSERT(msPair3.isConsistent(db));

  qvl.clear();
  qvl << Pairs_CatRef << 3; // ls
  qvl << Pairs_GrpNum << GroupNum_NotAssigned;
  qvl << Pairs_Player1Ref << f1.getId();
  tabPairs.insertRow(qvl);
  PlayerPair lsPair1{f1, 5};
  CPPUNIT_ASSERT(lsPair1.isConsistent(db));

  qvl.clear();
  qvl << Pairs_CatRef << 3; // ls
  qvl << Pairs_GrpNum << GroupNum_NotAssigned;
  qvl << Pairs_Player1Ref << m1.getId();  // semantically wrong (male in lady singles), here only for testing
  tabPairs.insertRow(qvl);
  PlayerPair weirdPair{m1, 6};
  CPPUNIT_ASSERT(weirdPair.isConsistent(db));

  // fake a valid category state
  TabRow catRow = (*db)[TabCategory][1];
  catRow.update(GenericStateFieldName, static_cast<int>(STAT_CAT_Idle));
  CPPUNIT_ASSERT(ms.getState() == STAT_CAT_Idle);

  // create 3 rounds with 8 groups of 4 matches each
  ERR e;
  for (int round: {1, 2, 3})
  {
    for (int nGrp = 0; nGrp < 8; ++nGrp)
    {
      auto grp = mm->createMatchGroup(ms, round, nGrp+1, &e);
      CPPUNIT_ASSERT(e == OK);
      CPPUNIT_ASSERT(grp != nullptr);

      for (int nMatch = 0; nMatch < 4; ++nMatch)
      {
        auto ma = mm->createMatch(*grp, &e);
        CPPUNIT_ASSERT(e == OK);
        CPPUNIT_ASSERT(ma != nullptr);
      }
    }
  }

  //----------- The actual test starts here ------------------

  // try assigning to a match in an invalid state
  MatchGroup mg = mm->getMatchGroupsForCat(ms, 3).at(0);
  Match m = mg.getMatches().at(0);
  auto tabMatch = db->getTab(TabMatch);
  for (auto stat : {STAT_MA_Ready, STAT_MA_Busy, STAT_MA_Finished, STAT_MA_Fuzzy, STAT_MA_Postponed, STAT_MA_Ready, STAT_MA_Running})
  {
    tabMatch[m.getId()].update(GenericStateFieldName, static_cast<int>(stat));
    CPPUNIT_ASSERT(m.getState() == stat);
    CPPUNIT_ASSERT(mm->canAssignPlayerPairToMatch(m, msPair1) == MatchNotConfiguraleAnymore);
  }

  // reset match state
  tabMatch[m.getId()].update(GenericStateFieldName, static_cast<int>(STAT_MA_Incomplete));
  CPPUNIT_ASSERT(m.getState() == STAT_MA_Incomplete);

  // try to assign a player pair from the wrong category
  CPPUNIT_ASSERT(mm->canAssignPlayerPairToMatch(m, lsPair1) == PlayerNotInCategory);
  CPPUNIT_ASSERT(mm->canAssignPlayerPairToMatch(m, weirdPair) == PlayerNotInCategory);  // single male, also in MS, but this pair is not in MS

  // "m" is a match in round 3, group 1
  //
  // try to assign a pair with "unassigned group number" or with a group number != 1
  CPPUNIT_ASSERT(mm->canAssignPlayerPairToMatch(m, msPair2) == GroupNumberMismatch);  // unassigned number for pair2
  CPPUNIT_ASSERT(mm->canAssignPlayerPairToMatch(m, msPair1) == GroupNumberMismatch);  // group num 3 for pair 1

  // switch to a match in round 1, group 3
  mg = mm->getMatchGroupsForCat(ms, 1).at(2);
  CPPUNIT_ASSERT(mg.getCategory() == ms);
  CPPUNIT_ASSERT(mg.getRound() == 1);
  CPPUNIT_ASSERT(mg.getGroupNumber() == 3);
  m = mg.getMatches().at(0);
  Match ma2 = mg.getMatches().at(1);  // another match in the same round and group
  mg = mm->getMatchGroupsForCat(ms, 1).at(1);  // same round, but group 2
  Match ma3 = mg.getMatches().at(1);  // same round, group 2, match 2

  // assign a valid player pair
  CPPUNIT_ASSERT(mm->canAssignPlayerPairToMatch(m, msPair1) == OK);
  CPPUNIT_ASSERT(mm->canAssignPlayerPairToMatch(m, msPair3) == OK);
  CPPUNIT_ASSERT(mm->setPlayerPairsForMatch(m, msPair1, msPair3) == OK);
  auto matchRow = tabMatch[m.getId()];
  CPPUNIT_ASSERT(matchRow[MA_Pair1Ref].toInt() == msPair1.getPairId());
  CPPUNIT_ASSERT(matchRow[MA_Pair2Ref].toInt() == msPair3.getPairId());

  // try to use the same pair in the same round and group twice
  CPPUNIT_ASSERT(mm->canAssignPlayerPairToMatch(ma2, msPair1) == PlayerAlreadyAssignedToOtherMatchInTheSameRoundAndCategory);
  CPPUNIT_ASSERT(mm->canAssignPlayerPairToMatch(ma2, msPair3) == PlayerAlreadyAssignedToOtherMatchInTheSameRoundAndCategory);

  // try to use the same pair in the same round but a different group
  CPPUNIT_ASSERT(mm->canAssignPlayerPairToMatch(ma3, msPair1) == GroupNumberMismatch);
  CPPUNIT_ASSERT(mm->canAssignPlayerPairToMatch(ma3, msPair3) == GroupNumberMismatch);

  // re-assigning the player to the same match should work
  CPPUNIT_ASSERT(mm->canAssignPlayerPairToMatch(m, msPair1) == OK);
  CPPUNIT_ASSERT(mm->canAssignPlayerPairToMatch(m, msPair3) == OK);
  CPPUNIT_ASSERT(mm->setPlayerPairsForMatch(m, msPair1, msPair3) == OK);

  // assigning the same player pair to the same match for both opponent roles must fail
  CPPUNIT_ASSERT(mm->setPlayerPairsForMatch(m, msPair1, msPair1) == PlayersIdentical);

  delete db;
  printEndMsg();
}

//----------------------------------------------------------------------------

void tstMatchMngr::testStageAndUnstageMatchGroup()
{
  printStartMsg("tstMatchMngr::testStageAndUnstageMatchGroup");

  TournamentDB* db = getScenario05(true);
  Tournament t(getSqliteFileName());
  MatchMngr* mm = Tournament::getMatchMngr();
  Category ms = Tournament::getCatMngr()->getCategoryById(1);

  // test getMaxStageSeqNum for empty staging area
  CPPUNIT_ASSERT(mm->getMaxStageSeqNum() == 0);

  // get a "non-stageable" match group
  ERR e;
  auto mg1_2 = mm->getMatchGroup(ms, 2, 1, &e);  // players group 1, round 2
  CPPUNIT_ASSERT(mg1_2 != nullptr);
  CPPUNIT_ASSERT(e == OK);
  CPPUNIT_ASSERT(mg1_2->getState() == STAT_MG_Frozen);
  CPPUNIT_ASSERT(mg1_2->getStageSequenceNumber() == -1);

  // try to stage this group
  CPPUNIT_ASSERT(mm->stageMatchGroup(*mg1_2) == WrongState);
  CPPUNIT_ASSERT(mg1_2->getState() == STAT_MG_Frozen);
  CPPUNIT_ASSERT(mg1_2->getStageSequenceNumber() == -1);

  // get a "stageable" group
  auto mg1_1 = mm->getMatchGroup(ms, 1, 1, &e);  // players group 1, round 1
  CPPUNIT_ASSERT(mg1_1 != nullptr);
  CPPUNIT_ASSERT(e == OK);
  CPPUNIT_ASSERT(mg1_1->getState() == STAT_MG_Idle);
  CPPUNIT_ASSERT(mg1_1->getStageSequenceNumber() == -1);

  // try to stage this group
  CPPUNIT_ASSERT(mm->stageMatchGroup(*mg1_1) == OK);
  CPPUNIT_ASSERT(mg1_1->getState() == STAT_MG_Staged);

  // make sure that the match group for round 2 has been promoted
  // from FROZEN to IDLE
  CPPUNIT_ASSERT(mg1_2->getState() == STAT_MG_Idle);
  CPPUNIT_ASSERT(mg1_2->getStageSequenceNumber() == -1);

  // test getMaxStageSeqNum for a non-empty staging area
  CPPUNIT_ASSERT(mm->getMaxStageSeqNum() == 1);

  // make sure the correct sequence number has been assigned
  TabRow r = (db->getTab(TabMatch_GROUP))[mg1_1->getId()];
  CPPUNIT_ASSERT(r[MG_StageSeqNum].toInt() == 1);
  CPPUNIT_ASSERT(mg1_1->getStageSequenceNumber() == 1);

  // stage the next round
  CPPUNIT_ASSERT(mm->stageMatchGroup(*mg1_2) == OK);
  CPPUNIT_ASSERT(mg1_2->getState() == STAT_MG_Staged);
  CPPUNIT_ASSERT(mm->getMaxStageSeqNum() == 2);
  r = (db->getTab(TabMatch_GROUP))[mg1_2->getId()];
  CPPUNIT_ASSERT(r[MG_StageSeqNum].toInt() == 2);
  CPPUNIT_ASSERT(mg1_2->getStageSequenceNumber() == 2);

  // make sure 3rd round is now IDLE
  auto mg1_3 = mm->getMatchGroup(ms, 3, 1, &e);  // players group 1, round 3
  CPPUNIT_ASSERT(mg1_3 != nullptr);
  CPPUNIT_ASSERT(e == OK);
  CPPUNIT_ASSERT(mg1_3->getState() == STAT_MG_Idle);
  CPPUNIT_ASSERT(mg1_3->getStageSequenceNumber() == -1);

  // add a match round of another players group
  auto mg2_1 = mm->getMatchGroup(ms, 1, 2, &e);  // players group 2, round 1
  CPPUNIT_ASSERT(mg2_1 != nullptr);
  CPPUNIT_ASSERT(e == OK);
  CPPUNIT_ASSERT(mg2_1->getState() == STAT_MG_Idle);
  CPPUNIT_ASSERT(mg2_1->getStageSequenceNumber() == -1);
  CPPUNIT_ASSERT(mm->stageMatchGroup(*mg2_1) == OK);
  CPPUNIT_ASSERT(mg2_1->getState() == STAT_MG_Staged);
  CPPUNIT_ASSERT(mg2_1->getStageSequenceNumber() == 3);

  // make sure the next match group has been promoted
  auto mg2_2 = mm->getMatchGroup(ms, 2, 2, &e);  // players group 2, round 2
  CPPUNIT_ASSERT(mg2_2 != nullptr);
  CPPUNIT_ASSERT(e == OK);
  CPPUNIT_ASSERT(mg2_2->getState() == STAT_MG_Idle);

  // now three match groups are staged
  CPPUNIT_ASSERT(mm->getMaxStageSeqNum() == 3);

  //
  // Intermediate status:
  //   * group 1, round 1: staged, seq num = 1
  //   * group 1, round 2: staged, seq num = 2
  //   * group 1, round 3: idle
  //   * group 1, round 4 & 5: frozen
  //   * group 2, round 1: staged, seq num = 3
  //   * group 2, round 2: idle
  //   * group 2, round 3...5: frozen
  //   * group 3...8, round 1: idle
  //   * group 3...8, round 2...5: frozen
  //


  //
  // now test unstaging
  //


  // "mg1_1 is round 1; since round 2 is staged, it should not
  // be unstageable
  CPPUNIT_ASSERT(mm->canUnstageMatchGroup(*mg1_1) == MatchGroupNotUnstageable);
  CPPUNIT_ASSERT(mm->unstageMatchGroup(*mg1_1)  == MatchGroupNotUnstageable);
  CPPUNIT_ASSERT(mg1_1->getState() == STAT_MG_Staged);
  CPPUNIT_ASSERT(mg1_1->getStageSequenceNumber() == 1);

  // mg1_2 is the last staged round of players group 2, so it
  // should be unstageable
  CPPUNIT_ASSERT(mm->canUnstageMatchGroup(*mg1_2) == OK);
  CPPUNIT_ASSERT(mm->unstageMatchGroup(*mg1_2)  == OK);
  CPPUNIT_ASSERT(mg1_2->getState() == STAT_MG_Idle);
  CPPUNIT_ASSERT(mg1_2->getStageSequenceNumber() == -1);

  // the sequence number of the follow-up match group
  // should have been adjusted from "3" to "2"
  CPPUNIT_ASSERT(mg2_1->getStageSequenceNumber() == 2);
  CPPUNIT_ASSERT(mm->getMaxStageSeqNum() == 2);

  // match group for round 1 remains untouched
  CPPUNIT_ASSERT(mg1_1->getState() == STAT_MG_Staged);
  CPPUNIT_ASSERT(mg1_1->getStageSequenceNumber() == 1);

  // match group for round 3 must have been demoted from IDLE to FROZEN
  CPPUNIT_ASSERT(mg1_3->getState() == STAT_MG_Frozen);

  // now unstage match group 1.1
  CPPUNIT_ASSERT(mm->canUnstageMatchGroup(*mg1_1) == OK);
  CPPUNIT_ASSERT(mm->unstageMatchGroup(*mg1_1)  == OK);
  CPPUNIT_ASSERT(mg1_1->getState() == STAT_MG_Idle);
  CPPUNIT_ASSERT(mg1_1->getStageSequenceNumber() == -1);

  // the sequence number of the follow-up match group
  // should have been adjusted from "3" to "2"
  CPPUNIT_ASSERT(mg2_1->getStageSequenceNumber() == 1);
  CPPUNIT_ASSERT(mm->getMaxStageSeqNum() == 1);

  // match group for round 3 must have been demoted from IDLE to FROZEN
  CPPUNIT_ASSERT(mg1_2->getState() == STAT_MG_Frozen);

  // finally unstage the 2.1 match group
  CPPUNIT_ASSERT(mm->canUnstageMatchGroup(*mg2_1) == OK);
  CPPUNIT_ASSERT(mm->unstageMatchGroup(*mg2_1)  == OK);
  CPPUNIT_ASSERT(mg2_1->getState() == STAT_MG_Idle);
  CPPUNIT_ASSERT(mg2_1->getStageSequenceNumber() == -1);
  CPPUNIT_ASSERT(mg2_2->getState() == STAT_MG_Frozen);
  CPPUNIT_ASSERT(mm->getMaxStageSeqNum() == 0);


  delete db;
  printEndMsg();
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
    
