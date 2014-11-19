/* 
 * File:   tstGenericDBObject.cpp
 * Author: volker
 * 
 * Created on March 2, 2014, 3:46 PM
 */

#include <stdexcept>

#include "tstMatch.h"
#include "TournamentDB.h"
#include "TournamentDataDefs.h"
#include "TournamentErrorCodes.h"
#include "Tournament.h"

using namespace QTournament;
using namespace dbOverlay;

//----------------------------------------------------------------------------

void tstMatch::testGetCat()
{
  printStartMsg("tstMatch::testGetCat");

  TournamentDB db = getScenario02(true);
  Tournament t(getSqliteFileName());
  Category ms = Tournament::getCatMngr()->getCategoryById(1);
  auto ma = initMatchGroup();

  CPPUNIT_ASSERT(ma->getCategory() == ms);

  printEndMsg();
}

//----------------------------------------------------------------------------

unique_ptr<Match> tstMatch::initMatchGroup()
{
  MatchMngr* mm = Tournament::getMatchMngr();
  Category ms = Tournament::getCatMngr()->getCategoryById(1);
  TournamentDB db{getSqliteFileName(), false};

  // fake a valid category state
  TabRow catRow = db[TAB_CATEGORY][1];
  catRow.update(GENERIC_STATE_FIELD_NAME, static_cast<int>(STAT_CAT_IDLE));
  CPPUNIT_ASSERT(ms.getState() == STAT_CAT_IDLE);

  // create a new match group
  ERR e;
  auto grp = mm->createMatchGroup(ms, 1, 2, &e);
  CPPUNIT_ASSERT(e == OK);
  CPPUNIT_ASSERT(grp != nullptr);

  grp = mm->createMatchGroup(ms, 2, 3, &e);
  CPPUNIT_ASSERT(e == OK);
  CPPUNIT_ASSERT(grp != nullptr);

  // create a new match in this group
  auto ma = mm->createMatch(*grp, &e);
  CPPUNIT_ASSERT(e == OK);
  CPPUNIT_ASSERT(ma != nullptr);

  return ma;
}

//----------------------------------------------------------------------------
    
void tstMatch::testGetGroup()
{
  printStartMsg("tstMatch::testGetGroup");

  TournamentDB db = getScenario02(true);
  Tournament t(getSqliteFileName());
  auto ma = initMatchGroup();

  MatchMngr* mm = Tournament::getMatchMngr();
  Category ms = Tournament::getCatMngr()->getCategoryById(1);

  ERR e;
  auto mg = mm->getMatchGroup(ms, 2, 3, &e);
  CPPUNIT_ASSERT(e == OK);
  CPPUNIT_ASSERT(mg != nullptr);

  CPPUNIT_ASSERT(ma->getMatchGroup() == *mg);

  // also make a "negative test", for the paranoid
  mg = mm->getMatchGroup(ms, 1, 2, &e);
  CPPUNIT_ASSERT(e == OK);
  CPPUNIT_ASSERT(mg != nullptr);

  CPPUNIT_ASSERT(ma->getMatchGroup() != *mg);

  printEndMsg();
}

//----------------------------------------------------------------------------

void tstMatch::testGetPairs()
{
  printStartMsg("tstMatch::testGetGroup");

  TournamentDB db = getScenario04(true);
  Tournament t(getSqliteFileName());
  MatchMngr* mm = Tournament::getMatchMngr();
  Category mx = Tournament::getCatMngr()->getCategoryById(5);

  ERR e;
  auto mg = mm->getMatchGroup(mx, 1, 1, &e);
  CPPUNIT_ASSERT(e == OK);
  CPPUNIT_ASSERT(mg != nullptr);

  auto mgl = mg->getMatches();
  CPPUNIT_ASSERT(mgl.count() == 1);
  Match ma = mgl.at(0);

  // test the getPair-functions
  CPPUNIT_ASSERT(ma.hasPlayerPair1() == false);
  CPPUNIT_ASSERT(ma.hasPlayerPair2() == false);
  CPPUNIT_ASSERT_THROW(ma.getPlayerPair1(), std::runtime_error);
  CPPUNIT_ASSERT_THROW(ma.getPlayerPair2(), std::runtime_error);

  // (artificially) assign one pair
  PlayerPair pp1 = Tournament::getPlayerMngr()->getPlayerPair(1);
  db[TAB_MATCH][ma.getId()].update(MA_PAIR1_REF, pp1.getPairId());

  // test again
  CPPUNIT_ASSERT(ma.hasPlayerPair1() == true);
  CPPUNIT_ASSERT(ma.hasPlayerPair2() == false);
  CPPUNIT_ASSERT(ma.getPlayerPair1().getPairId() == pp1.getPairId());
  CPPUNIT_ASSERT_THROW(ma.getPlayerPair2(), std::runtime_error);

  // (artificially) assign the second pair
  PlayerPair pp2 = Tournament::getPlayerMngr()->getPlayerPair(2);
  db[TAB_MATCH][ma.getId()].update(MA_PAIR2_REF, pp2.getPairId());

  // test again
  CPPUNIT_ASSERT(ma.hasPlayerPair1() == true);
  CPPUNIT_ASSERT(ma.hasPlayerPair2() == true);
  CPPUNIT_ASSERT(ma.getPlayerPair1().getPairId() == pp1.getPairId());
  CPPUNIT_ASSERT(ma.getPlayerPair2().getPairId() == pp2.getPairId());

  // remove the first one
  db[TAB_MATCH][ma.getId()].update(MA_PAIR1_REF, QVariant());

  // test again
  CPPUNIT_ASSERT(ma.hasPlayerPair1() == false);
  CPPUNIT_ASSERT(ma.hasPlayerPair2() == true);
  CPPUNIT_ASSERT_THROW(ma.getPlayerPair1(), std::runtime_error);
  CPPUNIT_ASSERT(ma.getPlayerPair2().getPairId() == pp2.getPairId());

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


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
    
