/* 
 * File:   tstTeamMngr.h
 * Author: volker
 *
 * Created on March 2, 2014, 3:46 PM
 */

#ifndef TSTRANKINGMNGR_H
#define	TSTRANKINGMNGR_H

#include <cppunit/TestCase.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "DatabaseTestScenario.h"

class tstRankingMngr : public DatabaseTestScenario
{
  CPPUNIT_TEST_SUITE( tstRankingMngr );
    //CPPUNIT_TEST(testCreateUnsortedRanking);
    CPPUNIT_TEST(testSortRanking__RoundRobin);
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
  void testCreateUnsortedRanking();
  void testSortRanking__RoundRobin();

private:
  void checkSortSequence__RoundRobin(int round);
};

#endif	/* TSTRANKINGMNGR_H */

