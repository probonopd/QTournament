/* 
 * File:   tstTeamMngr.h
 * Author: volker
 *
 * Created on March 2, 2014, 3:46 PM
 */

#ifndef TSTSCORE_H
#define	TSTSCORE_H

#include <cppunit/TestCase.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "BasicTestClass.h"

class tstScore : public BasicTestClass
{
  CPPUNIT_TEST_SUITE( tstScore );
    CPPUNIT_TEST(testGameScore_IsValidScore);
    CPPUNIT_TEST(testGameScore_ToString);
    CPPUNIT_TEST(testGameScore_GetWinner);
    CPPUNIT_TEST(testMatchScore_FactoryFuncs_ToString);
    CPPUNIT_TEST(testMatchScore_GetWinner_GetLoser);
    CPPUNIT_TEST(testRandomMatchGeneration);
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
  void testGameScore_IsValidScore();
  void testGameScore_ToString();
  void testGameScore_GetWinner();
  void testMatchScore_FactoryFuncs_ToString();
  void testMatchScore_GetWinner_GetLoser();
  void testRandomMatchGeneration();
};

#endif	/* TSTSCORE_H */

