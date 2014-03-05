/* 
 * File:   tstGenericDBObject.cpp
 * Author: volker
 * 
 * Created on March 2, 2014, 3:46 PM
 */

#include <stdexcept>

#include "tstTeamMngr.h"
#include "BasicTestClass.h"
#include "GenericDatabaseObject.h"
#include "TournamentDB.h"
#include "TournamentDataDefs.h"
#include "TeamMngr.h"
#include "TournamentErrorCodes.h"

using namespace QTournament;
using namespace dbOverlay;

//----------------------------------------------------------------------------

void tstTeamMngr::testCreateNewTeam()
{
  printStartMsg("tstTeamMngr::testCreateNewTeam");
  
  TournamentDB db = getScenario01(true);
  TeamMngr tmngr(db);
  
  // try empty or invalid name
  CPPUNIT_ASSERT(tmngr.createNewTeam("") == INVALID_NAME);
  CPPUNIT_ASSERT(tmngr.createNewTeam(QString::null) == INVALID_NAME);
  CPPUNIT_ASSERT(db[TAB_TEAM].length() == 0);
  
  // actually create a valid team
  CPPUNIT_ASSERT(tmngr.createNewTeam("t1") == OK);
  CPPUNIT_ASSERT(db[TAB_TEAM].length() == 1);
  TabRow r = db[TAB_TEAM][1];
  CPPUNIT_ASSERT(r[GENERIC_NAME_FIELD_NAME].toString() == "t1");
  
  // name collision
  CPPUNIT_ASSERT(tmngr.createNewTeam("t1") == NAME_EXISTS);
  CPPUNIT_ASSERT(db[TAB_TEAM].length() == 1);
  
  printEndMsg();
}

//----------------------------------------------------------------------------

void tstTeamMngr::testCreateNewTeam2()
{
  printStartMsg("tstTeamMngr::testCreateNewTeam2");
  
  // !! start a tournament that doesn't use teams !!
  TournamentDB db = getScenario01(false);
  TeamMngr tmngr(db);
  
  // try empty or invalid name
  CPPUNIT_ASSERT(tmngr.createNewTeam("") == NOT_USING_TEAMS);
  CPPUNIT_ASSERT(tmngr.createNewTeam(QString::null) == NOT_USING_TEAMS);
  CPPUNIT_ASSERT(db[TAB_TEAM].length() == 0);
  
  // actually create a valid team
  CPPUNIT_ASSERT(tmngr.createNewTeam("t1") == NOT_USING_TEAMS);
  CPPUNIT_ASSERT(db[TAB_TEAM].length() == 0);
  
  printEndMsg();
}

//----------------------------------------------------------------------------

void tstTeamMngr::testHasTeam()
{
  printStartMsg("tstTeamMngr::testHasTeam");
  
  TournamentDB db = getScenario01(true);
  TeamMngr tmngr(db);
  
  // try queries on empty table
  CPPUNIT_ASSERT(tmngr.hasTeam("") == false);
  CPPUNIT_ASSERT(tmngr.hasTeam(QString::null) == false);
  CPPUNIT_ASSERT(tmngr.hasTeam("abc") == false);
  
  // actually create a valid team
  CPPUNIT_ASSERT(tmngr.createNewTeam("t1") == OK);
  
  // try queries on filled table
  CPPUNIT_ASSERT(tmngr.hasTeam("") == false);
  CPPUNIT_ASSERT(tmngr.hasTeam(QString::null) == false);
  CPPUNIT_ASSERT(tmngr.hasTeam("abc") == false);
  CPPUNIT_ASSERT(tmngr.hasTeam("t1") == true);
  
  printEndMsg();
}

//----------------------------------------------------------------------------

void tstTeamMngr::testGetTeam()
{
  printStartMsg("tstTeamMngr::testGetTeam");
  
  TournamentDB db = getScenario01(true);
  TeamMngr tmngr(db);
  
  // actually create a valid team
  CPPUNIT_ASSERT(tmngr.createNewTeam("t1") == OK);
  CPPUNIT_ASSERT(tmngr.createNewTeam("t2") == OK);
  
  // try queries on filled table
  CPPUNIT_ASSERT_THROW(tmngr.getTeam(""), std::invalid_argument);
  CPPUNIT_ASSERT_THROW(tmngr.getTeam(QString::null), std::invalid_argument);
  CPPUNIT_ASSERT_THROW(tmngr.getTeam("dflsjdf"), std::invalid_argument);
  Team t = tmngr.getTeam("t2");
  CPPUNIT_ASSERT(t.getId() == 2);
  CPPUNIT_ASSERT(t.getName() == "t2");
  
  printEndMsg();
}

//----------------------------------------------------------------------------
    
void tstTeamMngr::testGetAllTeams()
{
  printStartMsg("tstTeamMngr::testGetAllTeams");
  
  TournamentDB db = getScenario01(true);
  TeamMngr tmngr(db);
  
  // run on empty table
  QList<Team> result = tmngr.getAllTeams();
  CPPUNIT_ASSERT(result.length() == 0);
  
  // actually create a valid team
  CPPUNIT_ASSERT(tmngr.createNewTeam("t1") == OK);
  CPPUNIT_ASSERT(tmngr.createNewTeam("t2") == OK);
  
  // run on filled table
  result = tmngr.getAllTeams();
  CPPUNIT_ASSERT(result.length() == 2);
  Team t = result.at(0);
  CPPUNIT_ASSERT(t.getId() == 1);
  CPPUNIT_ASSERT(t.getName() == "t1");
  t = result.at(1);
  CPPUNIT_ASSERT(t.getId() == 2);
  CPPUNIT_ASSERT(t.getName() == "t2");
  
  
  printEndMsg();
}

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    
