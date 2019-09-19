/* 
 * File:   tstGenericDBObject.cpp
 * Author: volker
 * 
 * Created on March 2, 2014, 3:46 PM
 */

#include <stdexcept>

#include "tstKOConfig.h"
#include "TournamentDataDefs.h"
#include "KO_Config.h"
#include <stdexcept>

using namespace QTournament;
using namespace dbOverlay;

//----------------------------------------------------------------------------

void tstKOConfig::testConstructors()
{
  printStartMsg("tstKOConfig::testConstructors");
  
  // Constructor with empty groups
  KO_Config c = KO_Config(KO_Start::Semi, false);
  CPPUNIT_ASSERT(c.isValid() == false);
  CPPUNIT_ASSERT(c.getNumGroupDefs() == 0);
  CPPUNIT_ASSERT(c.getSecondSurvives() == false);
  CPPUNIT_ASSERT(c.getStartLevel() == KO_Start::Semi);
  
  // create some dummy group definitions
  GroupDefList l;
  l.append(GroupDef(4,50));
  l.append(GroupDef(5,10));
  
  // constructor with non-empty group defs
  c = KO_Config(KO_Start::Semi, false, l);
  CPPUNIT_ASSERT(c.isValid() == false);
  CPPUNIT_ASSERT(c.getNumGroupDefs() == 2);
  CPPUNIT_ASSERT(c.getSecondSurvives() == false);
  CPPUNIT_ASSERT(c.getStartLevel() == KO_Start::Semi);
  CPPUNIT_ASSERT(c.getGroupDef(0) == l[0]);
  CPPUNIT_ASSERT(c.getGroupDef(1) == l[1]);
  
  // test the copy constructor
  KO_Config* c2 = new KO_Config(KO_Start::Quarter, true, l);
  KO_Config c3 = KO_Config(*c2);
  c2->setSecondSurvives(false);  // modify the original
  c2->setStartLevel(KO_Start::L16);        // modify the original
  CPPUNIT_ASSERT(c3.isValid() == false);
  CPPUNIT_ASSERT(c3.getNumGroupDefs() == 2);
  CPPUNIT_ASSERT(c3.getSecondSurvives() == true);
  CPPUNIT_ASSERT(c3.getStartLevel() == KO_Start::Quarter);
  delete c2;
  
  printEndMsg();
}

//----------------------------------------------------------------------------

void tstKOConfig::testNumMatches()
{
  printStartMsg("tstGroupDef::testNumMatches");
  
  // test the calculation of the matches for the KO-rounds
  KO_Config c = KO_Config(KO_Start::Semi, false);
  CPPUNIT_ASSERT(c.getNumMatches() == 4);
  c.setStartLevel(KO_Start::Quarter);
  CPPUNIT_ASSERT(c.getNumMatches() == 8);
  c.setStartLevel(KO_Start::L16);
  CPPUNIT_ASSERT(c.getNumMatches() == 16);
  
  // include some group matches
  GroupDefList l;
  l.append(GroupDef(4, 2));  // results in two groups with six matches each
  c = KO_Config(KO_Start::Semi, false, l);
  CPPUNIT_ASSERT(c.getNumMatches() == 4 + 2*6);
  
  l.append(GroupDef(5, 3));  // results in three groups with 10 matches each
  c = KO_Config(KO_Start::Semi, false, l);
  CPPUNIT_ASSERT(c.getNumMatches() == 4 + 2*6 + 3*10);
  
  printEndMsg();
}

//----------------------------------------------------------------------------
    
void tstKOConfig::testNumGroups()
{
  printStartMsg("tstGroupDef::testNumGroups");
  
  // test the calculation of the groups
  KO_Config c = KO_Config(KO_Start::Semi, false);
  CPPUNIT_ASSERT(c.getNumReqGroups() == 4);
  c.setSecondSurvives(true);
  CPPUNIT_ASSERT(c.getNumReqGroups() == 2);
  
  c.setStartLevel(KO_Start::Quarter);
  CPPUNIT_ASSERT(c.getNumReqGroups() == 4);
  c.setSecondSurvives(false);
  CPPUNIT_ASSERT(c.getNumReqGroups() == 8);
  
  c.setStartLevel(KO_Start::L16);
  CPPUNIT_ASSERT(c.getNumReqGroups() == 16);
  c.setSecondSurvives(true);
  CPPUNIT_ASSERT(c.getNumReqGroups() == 8);
  
  printEndMsg();
}

