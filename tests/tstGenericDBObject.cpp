/* 
 * File:   tstGenericDBObject.cpp
 * Author: volker
 * 
 * Created on March 2, 2014, 3:46 PM
 */

#include <stdexcept>

#include "tstGenericDBObject.h"
#include "BasicTestClass.h"
#include "GenericDatabaseObject.h"

using namespace QTournament;

void tstGenericDBObject::testConstructor()
{
  printStartMsg("tstGenericDBObject::testConstructor");
  
  // test invalid name
  CPPUNIT_ASSERT_THROW(GenericDatabaseObject gdo("", 42), std::invalid_argument);
  CPPUNIT_ASSERT_THROW(GenericDatabaseObject gdo(QString::null, 42), std::invalid_argument);
  
  // test invalid ID
  CPPUNIT_ASSERT_THROW(GenericDatabaseObject gdo("t1", 0), std::invalid_argument);
  CPPUNIT_ASSERT_THROW(GenericDatabaseObject gdo("t1", -5), std::invalid_argument);
  
  // successful init
  GenericDatabaseObject gdo("t1", 5);
  
  printEndMsg();
}

//----------------------------------------------------------------------------

void tstGenericDBObject::testComparison()
{
  printStartMsg("tstGenericDBObject::testComparison");

  GenericDatabaseObject o1("t1", 1);
  GenericDatabaseObject o2("t1", 2);
  GenericDatabaseObject o3("t2", 1);
  GenericDatabaseObject o4("t2", 2);
  GenericDatabaseObject o5("t1", 2);
  
  // test some combinations of table name and ID
  CPPUNIT_ASSERT(!(o1 == o2));
  CPPUNIT_ASSERT(!(o1 == o3));
  CPPUNIT_ASSERT(!(o1 == o4));
  
  // test comparison with self
  CPPUNIT_ASSERT(o1 == o1);
  
  // test comparison with non-self object referencing the same DB object
  CPPUNIT_ASSERT(o2 == o5);
  
  // repeat all tests above for the != operator
  CPPUNIT_ASSERT(o1 != o2);
  CPPUNIT_ASSERT(o1 != o3);
  CPPUNIT_ASSERT(o1 != o4);
  CPPUNIT_ASSERT(!(o1 != o1));
  CPPUNIT_ASSERT(!(o2 != o5));
  
}

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    
