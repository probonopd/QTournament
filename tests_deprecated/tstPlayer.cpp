/* 
 * File:   tstGenericDBObject.cpp
 * Author: volker
 * 
 * Created on March 2, 2014, 3:46 PM
 */

#include <stdexcept>

#include "tstPlayer.h"
#include "TournamentDB.h"
#include "TournamentDataDefs.h"
#include "TournamentErrorCodes.h"
#include "PlayerMngr.h"
#include "Tournament.h"

using namespace QTournament;
using namespace dbOverlay;

//----------------------------------------------------------------------------

void tstPlayer::testGetAssignedCategories()
{
  printStartMsg("tstPlayer::testGetAssignedTeams");
  
  
  TournamentDB* db = getScenario02(true);
  Tournament t(getSqliteFileName());
  
  // get some players and categories
  Player m1 = Tournament::getPlayerMngr()->getPlayer("f", "l1");
  CatMngr* cmngr = Tournament::getCatMngr();
  Category ms = cmngr->getCategory("MS");
  Category mx = cmngr->getCategory("MX");
  
  // test empty result
  QList<Category> result;
  result = m1.getAssignedCategories();
  CPPUNIT_ASSERT(result.length() == 0);
  
  // assign player to one category
  CPPUNIT_ASSERT(cmngr->addPlayerToCategory(m1, ms) == OK);
  CPPUNIT_ASSERT(ms.hasPlayer(m1));
  result = m1.getAssignedCategories();
  CPPUNIT_ASSERT(result.length() == 1);
  CPPUNIT_ASSERT(result.at(0) == ms);
  
  // assign player to another category
  CPPUNIT_ASSERT(cmngr->addPlayerToCategory(m1, mx) == OK);
  CPPUNIT_ASSERT(mx.hasPlayer(m1));
  result = m1.getAssignedCategories();
  CPPUNIT_ASSERT(result.length() == 2);
  CPPUNIT_ASSERT(result.at(0) == ms);
  CPPUNIT_ASSERT(result.at(1) == mx);
  
  delete db;
  printEndMsg();
}

//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    
