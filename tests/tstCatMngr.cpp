/* 
 * File:   tstGenericDBObject.cpp
 * Author: volker
 * 
 * Created on March 2, 2014, 3:46 PM
 */

#include <stdexcept>

#include "tstCatMngr.h"
#include "BasicTestClass.h"
#include "GenericDatabaseObject.h"
#include "TournamentDB.h"
#include "TournamentDataDefs.h"
#include "CatMngr.h"
#include "TournamentErrorCodes.h"

using namespace QTournament;
using namespace dbOverlay;

//----------------------------------------------------------------------------

void tstCatMngr::testCreateNewCategory()
{
  printStartMsg("tstCatMngr::testCreateNewCategory");
  
  TournamentDB db = getScenario01(true);
  CatMngr cmngr(&db);
  
  // try empty or invalid name
  CPPUNIT_ASSERT(cmngr.createNewCategory("") == INVALID_NAME);
  CPPUNIT_ASSERT(cmngr.createNewCategory(QString::null) == INVALID_NAME);
  CPPUNIT_ASSERT(db[TAB_CATEGORY].length() == 0);
  
  // actually create a valid category
  CPPUNIT_ASSERT(cmngr.createNewCategory("c1") == OK);
  CPPUNIT_ASSERT(db[TAB_CATEGORY].length() == 1);
  TabRow r = db[TAB_CATEGORY][1];
  CPPUNIT_ASSERT(r[GENERIC_NAME_FIELD_NAME].toString() == "c1");
  
  // make sure the default values are set correctly
  Category c = cmngr.getCategory("c1");
  CPPUNIT_ASSERT(c.getState() == CAT_CONFIG);
  CPPUNIT_ASSERT(c.getMatchSystem() == GROUPS_WITH_KO);
  CPPUNIT_ASSERT(c.getMatchType() == SINGLES);
  CPPUNIT_ASSERT(c.getSex() == M);
  
  // name collision
  CPPUNIT_ASSERT(cmngr.createNewCategory("c1") == NAME_EXISTS);
  CPPUNIT_ASSERT(db[TAB_CATEGORY].length() == 1);
  
  printEndMsg();
}

//----------------------------------------------------------------------------

void tstCatMngr::testHasCategory()
{
  printStartMsg("tstCatMngr::testHasCategory");
  
  TournamentDB db = getScenario01(true);
  CatMngr cmngr(&db);
  
  // try queries on empty table
  CPPUNIT_ASSERT(cmngr.hasCategory("") == false);
  CPPUNIT_ASSERT(cmngr.hasCategory(QString::null) == false);
  CPPUNIT_ASSERT(cmngr.hasCategory("abc") == false);
  
  // actually create a valid category
  CPPUNIT_ASSERT(cmngr.createNewCategory("c1") == OK);
  
  // try queries on filled table
  CPPUNIT_ASSERT(cmngr.hasCategory("") == false);
  CPPUNIT_ASSERT(cmngr.hasCategory(QString::null) == false);
  CPPUNIT_ASSERT(cmngr.hasCategory("abc") == false);
  CPPUNIT_ASSERT(cmngr.hasCategory("c1") == true);
  
  printEndMsg();
}

//----------------------------------------------------------------------------

void tstCatMngr::testGetCategory()
{
  printStartMsg("tstCatMngr::testGetCategory");
  
  TournamentDB db = getScenario01(true);
  CatMngr cmngr(&db);
  
  // actually create a valid category
  CPPUNIT_ASSERT(cmngr.createNewCategory("c1") == OK);
  CPPUNIT_ASSERT(cmngr.createNewCategory("c2") == OK);
  
  // try queries on filled table
  CPPUNIT_ASSERT_THROW(cmngr.getCategory(""), std::invalid_argument);
  CPPUNIT_ASSERT_THROW(cmngr.getCategory(QString::null), std::invalid_argument);
  CPPUNIT_ASSERT_THROW(cmngr.getCategory("dflsjdf"), std::invalid_argument);
  Category c = cmngr.getCategory("c2");
  CPPUNIT_ASSERT(c.getId() == 2);
  CPPUNIT_ASSERT(c.getName() == "c2");
  
  printEndMsg();
}

//----------------------------------------------------------------------------
    
void tstCatMngr::testGetAllCategories()
{
  printStartMsg("tstCatMngr::testGetAllCategories");
  
  TournamentDB db = getScenario01(true);
  CatMngr cmngr(&db);
  
  // run on empty table
  QList<Category> result = cmngr.getAllCategories();
  CPPUNIT_ASSERT(result.length() == 0);
  
  // actually create a valid category
  CPPUNIT_ASSERT(cmngr.createNewCategory("c1") == OK);
  CPPUNIT_ASSERT(cmngr.createNewCategory("c2") == OK);
  
  // run on filled table
  result = cmngr.getAllCategories();
  CPPUNIT_ASSERT(result.length() == 2);
  Category c = result.at(0);
  CPPUNIT_ASSERT(c.getId() == 1);
  CPPUNIT_ASSERT(c.getName() == "c1");
  c = result.at(1);
  CPPUNIT_ASSERT(c.getId() == 2);
  CPPUNIT_ASSERT(c.getName() == "c2");
  
  
  printEndMsg();
}

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    
