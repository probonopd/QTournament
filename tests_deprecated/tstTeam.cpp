/* 
 * File:   tstGenericDBObject.cpp
 * Author: volker
 * 
 * Created on March 2, 2014, 3:46 PM
 */

#include <stdexcept>

#include "tstTeam.h"
#include "TournamentDB.h"
#include "TournamentDataDefs.h"
#include "TournamentErrorCodes.h"
#include "TeamMngr.h"
#include "Tournament.h"

using namespace QTournament;
using namespace dbOverlay;

//----------------------------------------------------------------------------

void tstTeam::testRename()
{
  printStartMsg("tstTeam::testRename");
  
  TournamentDB* db = getScenario01(true);
  Tournament t(getSqliteFileName());
  TeamMngr* tmngr = t.getTeamMngr();
  
  // actually create a valid team
  CPPUNIT_ASSERT(tmngr->createNewTeam("t1") == OK);
  CPPUNIT_ASSERT(tmngr->createNewTeam("t2") == OK);
  
  // get t2
  Team t2 = tmngr->getTeam("t2");
  
  // try to rename to an invalid name
  CPPUNIT_ASSERT(t2.rename("") == INVALID_NAME);
  CPPUNIT_ASSERT(t2.rename(" ") == INVALID_NAME);
  CPPUNIT_ASSERT(t2.rename(QString::null) == INVALID_NAME);
  
  // try to rename to an existing name
  CPPUNIT_ASSERT(t2.rename("t1") == NAME_EXISTS);
  
  // valid rename
  CPPUNIT_ASSERT(t2.rename("xx") == OK);
  CPPUNIT_ASSERT(t2.getName() == "xx");
  
  delete db;
  printEndMsg();
}

//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    
