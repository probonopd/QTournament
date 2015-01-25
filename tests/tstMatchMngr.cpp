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
  CPPUNIT_ASSERT(ms.getState() == STAT_CAT_CONFIG);
  auto mg = mm->createMatchGroup(ms, 2, 3, &e);
  CPPUNIT_ASSERT(e == CATEGORY_STILL_CONFIGURABLE);
  CPPUNIT_ASSERT(mg == nullptr);
  TabRow catRow = (*db)[TAB_CATEGORY][1];
  catRow.update(GENERIC_STATE_FIELD_NAME, static_cast<int>(STAT_CAT_FROZEN));
  CPPUNIT_ASSERT(ms.getState() == STAT_CAT_FROZEN);
  mg = mm->createMatchGroup(ms, 2, 3, &e);
  CPPUNIT_ASSERT(e == CATEGORY_STILL_CONFIGURABLE);
  CPPUNIT_ASSERT(mg == nullptr);

  // fake a valid category state
  catRow.update(GENERIC_STATE_FIELD_NAME, static_cast<int>(STAT_CAT_IDLE));
  CPPUNIT_ASSERT(ms.getState() == STAT_CAT_IDLE);

  // try empty or invalid parameters
  mg = mm->createMatchGroup(ms, -5, 1, &e);
  CPPUNIT_ASSERT(e == INVALID_ROUND);
  CPPUNIT_ASSERT(mg == nullptr);

  mg = mm->createMatchGroup(ms, 0, 1, &e);
  CPPUNIT_ASSERT(e == INVALID_ROUND);
  CPPUNIT_ASSERT(mg == nullptr);

  mg = mm->createMatchGroup(ms, 1, -42, &e);
  CPPUNIT_ASSERT(e == INVALID_GROUP_NUM);
  CPPUNIT_ASSERT(mg == nullptr);

  mg = mm->createMatchGroup(ms, 1, 0, &e);
  CPPUNIT_ASSERT(e == INVALID_GROUP_NUM);
  CPPUNIT_ASSERT(mg == nullptr);
  
  // make sure nothing has been inserted so far
  CPPUNIT_ASSERT((*db)[TAB_MATCH_GROUP].length() == 0);
  
  // create a valid group with a real (positive) group number
  mg = mm->createMatchGroup(ms, 2, 3, &e);
  CPPUNIT_ASSERT(e == OK);
  CPPUNIT_ASSERT(mg != nullptr);
  
  // create the same group twice
  auto mg2 = mm->createMatchGroup(ms, 2, 3, &e);
  CPPUNIT_ASSERT(e == MATCH_GROUP_EXISTS);
  CPPUNIT_ASSERT(mg2 == nullptr);
  
  // check database entries
  CPPUNIT_ASSERT((*db)[TAB_MATCH_GROUP].length() == 1);
  CPPUNIT_ASSERT((*db)[TAB_MATCH_GROUP][1][MG_CAT_REF].toInt() == 1);
  CPPUNIT_ASSERT((*db)[TAB_MATCH_GROUP][1][MG_ROUND].toInt() == 2);
  CPPUNIT_ASSERT((*db)[TAB_MATCH_GROUP][1][MG_GRP_NUM].toInt() == 3);
  CPPUNIT_ASSERT(mg->getState() == STAT_MG_CONFIG);
  
  // create a match group with a special value for the group number
  mg2 = mm->createMatchGroup(ms, 4, GROUP_NUM__L16, &e);
  CPPUNIT_ASSERT(e == OK);
  CPPUNIT_ASSERT(mg2 != 0);
  CPPUNIT_ASSERT((*db)[TAB_MATCH_GROUP].length() == 2);
  CPPUNIT_ASSERT((*db)[TAB_MATCH_GROUP][2][MG_CAT_REF].toInt() == 1);
  CPPUNIT_ASSERT((*db)[TAB_MATCH_GROUP][2][MG_ROUND].toInt() == 4);
  CPPUNIT_ASSERT((*db)[TAB_MATCH_GROUP][2][MG_GRP_NUM].toInt() == -8);
  CPPUNIT_ASSERT(mg2->getState() == STAT_MG_CONFIG);
  
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
  CPPUNIT_ASSERT(e == INVALID_ROUND);

  CPPUNIT_ASSERT(mm->hasMatchGroup(ms, 0, 1, &e) == false);
  CPPUNIT_ASSERT(e == INVALID_ROUND);

  CPPUNIT_ASSERT(mm->hasMatchGroup(ms, 1, -42, &e) == false);
  CPPUNIT_ASSERT(e == INVALID_GROUP_NUM);

  CPPUNIT_ASSERT(mm->hasMatchGroup(ms, 1, 0, &e) == false);
  CPPUNIT_ASSERT(e == INVALID_GROUP_NUM);
  
  // try empty or invalid parameters without
  // pointer to an error code
  CPPUNIT_ASSERT(mm->hasMatchGroup(ms, -5, 1) == false);
  CPPUNIT_ASSERT(mm->hasMatchGroup(ms, 0, 1) == false);
  CPPUNIT_ASSERT(mm->hasMatchGroup(ms, 1, -42) == false);
  CPPUNIT_ASSERT(mm->hasMatchGroup(ms, 1, 0) == false);
  
  // fake a valid category state
  TabRow catRow = (*db)[TAB_CATEGORY][1];
  catRow.update(GENERIC_STATE_FIELD_NAME, static_cast<int>(STAT_CAT_IDLE));
  CPPUNIT_ASSERT(ms.getState() == STAT_CAT_IDLE);

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
  CPPUNIT_ASSERT(e == NO_SUCH_MATCH_GROUP);
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
  TabRow catRow = (*db)[TAB_CATEGORY][1];
  catRow.update(GENERIC_STATE_FIELD_NAME, static_cast<int>(STAT_CAT_IDLE));
  CPPUNIT_ASSERT(ms.getState() == STAT_CAT_IDLE);

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
  CPPUNIT_ASSERT(e == NO_SUCH_MATCH_GROUP);
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
  TabRow catRow = (*db)[TAB_CATEGORY][1];
  catRow.update(GENERIC_STATE_FIELD_NAME, static_cast<int>(STAT_CAT_IDLE));
  CPPUNIT_ASSERT(ms.getState() == STAT_CAT_IDLE);

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
  CPPUNIT_ASSERT((*db)[TAB_MATCH].length() == 1);
  CPPUNIT_ASSERT((*db)[TAB_MATCH][1][MA_GRP_REF].toInt() == 1);
  CPPUNIT_ASSERT(ma->getState() == STAT_MA_INCOMPLETE);

  // fake a new match group state
  (*db)[TAB_MATCH_GROUP][1].update(GENERIC_STATE_FIELD_NAME, static_cast<int>(STAT_MG_FROZEN));
  CPPUNIT_ASSERT(grp->getState() == STAT_MG_FROZEN);

  // try to create a new match in a frozen match group
  auto ma2 = mm->createMatch(*grp, &e);
  CPPUNIT_ASSERT(e == MATCH_GROUP_NOT_CONFIGURALE_ANYMORE);
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
  auto tabPairs = db->getTab(TAB_PAIRS);
  auto catMngr = Tournament::getCatMngr();
  Player m1 = Tournament::getPlayerMngr()->getPlayer("f", "l1");
  Player f1 = Tournament::getPlayerMngr()->getPlayer("f", "l2");
  Player m2 = Tournament::getPlayerMngr()->getPlayer("f", "l3");
  Player m3 = Tournament::getPlayerMngr()->getPlayer("f", "l5");
  CPPUNIT_ASSERT(catMngr->pairPlayers(mx, m1, f1) == OK);
  PlayerPair mxPair{m1, f1, 1};
  CPPUNIT_ASSERT(mxPair.isConsistent(db));

  // leave out PAIRS_PLAYER2_REF to assign a NULL value in the database
  QVariantList qvl;
  qvl << PAIRS_CAT_REF << 1; // ms
  qvl << PAIRS_GRP_NUM << 3;
  qvl << PAIRS_PLAYER1_REF << m1.getId();
  tabPairs.insertRow(qvl);
  PlayerPair msPair1{m1, 2};
  CPPUNIT_ASSERT(msPair1.isConsistent(db));

  qvl.clear();
  qvl << PAIRS_CAT_REF << 1; // ms
  qvl << PAIRS_GRP_NUM << GRP_NUM__NOT_ASSIGNED;
  qvl << PAIRS_PLAYER1_REF << m2.getId();
  tabPairs.insertRow(qvl);
  PlayerPair msPair2{m2, 3};
  CPPUNIT_ASSERT(msPair2.isConsistent(db));

  qvl.clear();
  qvl << PAIRS_CAT_REF << 1; // ms
  qvl << PAIRS_GRP_NUM << 3;
  qvl << PAIRS_PLAYER1_REF << m3.getId();
  tabPairs.insertRow(qvl);
  PlayerPair msPair3{m3, 4};
  CPPUNIT_ASSERT(msPair3.isConsistent(db));

  qvl.clear();
  qvl << PAIRS_CAT_REF << 3; // ls
  qvl << PAIRS_GRP_NUM << GRP_NUM__NOT_ASSIGNED;
  qvl << PAIRS_PLAYER1_REF << f1.getId();
  tabPairs.insertRow(qvl);
  PlayerPair lsPair1{f1, 5};
  CPPUNIT_ASSERT(lsPair1.isConsistent(db));

  qvl.clear();
  qvl << PAIRS_CAT_REF << 3; // ls
  qvl << PAIRS_GRP_NUM << GRP_NUM__NOT_ASSIGNED;
  qvl << PAIRS_PLAYER1_REF << m1.getId();  // semantically wrong (male in lady singles), here only for testing
  tabPairs.insertRow(qvl);
  PlayerPair weirdPair{m1, 6};
  CPPUNIT_ASSERT(weirdPair.isConsistent(db));

  // fake a valid category state
  TabRow catRow = (*db)[TAB_CATEGORY][1];
  catRow.update(GENERIC_STATE_FIELD_NAME, static_cast<int>(STAT_CAT_IDLE));
  CPPUNIT_ASSERT(ms.getState() == STAT_CAT_IDLE);

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
  auto tabMatch = db->getTab(TAB_MATCH);
  for (auto stat : {STAT_MA_READY, STAT_MA_BUSY, STAT_MA_FINISHED, STAT_MA_FUZZY, STAT_MA_POSTPONED, STAT_MA_READY, STAT_MA_RUNNING})
  {
    tabMatch[m.getId()].update(GENERIC_STATE_FIELD_NAME, static_cast<int>(stat));
    CPPUNIT_ASSERT(m.getState() == stat);
    CPPUNIT_ASSERT(mm->canAssignPlayerPairToMatch(m, msPair1) == MATCH_NOT_CONFIGURALE_ANYMORE);
  }

  // reset match state
  tabMatch[m.getId()].update(GENERIC_STATE_FIELD_NAME, static_cast<int>(STAT_MA_INCOMPLETE));
  CPPUNIT_ASSERT(m.getState() == STAT_MA_INCOMPLETE);

  // try to assign a player pair from the wrong category
  CPPUNIT_ASSERT(mm->canAssignPlayerPairToMatch(m, lsPair1) == PLAYER_NOT_IN_CATEGORY);
  CPPUNIT_ASSERT(mm->canAssignPlayerPairToMatch(m, weirdPair) == PLAYER_NOT_IN_CATEGORY);  // single male, also in MS, but this pair is not in MS

  // "m" is a match in round 3, group 1
  //
  // try to assign a pair with "unassigned group number" or with a group number != 1
  CPPUNIT_ASSERT(mm->canAssignPlayerPairToMatch(m, msPair2) == GROUP_NUMBER_MISMATCH);  // unassigned number for pair2
  CPPUNIT_ASSERT(mm->canAssignPlayerPairToMatch(m, msPair1) == GROUP_NUMBER_MISMATCH);  // group num 3 for pair 1

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
  CPPUNIT_ASSERT(matchRow[MA_PAIR1_REF].toInt() == msPair1.getPairId());
  CPPUNIT_ASSERT(matchRow[MA_PAIR2_REF].toInt() == msPair3.getPairId());

  // try to use the same pair in the same round and group twice
  CPPUNIT_ASSERT(mm->canAssignPlayerPairToMatch(ma2, msPair1) == PLAYER_ALREADY_ASSIGNED_TO_OTHER_MATCH_IN_THE_SAME_ROUND_AND_CATEGORY);
  CPPUNIT_ASSERT(mm->canAssignPlayerPairToMatch(ma2, msPair3) == PLAYER_ALREADY_ASSIGNED_TO_OTHER_MATCH_IN_THE_SAME_ROUND_AND_CATEGORY);

  // try to use the same pair in the same round but a different group
  CPPUNIT_ASSERT(mm->canAssignPlayerPairToMatch(ma3, msPair1) == GROUP_NUMBER_MISMATCH);
  CPPUNIT_ASSERT(mm->canAssignPlayerPairToMatch(ma3, msPair3) == GROUP_NUMBER_MISMATCH);

  // re-assigning the player to the same match should work
  CPPUNIT_ASSERT(mm->canAssignPlayerPairToMatch(m, msPair1) == OK);
  CPPUNIT_ASSERT(mm->canAssignPlayerPairToMatch(m, msPair3) == OK);
  CPPUNIT_ASSERT(mm->setPlayerPairsForMatch(m, msPair1, msPair3) == OK);

  // assigning the same player pair to the same match for both opponent roles must fail
  CPPUNIT_ASSERT(mm->setPlayerPairsForMatch(m, msPair1, msPair1) == PLAYERS_IDENTICAL);

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
    

//----------------------------------------------------------------------------
    
