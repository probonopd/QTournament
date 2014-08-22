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
  
  TournamentDB db = getScenario02(true);
  Tournament t(getSqliteFileName());
  MatchMngr* mm = Tournament::getMatchMngr();
  
  // try empty or invalid parameters
  Category ms = Tournament::getCatMngr()->getCategoryById(1);
  MatchGroup* g = (MatchGroup*) 1;
  CPPUNIT_ASSERT(mm->createMatchGroup(ms, -5, 1, &g) == INVALID_ROUND);
  CPPUNIT_ASSERT(g == 0);
  g = (MatchGroup*) 1;
  CPPUNIT_ASSERT(mm->createMatchGroup(ms, 0, 1, &g) == INVALID_ROUND);
  CPPUNIT_ASSERT(g == 0);
  g = (MatchGroup*) 1;
  CPPUNIT_ASSERT(mm->createMatchGroup(ms, 1, -42, &g) == INVALID_GROUP_NUM);
  CPPUNIT_ASSERT(g == 0);
  g = (MatchGroup*) 1;
  CPPUNIT_ASSERT(mm->createMatchGroup(ms, 1, 0, &g) == INVALID_GROUP_NUM);
  CPPUNIT_ASSERT(g == 0);
  
  // make sure nothing has been inserted so far
  CPPUNIT_ASSERT(db[TAB_MATCH_GROUP].length() == 0);
  
  // create a valid group with a real (positive) group number
  CPPUNIT_ASSERT(mm->createMatchGroup(ms, 2, 3, &g) == OK);
  CPPUNIT_ASSERT(g != 0);
  
  // create the same group twice
  MatchGroup* g2 = (MatchGroup*) 1;
  CPPUNIT_ASSERT(mm->createMatchGroup(ms, 2, 3, &g2) == MATCH_GROUP_EXISTS);
  CPPUNIT_ASSERT(g2 == 0);
  
  // check database entries
  CPPUNIT_ASSERT(db[TAB_MATCH_GROUP].length() == 1);
  CPPUNIT_ASSERT(db[TAB_MATCH_GROUP][1][MG_CAT_REF].toInt() == 1);
  CPPUNIT_ASSERT(db[TAB_MATCH_GROUP][1][MG_ROUND].toInt() == 2);
  CPPUNIT_ASSERT(db[TAB_MATCH_GROUP][1][MG_GRP_NUM].toInt() == 3);
  
  // create a match group with a special value for the group number
  CPPUNIT_ASSERT(mm->createMatchGroup(ms, 4, GROUP_NUM__L16, &g2) == OK);
  CPPUNIT_ASSERT(g2 != 0);
  CPPUNIT_ASSERT(db[TAB_MATCH_GROUP].length() == 2);
  CPPUNIT_ASSERT(db[TAB_MATCH_GROUP][2][MG_CAT_REF].toInt() == 1);
  CPPUNIT_ASSERT(db[TAB_MATCH_GROUP][2][MG_ROUND].toInt() == 4);
  CPPUNIT_ASSERT(db[TAB_MATCH_GROUP][2][MG_GRP_NUM].toInt() == -8);
  
  // clean-up
  delete g;
  delete g2;
  
  printEndMsg();
}

//----------------------------------------------------------------------------

void tstMatchMngr::testHasGroup()
{
  printStartMsg("tstMatchMngr::testHasGroup");
  
  TournamentDB db = getScenario02(true);
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
  
  // create a valid group with a real (positive) group number
  MatchGroup* g;
  CPPUNIT_ASSERT(mm->createMatchGroup(ms, 2, 3, &g) == OK);
  CPPUNIT_ASSERT(g != 0);
  
  // look up this group
  CPPUNIT_ASSERT(mm->hasMatchGroup(ms, 2, 3, &e) == true);
  CPPUNIT_ASSERT(e == OK);
  CPPUNIT_ASSERT(mm->hasMatchGroup(ms, 2, 3) == true);
  
  // look up a non-existing group with valid parameters
  CPPUNIT_ASSERT(mm->hasMatchGroup(ms, 3, 3, &e) == false);
  CPPUNIT_ASSERT(e == NO_SUCH_MATCH_GROUP);
  CPPUNIT_ASSERT(mm->hasMatchGroup(ms, 3, 3) == false);
  
  printEndMsg();
}

//----------------------------------------------------------------------------
    
void tstMatchMngr::testGetGroup()
{
  printStartMsg("tstMatchMngr::testHasGroup");
  
  TournamentDB db = getScenario02(true);
  Tournament t(getSqliteFileName());
  MatchMngr* mm = Tournament::getMatchMngr();
  Category ms = Tournament::getCatMngr()->getCategoryById(1);
  
  // don't repeat all the test for INVALID_XXX here, because internally
  // it's the same code for hasMatchGroup (see above) and getMatchGroup()
  // that's being executed for the checks
  
  // so we only check here whether the right object is returned
  
  // create a valid group with a real (positive) group number
  MatchGroup* g;
  CPPUNIT_ASSERT(mm->createMatchGroup(ms, 2, 3, &g) == OK);
  CPPUNIT_ASSERT(g != 0);

  MatchGroup* g2;
  CPPUNIT_ASSERT(mm->getMatchGroup(ms, 2, 3, &g2) == OK);
  CPPUNIT_ASSERT(g2 != 0);
  CPPUNIT_ASSERT((*g2) == (*g));
  delete g2;
  delete g;
  
  CPPUNIT_ASSERT(g2 != 0);
  CPPUNIT_ASSERT(mm->getMatchGroup(ms, 3, 3, &g2) == NO_SUCH_MATCH_GROUP);
  CPPUNIT_ASSERT(g2 == 0);
  
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
    