//----------------------------------------------------------------------------

void tstKOConfig::testToString()
{
  printStartMsg("tstGroupDef::testToString");
  
  // Constructor with empty groups
  KO_Config c = KO_Config(KO_Start::Semi, false);
  CPPUNIT_ASSERT(c.toString() == "S;0;");
  
  c = KO_Config(KO_Start::Quarter, true);
  CPPUNIT_ASSERT(c.toString() == "Q;1;");
  
  // add some group matches
  GroupDefList l;
  l.append(GroupDef(4, 2));  // results in two groups with four players each
  c = KO_Config(KO_Start::L16, false, l);
  CPPUNIT_ASSERT(c.toString() == "KO_Start::L16;0;2;4;");
  
  l.append(GroupDef(8, 4));
  c = KO_Config(KO_Start::L16, true, l);
  CPPUNIT_ASSERT(c.toString() == "KO_Start::L16;1;2;4;4;8;");
}

//----------------------------------------------------------------------------

void tstKOConfig::testFromString()
{
  CPPUNIT_ASSERT_THROW(KO_Config(""), std::invalid_argument);
  CPPUNIT_ASSERT_THROW(KO_Config(";"), std::invalid_argument);
  CPPUNIT_ASSERT_THROW(KO_Config(";;"), std::invalid_argument);
  CPPUNIT_ASSERT_THROW(KO_Config(";;;"), std::invalid_argument);
  CPPUNIT_ASSERT_THROW(KO_Config("x;0;"), std::invalid_argument);
  CPPUNIT_ASSERT_THROW(KO_Config("Q;x;"), std::invalid_argument);
  CPPUNIT_ASSERT_THROW(KO_Config("fsdklfjs;1;"), std::invalid_argument);
  CPPUNIT_ASSERT_THROW(KO_Config("Q;1;42;4;88"), std::invalid_argument);
  CPPUNIT_ASSERT_THROW(KO_Config("Q;1;42"), std::invalid_argument);
  CPPUNIT_ASSERT_THROW(KO_Config("Q;6;"), std::invalid_argument);
  
  KO_Config c = KO_Config("Q;1;");
  CPPUNIT_ASSERT(c.getStartLevel() == KO_Start::Quarter);
  CPPUNIT_ASSERT(c.getSecondSurvives() == true);
  CPPUNIT_ASSERT(c.getNumGroupDefs() == 0);
  
  c = KO_Config("KO_Start::L16;0;");
  CPPUNIT_ASSERT(c.getStartLevel() == KO_Start::L16);
  CPPUNIT_ASSERT(c.getSecondSurvives() == false);
  CPPUNIT_ASSERT(c.getNumGroupDefs() == 0);
  
  c = KO_Config("S;0;10;4;");
  CPPUNIT_ASSERT(c.getStartLevel() == KO_Start::Semi);
  CPPUNIT_ASSERT(c.getSecondSurvives() == false);
  CPPUNIT_ASSERT(c.getNumGroupDefs() == 1);
  CPPUNIT_ASSERT(c.getGroupDef(0) == GroupDef(4, 10));
  
  c = KO_Config("  KO_Start::L16  ; 1 ;10 ;4   ; 0 ; 6  ;   ");
  CPPUNIT_ASSERT(c.getStartLevel() == KO_Start::L16);
  CPPUNIT_ASSERT(c.getSecondSurvives() == true);
  CPPUNIT_ASSERT(c.getNumGroupDefs() == 2);
  CPPUNIT_ASSERT(c.getGroupDef(0) == GroupDef(4, 10));
  CPPUNIT_ASSERT(c.getGroupDef(1) == GroupDef(6));
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
    
