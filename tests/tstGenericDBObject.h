/* 
 * File:   tstGenericDBObject.h
 * Author: volker
 *
 * Created on March 2, 2014, 3:46 PM
 */

#ifndef TSTGENERICDBOBJECT_H
#define	TSTGENERICDBOBJECT_H

#include <cppunit/TestCase.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "BasicTestClass.h"

class tstGenericDBObject : public BasicTestClass
{
  CPPUNIT_TEST_SUITE( tstGenericDBObject );
  CPPUNIT_TEST(testConstructor);
  CPPUNIT_TEST(testComparison);
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
    void testComparison();
};

#endif	/* TSTGENERICDBOBJECT_H */

