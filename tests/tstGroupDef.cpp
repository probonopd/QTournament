/* 
 * File:   tstGenericDBObject.cpp
 * Author: volker
 * 
 * Created on March 2, 2014, 3:46 PM
 */

#include <stdexcept>

#include "tstGroupDef.h"
#include "TournamentDataDefs.h"
#include "GroupDef.h"
#include <stdexcept>

using namespace QTournament;
using namespace dbOverlay;

//----------------------------------------------------------------------------

void tstGroupDef::testConstructors()
{
  printStartMsg("tstGroupDef::testConstructors");
  
  // regular constructor
  GroupDef g = GroupDef(4, 42);
  CPPUNIT_ASSERT(g.getGroupSize() == 4);
  CPPUNIT_ASSERT(g.getNumGroups() == 42);
  
  // default argument: zero groups
  g = GroupDef(8);
  CPPUNIT_ASSERT(g.getGroupSize() == 8);
  CPPUNIT_ASSERT(g.getNumGroups() == 0);
  
  // invalid group size
  CPPUNIT_ASSERT_THROW(GroupDef(2, 42), std::invalid_argument);
  CPPUNIT_ASSERT_THROW(GroupDef(0, 42), std::invalid_argument);
  CPPUNIT_ASSERT_THROW(GroupDef(-1, 42), std::invalid_argument);
  
  // invalid number of groups
  CPPUNIT_ASSERT_THROW(GroupDef(3, -1), std::invalid_argument);
  
  // test the copy constructor
  g = GroupDef(8, 88);
  GroupDef gCopy = GroupDef(g);
  g.setGroupSize(7);  // modify original
  g.setNumGroups(77); // modify original
  CPPUNIT_ASSERT(g.getGroupSize() == 7);  // test original
  CPPUNIT_ASSERT(g.getNumGroups() == 77);  // test original
  CPPUNIT_ASSERT(gCopy.getGroupSize() == 8);  // test copy
  CPPUNIT_ASSERT(gCopy.getNumGroups() == 88);  // test copy
  
  printEndMsg();
}

//----------------------------------------------------------------------------

void tstGroupDef::testSetters()
{
  printStartMsg("tstGroupDef::testSetters");
  
  // regular constructor
  GroupDef g = GroupDef(4, 42);
  CPPUNIT_ASSERT(g.getGroupSize() == 4);
  CPPUNIT_ASSERT(g.getNumGroups() == 42);
  
  // set group size
  g.setGroupSize(3);
  CPPUNIT_ASSERT(g.getGroupSize() == 3);
  CPPUNIT_ASSERT(g.getNumGroups() == 42);
  
  // illegal group size
  CPPUNIT_ASSERT(g.setGroupSize(2) == false);
  CPPUNIT_ASSERT(g.getGroupSize() == 3);
  CPPUNIT_ASSERT(g.getNumGroups() == 42);
  CPPUNIT_ASSERT(g.setGroupSize(0) == false);
  CPPUNIT_ASSERT(g.getGroupSize() == 3);
  CPPUNIT_ASSERT(g.getNumGroups() == 42);
  CPPUNIT_ASSERT(g.setGroupSize(-1) == false);
  CPPUNIT_ASSERT(g.getGroupSize() == 3);
  CPPUNIT_ASSERT(g.getNumGroups() == 42);
  
  // set number of groups
  g.setNumGroups(0);
  CPPUNIT_ASSERT(g.getGroupSize() == 3);
  CPPUNIT_ASSERT(g.getNumGroups() == 0);
  g.setNumGroups(23);
  CPPUNIT_ASSERT(g.getGroupSize() == 3);
  CPPUNIT_ASSERT(g.getNumGroups() == 23);
  
  // illegal group numbers
  CPPUNIT_ASSERT(g.setNumGroups(-1) == false);
  CPPUNIT_ASSERT(g.getGroupSize() == 3);
  CPPUNIT_ASSERT(g.getNumGroups() == 23);
  
  printEndMsg();
}

//----------------------------------------------------------------------------

void tstGroupDef::testNumMatches()
{
  printStartMsg("tstGroupDef::testNumMatches");
  
  GroupDef g = GroupDef(4, 1);
  CPPUNIT_ASSERT(g.getNumMatches() == 6);
  
  g = GroupDef(4, 2);
  CPPUNIT_ASSERT(g.getNumMatches() == 12);
  
  printEndMsg();
}

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    
