/* 
 * File:   tstTeamMngr.h
 * Author: volker
 *
 * Created on March 2, 2014, 3:46 PM
 */

#ifndef TSTCATEGORY_H
#define	TSTCATEGORY_H

#include <cppunit/TestCase.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "DatabaseTestScenario.h"

class tstCategory : public DatabaseTestScenario
{
  CPPUNIT_TEST_SUITE( tstCategory );
    CPPUNIT_TEST(testGetAddState);
    CPPUNIT_TEST(testHasPlayer);
//  CPPUNIT_TEST();
//  CPPUNIT_TEST();
//  CPPUNIT_TEST();
//  CPPUNIT_TEST();
//  CPPUNIT_TEST();
  CPPUNIT_TEST_SUITE_END();
  
public:
  void testGetAddState();
  void testHasPlayer();
};

#endif	/* TSTGENERICDBOBJECT_H */

