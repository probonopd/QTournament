/* 
 * File:   tstTeamMngr.h
 * Author: volker
 *
 * Created on March 2, 2014, 3:46 PM
 */

#ifndef TSTPLAYERMNGR_H
#define	TSTPLAYERMNGR_H

#include <cppunit/TestCase.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "DatabaseTestScenario.h"

class tstPlayerMngr : public DatabaseTestScenario
{
  CPPUNIT_TEST_SUITE( tstPlayerMngr );
    CPPUNIT_TEST(testCreateNewPlayer);
    CPPUNIT_TEST(testHasPlayer);
    CPPUNIT_TEST(testGetPlayer);
    CPPUNIT_TEST(testGetAllPlayers);
    CPPUNIT_TEST(testRenamePlayer);
//  CPPUNIT_TEST();
//  CPPUNIT_TEST();
//  CPPUNIT_TEST();
//  CPPUNIT_TEST();
//  CPPUNIT_TEST();
  CPPUNIT_TEST_SUITE_END();
  
public:
  void testCreateNewPlayer();
  void testHasPlayer();
  void testGetPlayer();
  void testGetAllPlayers();
  void testRenamePlayer();
};

#endif	/* TSTGENERICDBOBJECT_H */

