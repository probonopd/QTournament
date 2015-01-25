/* 
 * File:   tstGenericDBObject.cpp
 * Author: volker
 * 
 * Created on March 2, 2014, 3:46 PM
 */

#include <stdexcept>

#include "tstGenericDBObject.h"
#include "BasicTestClass.h"
#include "GenericDatabaseObject.h"
#include "TournamentDB.h"
#include "TournamentDataDefs.h"
#include "DbTab.h"

using namespace QTournament;
using namespace dbOverlay;

//----------------------------------------------------------------------------

void tstGenericDBObject::testStateSetGet()
{
  printStartMsg("tstGenericDBObject::testStateSetGet");
  
  // initialize a database
  TournamentDB* db = getScenario01(true);

  // fake a player to get a "state" column to play with
  (*db)[TAB_PLAYER].insertRow();
  
  // construct a GenericDatabaseObject from this player
  TabRow r = (*db)[TAB_PLAYER][1];
  GenericDatabaseObject gdo(db, r);
  
  // set a state to initially fill the column
  gdo.setState(STAT_PL_IDLE);
  
  // get and change the state
  CPPUNIT_ASSERT(gdo.getState() == STAT_PL_IDLE);
  gdo.setState(STAT_CAT_CONFIG);
  CPPUNIT_ASSERT(gdo.getState() == STAT_CAT_CONFIG);

  delete db;
  
  printEndMsg();
}

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    
