/* 
 * File:   tstGroupDef.h
 * Author: volker
 *
 * Created on March 2, 2014, 3:46 PM
 */

#ifndef TSTKOCONFIG_H
#define	TSTKOCONFIG_H

#include <cppunit/TestCase.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "BasicTestClass.h"

class tstKOConfig : public BasicTestClass
{
  CPPUNIT_TEST_SUITE( tstKOConfig );
    CPPUNIT_TEST(testConstructors);
    //CPPUNIT_TEST(testSetters);
    CPPUNIT_TEST(testNumMatches);
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
  void testConstructors();
  //void testSetters();
  void testNumMatches();
};

#endif	/* TSTGENERICDBOBJECT_H */

