/* 
 * File:   tstTeamMngr.h
 * Author: volker
 *
 * Created on March 2, 2014, 3:46 PM
 */

#ifndef TSTRRGENERATOR_H
#define	TSTRRGENERATOR_H

#include <cppunit/TestCase.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "DatabaseTestScenario.h"

class tstRoundRobinGenerator : public DatabaseTestScenario
{
  CPPUNIT_TEST_SUITE( tstRoundRobinGenerator );
    CPPUNIT_TEST(testMatchCreation);
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
  void testMatchCreation();
};

#endif	/* TSTRRGENERATOR_H */

