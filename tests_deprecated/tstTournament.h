/* 
 * File:   tstTournament.h
 * Author: volker
 *
 * Created on March 2, 2014, 4:50 PM
 */

#ifndef TSTTOURNAMENT_H
#define	TSTTOURNAMENT_H

#include <cppunit/TestCase.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "DatabaseTestScenario.h"

class tstTournament : public DatabaseTestScenario
{
  CPPUNIT_TEST_SUITE( tstTournament );
  CPPUNIT_TEST(testConstructor);
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
    void testConstructor();
};


#endif	/* TSTTOURNAMENT_H */

