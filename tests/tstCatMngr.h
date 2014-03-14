/* 
 * File:   tstTeamMngr.h
 * Author: volker
 *
 * Created on March 2, 2014, 3:46 PM
 */

#ifndef TSTCATMNGR_H
#define	TSTCATMNGR_H

#include <cppunit/TestCase.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "DatabaseTestScenario.h"

class tstCatMngr : public DatabaseTestScenario
{
  CPPUNIT_TEST_SUITE( tstCatMngr );
    CPPUNIT_TEST(testCreateNewCategory);
    CPPUNIT_TEST(testHasCategory);
    CPPUNIT_TEST(testGetCategory);
    CPPUNIT_TEST(testGetAllCategories);
    CPPUNIT_TEST(testAddPlayerToCategory);
    CPPUNIT_TEST(testRemovePlayerFromCategory);
//  CPPUNIT_TEST();
//  CPPUNIT_TEST();
//  CPPUNIT_TEST();
//  CPPUNIT_TEST();
  CPPUNIT_TEST_SUITE_END();
  
public:
  void testCreateNewCategory();
  void testHasCategory();
  void testGetCategory();
  void testGetAllCategories();
  void testAddPlayerToCategory();
  void testRemovePlayerFromCategory();
};

#endif	/* TSTGENERICDBOBJECT_H */

