/* 
 * File:   tstGenericDBObject.cpp
 * Author: volker
 * 
 * Created on March 2, 2014, 3:46 PM
 */

#include <stdexcept>

#include "tstCategory.h"
#include "BasicTestClass.h"
#include "GenericDatabaseObject.h"
#include "TournamentDB.h"
#include "TournamentDataDefs.h"
#include "Category.h"
#include "TournamentErrorCodes.h"
#include "Tournament.h"
#include "tstPlayerMngr.h"

using namespace QTournament;
using namespace dbOverlay;

//----------------------------------------------------------------------------

void tstCategory::testIsPlayerSuitable()
{
  printStartMsg("tstCategory::testIsPlayerSuitable");
  
  TournamentDB db = getScenario02(true);
  Tournament t(getSqliteFileName());
  
  // create a team some dummy players
  Player m1 = Tournament::getPlayerMngr()->getPlayer("f", "l1");
  Player f1 = Tournament::getPlayerMngr()->getPlayer("f", "l2");
  
  // create one category of every kind
  CatMngr* cmngr = Tournament::getCatMngr();
  Category ms = cmngr->getCategory("MS");
  Category md = cmngr->getCategory("MD");
  Category ls = cmngr->getCategory("LS");
  Category ld = cmngr->getCategory("LD");
  Category mx = cmngr->getCategory("MX");
  
  // check player suitability for men's single
  CPPUNIT_ASSERT(ms.isPlayerSuitable(m1) == true);
  CPPUNIT_ASSERT(ms.isPlayerSuitable(f1) == false);
  ms.setSex(DONT_CARE);   // disable checking
  CPPUNIT_ASSERT(ms.isPlayerSuitable(m1) == true);
  CPPUNIT_ASSERT(ms.isPlayerSuitable(f1) == true);
  
  // check player suitability for men's doubles
  CPPUNIT_ASSERT(md.isPlayerSuitable(m1) == true);
  CPPUNIT_ASSERT(md.isPlayerSuitable(f1) == false);
  md.setSex(DONT_CARE);   // disable checking
  CPPUNIT_ASSERT(md.isPlayerSuitable(m1) == true);
  CPPUNIT_ASSERT(md.isPlayerSuitable(f1) == true);
  
  // check player suitability for ladies' single
  CPPUNIT_ASSERT(ls.isPlayerSuitable(m1) == false);
  CPPUNIT_ASSERT(ls.isPlayerSuitable(f1) == true);
  ls.setSex(DONT_CARE);   // disable checking
  CPPUNIT_ASSERT(ls.isPlayerSuitable(m1) == true);
  CPPUNIT_ASSERT(ls.isPlayerSuitable(f1) == true);
  
  // check player suitability for ladies' doubles
  CPPUNIT_ASSERT(ld.isPlayerSuitable(m1) == false);
  CPPUNIT_ASSERT(ld.isPlayerSuitable(f1) == true);
  ld.setSex(DONT_CARE);   // disable checking
  CPPUNIT_ASSERT(ld.isPlayerSuitable(m1) == true);
  CPPUNIT_ASSERT(ld.isPlayerSuitable(f1) == true);
  
  // check player suitability for mixed doubles
  CPPUNIT_ASSERT(mx.isPlayerSuitable(m1) == true);
  CPPUNIT_ASSERT(mx.isPlayerSuitable(f1) == true);
  mx.setSex(DONT_CARE);   // disable checking
  CPPUNIT_ASSERT(mx.isPlayerSuitable(m1) == true);
  CPPUNIT_ASSERT(mx.isPlayerSuitable(f1) == true);
  
  
  printEndMsg();
}

//----------------------------------------------------------------------------

void tstCategory::testHasPlayer()
{
  printStartMsg("tstCategory::testHasPlayer");
  
  TournamentDB db = getScenario02(true);
  Tournament t(getSqliteFileName());
  
  // create a team some dummy players
  Player m1 = Tournament::getPlayerMngr()->getPlayer("f", "l1");
  Player f1 = Tournament::getPlayerMngr()->getPlayer("f", "l2");
  Player m2 = Tournament::getPlayerMngr()->getPlayer("f", "l3");
  Player f2 = Tournament::getPlayerMngr()->getPlayer("f", "l4");
  
  // create a category
  CatMngr* cmngr = Tournament::getCatMngr();
  Category ms = cmngr->getCategory("MS");
  
  // query empty category
  CPPUNIT_ASSERT(ms.hasPlayer(m1) == false);
  
  // add a player
  CPPUNIT_ASSERT(cmngr->addPlayerToCategory(m1, ms) == OK);
  
  // query filled category
  CPPUNIT_ASSERT(ms.hasPlayer(m1) == true);
  CPPUNIT_ASSERT(ms.hasPlayer(f1) == false);
  CPPUNIT_ASSERT(ms.hasPlayer(m2) == false);
  CPPUNIT_ASSERT(ms.hasPlayer(f2) == false);
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
    
