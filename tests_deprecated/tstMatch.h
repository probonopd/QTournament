/* 
 * File:   tstTeamMngr.h
 * Author: volker
 *
 * Created on March 2, 2014, 3:46 PM
 */

#ifndef TSTMATCH_H
#define	TSTMATCH_H

#include <memory>
#include <cppunit/TestCase.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include "Match.h"

#include "DatabaseTestScenario.h"

class tstMatch : public DatabaseTestScenario
{
  CPPUNIT_TEST_SUITE( tstMatch );
    CPPUNIT_TEST(testGetCat);
    CPPUNIT_TEST(testGetGroup);
    CPPUNIT_TEST(testGetPairs);
//  CPPUNIT_TEST();
//  CPPUNIT_TEST();
//  CPPUNIT_TEST();
//  CPPUNIT_TEST();
//  CPPUNIT_TEST();
//  CPPUNIT_TEST();
//  CPPUNIT_TEST();
//  CPPUNIT_TEST();
//  CPPUNIT_TEST();
//  CPPUNIT_TEST();
//  CPPUNIT_TEST();
//  CPPUNIT_TEST();
  CPPUNIT_TEST_SUITE_END();
  
public:
  void testGetCat();
  void testGetGroup();
  void testGetPairs();

  unique_ptr<QTournament::Match> initMatchGroup();
};

#endif	/* TSTMATCH_H */

