/* 
 * File:   tstTeamMngr.h
 * Author: volker
 *
 * Created on March 2, 2014, 3:46 PM
 */

#ifndef TSTTEAMMNGR_H
#define	TSTTEAMMNGR_H

#include <cppunit/TestCase.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "DatabaseTestScenario.h"

class tstTeamMngr : public DatabaseTestScenario
{
  CPPUNIT_TEST_SUITE( tstTeamMngr );
    CPPUNIT_TEST(testCreateNewTeam);
    CPPUNIT_TEST(testCreateNewTeam2);
    CPPUNIT_TEST(testHasTeam);
    CPPUNIT_TEST(testGetTeam);
    CPPUNIT_TEST(testGetAllTeams);
//  CPPUNIT_TEST();
//  CPPUNIT_TEST();
//  CPPUNIT_TEST();
//  CPPUNIT_TEST();
//  CPPUNIT_TEST();
//  CPPUNIT_TEST();
  CPPUNIT_TEST_SUITE_END();
  
public:
  void testCreateNewTeam();
  void testCreateNewTeam2();
  void testHasTeam();
  void testGetTeam();
  void testGetAllTeams();
};

#endif	/* TSTGENERICDBOBJECT_H */

