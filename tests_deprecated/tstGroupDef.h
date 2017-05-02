/* 
 * File:   tstGroupDef.h
 * Author: volker
 *
 * Created on March 2, 2014, 3:46 PM
 */

#ifndef TSTGROUPDEF_H
#define	TSTGROUPDEF_H

#include <cppunit/TestCase.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "BasicTestClass.h"

class tstGroupDef : public BasicTestClass
{
  CPPUNIT_TEST_SUITE( tstGroupDef );
    CPPUNIT_TEST(testConstructors);
    CPPUNIT_TEST(testSetters);
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
  void testSetters();
  void testNumMatches();
};

#endif	/* TSTGENERICDBOBJECT_H */

