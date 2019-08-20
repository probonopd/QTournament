/* 
 * File:   tstGenericDBObject.cpp
 * Author: volker
 * 
 * Created on March 2, 2014, 3:46 PM
 */

#include <stdexcept>

#include "tstCatMngr.h"
#include "BasicTestClass.h"
#include "GenericDatabaseObject.h"
#include "TournamentDB.h"
#include "TournamentDataDefs.h"
#include "CatMngr.h"
#include "TournamentErrorCodes.h"
#include "Tournament.h"

using namespace QTournament;
using namespace dbOverlay;

//----------------------------------------------------------------------------

void tstCatMngr::testCreateNewCategory()
{
  printStartMsg("tstCatMngr::testCreateNewCategory");
  
  TournamentDB* db = getScenario01(true);
  CatMngr cmngr(db);
  
  // try empty or invalid name
  CPPUNIT_ASSERT(cmngr.createNewCategory("") == InvalidName);
  CPPUNIT_ASSERT(cmngr.createNewCategory(QString::null) == InvalidName);
  CPPUNIT_ASSERT((*db)[TAB_CATEGORY].length() == 0);
  
  // actually create a valid category
  CPPUNIT_ASSERT(cmngr.createNewCategory("c1") == OK);
  CPPUNIT_ASSERT((*db)[TAB_CATEGORY].length() == 1);
  TabRow r = (*db)[TAB_CATEGORY][1];
  CPPUNIT_ASSERT(r[GENERIC_NAME_FIELD_NAME].toString() == "c1");
  
  // make sure the default values are set correctly
  Category c = cmngr.getCategory("c1");
  CPPUNIT_ASSERT(c.getState() == STAT_CAT_Config);
  CPPUNIT_ASSERT(c.getMatchSystem() == MatchSystem::GroupsWithKO);
  CPPUNIT_ASSERT(c.getMatchType() == MatchType::Singles);
  CPPUNIT_ASSERT(c.getSex() == M);
  
  // name collision
  CPPUNIT_ASSERT(cmngr.createNewCategory("c1") == NameExists);
  CPPUNIT_ASSERT((*db)[TAB_CATEGORY].length() == 1);
  
  delete db;
  printEndMsg();
}

//----------------------------------------------------------------------------

void tstCatMngr::testHasCategory()
{
  printStartMsg("tstCatMngr::testHasCategory");
  
  TournamentDB* db = getScenario01(true);
  CatMngr cmngr(db);
  
  // try queries on empty table
  CPPUNIT_ASSERT(cmngr.hasCategory("") == false);
  CPPUNIT_ASSERT(cmngr.hasCategory(QString::null) == false);
  CPPUNIT_ASSERT(cmngr.hasCategory("abc") == false);
  
  // actually create a valid category
  CPPUNIT_ASSERT(cmngr.createNewCategory("c1") == OK);
  
  // try queries on filled table
  CPPUNIT_ASSERT(cmngr.hasCategory("") == false);
  CPPUNIT_ASSERT(cmngr.hasCategory(QString::null) == false);
  CPPUNIT_ASSERT(cmngr.hasCategory("abc") == false);
  CPPUNIT_ASSERT(cmngr.hasCategory("c1") == true);
  
  delete db;
  printEndMsg();
}

//----------------------------------------------------------------------------

void tstCatMngr::testGetCategory()
{
  printStartMsg("tstCatMngr::testGetCategory");
  
  TournamentDB* db = getScenario01(true);
  CatMngr cmngr(db);
  
  // actually create a valid category
  CPPUNIT_ASSERT(cmngr.createNewCategory("c1") == OK);
  CPPUNIT_ASSERT(cmngr.createNewCategory("c2") == OK);
  
  // try queries on filled table
  CPPUNIT_ASSERT_THROW(cmngr.getCategory(""), std::invalid_argument);
  CPPUNIT_ASSERT_THROW(cmngr.getCategory(QString::null), std::invalid_argument);
  CPPUNIT_ASSERT_THROW(cmngr.getCategory("dflsjdf"), std::invalid_argument);
  Category c = cmngr.getCategory("c2");
  CPPUNIT_ASSERT(c.getId() == 2);
  CPPUNIT_ASSERT(c.getName() == "c2");
  
  delete db;
  printEndMsg();
}

//----------------------------------------------------------------------------
    
void tstCatMngr::testGetAllCategories()
{
  printStartMsg("tstCatMngr::testGetAllCategories");
  
  TournamentDB* db = getScenario01(true);
  CatMngr cmngr(db);
  
  // run on empty table
  QList<Category> result = cmngr.getAllCategories();
  CPPUNIT_ASSERT(result.length() == 0);
  
  // actually create a valid category
  CPPUNIT_ASSERT(cmngr.createNewCategory("c1") == OK);
  CPPUNIT_ASSERT(cmngr.createNewCategory("c2") == OK);
  
  // run on filled table
  result = cmngr.getAllCategories();
  CPPUNIT_ASSERT(result.length() == 2);
  Category c = result.at(0);
  CPPUNIT_ASSERT(c.getId() == 1);
  CPPUNIT_ASSERT(c.getName() == "c1");
  c = result.at(1);
  CPPUNIT_ASSERT(c.getId() == 2);
  CPPUNIT_ASSERT(c.getName() == "c2");
  
  delete db;
  printEndMsg();
}

//----------------------------------------------------------------------------
    
void tstCatMngr::testAddPlayerToCategory()
{
  printStartMsg("tstCatMngr::testAddPlayerToCategory");
  
  TournamentDB* db = getScenario02(true);
  Tournament t(getSqliteFileName());
  
  // create a team some dummy players
  Player m1 = Tournament::getPlayerMngr()->getPlayer("f", "l1");
  Player f1 = Tournament::getPlayerMngr()->getPlayer("f", "l2");
  Player m2 = Tournament::getPlayerMngr()->getPlayer("f", "l3");
  Player f2 = Tournament::getPlayerMngr()->getPlayer("f", "l4");
  Player f3 = Tournament::getPlayerMngr()->getPlayer("f", "l6");
  
  // create one category of every sex
  CatMngr* cmngr = Tournament::getCatMngr();
  Category ms = cmngr->getCategory("MS");
  Category ld = cmngr->getCategory("LD");
  Category mx = cmngr->getCategory("MX");
  
  // add players to men's singles
  CPPUNIT_ASSERT(cmngr->addPlayerToCategory(m1, ms) == OK);
  CPPUNIT_ASSERT(ms.hasPlayer(m1));
  CPPUNIT_ASSERT(cmngr->addPlayerToCategory(m1, ms) == PlayerAlreadyInCategory);
  CPPUNIT_ASSERT(cmngr->addPlayerToCategory(f1, ms) == PlayerNotSuitable);
  CPPUNIT_ASSERT(cmngr->addPlayerToCategory(m2, ms) == OK);
  CPPUNIT_ASSERT(ms.hasPlayer(m2));
  ms.setSex(DONT_CARE);   // relax checks
  CPPUNIT_ASSERT(cmngr->addPlayerToCategory(f1, ms) == OK);
  CPPUNIT_ASSERT(ms.hasPlayer(f1));
  
  // add players to ladies' doubles
  CPPUNIT_ASSERT(cmngr->addPlayerToCategory(f1, ld) == OK);
  CPPUNIT_ASSERT(ld.hasPlayer(f1));
  CPPUNIT_ASSERT(cmngr->addPlayerToCategory(f1, ld) == PlayerAlreadyInCategory);
  CPPUNIT_ASSERT(cmngr->addPlayerToCategory(m1, ld) == PlayerNotSuitable);
  CPPUNIT_ASSERT(cmngr->addPlayerToCategory(f2, ld) == OK);
  CPPUNIT_ASSERT(ld.hasPlayer(f2));
  ld.setSex(DONT_CARE);   // relax checks
  CPPUNIT_ASSERT(cmngr->addPlayerToCategory(m1, ld) == OK);
  CPPUNIT_ASSERT(ld.hasPlayer(m1));
  
  // add players to mixed doubles
  CPPUNIT_ASSERT(cmngr->addPlayerToCategory(f1, mx) == OK);
  CPPUNIT_ASSERT(mx.hasPlayer(f1));
  CPPUNIT_ASSERT(cmngr->addPlayerToCategory(f1, mx) == PlayerAlreadyInCategory);
  CPPUNIT_ASSERT(cmngr->addPlayerToCategory(m1, mx) == OK);
  CPPUNIT_ASSERT(mx.hasPlayer(m1));
  CPPUNIT_ASSERT(cmngr->addPlayerToCategory(f2, mx) == OK);
  CPPUNIT_ASSERT(mx.hasPlayer(f2));
  mx.setSex(DONT_CARE);   // relax checks
  CPPUNIT_ASSERT(cmngr->addPlayerToCategory(m2, mx) == OK);
  CPPUNIT_ASSERT(mx.hasPlayer(m2));
  CPPUNIT_ASSERT(cmngr->addPlayerToCategory(f3, mx) == OK);
  CPPUNIT_ASSERT(mx.hasPlayer(f3));
  
  // TODO:
  // Add state-dependent tests, e.g. adding players after category configuration

  delete db;
  printEndMsg();
}

//----------------------------------------------------------------------------
    
void tstCatMngr::testRemovePlayerFromCategory()
{
  printStartMsg("tstCatMngr::testRemovePlayerFromCategory");
  
  TournamentDB* db = getScenario02(true);
  Tournament t(getSqliteFileName());
  
  // create a team some dummy players
  Player m1 = Tournament::getPlayerMngr()->getPlayer("f", "l1");
  Player f1 = Tournament::getPlayerMngr()->getPlayer("f", "l2");
  Player f2 = Tournament::getPlayerMngr()->getPlayer("f", "l4");
  Player f3 = Tournament::getPlayerMngr()->getPlayer("f", "l6");
  
  // create a category
  CatMngr* cmngr = Tournament::getCatMngr();
  Category ls = cmngr->getCategory("LS");
  
  // add players to men's singles
  CPPUNIT_ASSERT(cmngr->addPlayerToCategory(f1, ls) == OK);
  CPPUNIT_ASSERT(cmngr->addPlayerToCategory(f2, ls) == OK);
  CPPUNIT_ASSERT(ls.hasPlayer(f1));
  CPPUNIT_ASSERT(ls.hasPlayer(f2));
  
  // try to remove not-added players
  CPPUNIT_ASSERT(cmngr->removePlayerFromCategory(f3, ls) == PlayerNotInCategory);
  CPPUNIT_ASSERT(cmngr->removePlayerFromCategory(m1, ls) == PlayerNotInCategory);
  CPPUNIT_ASSERT(ls.hasPlayer(f1));
  CPPUNIT_ASSERT(ls.hasPlayer(f2));
  
  // actually remove player
  CPPUNIT_ASSERT(cmngr->removePlayerFromCategory(f2, ls) == OK);
  CPPUNIT_ASSERT(ls.hasPlayer(f1));
  CPPUNIT_ASSERT(ls.hasPlayer(f2) == false);
  
  delete db;
  printEndMsg();
}

//----------------------------------------------------------------------------

void tstCatMngr::testFreezeCategory()
{
  printStartMsg("tstCatMngr::testRemovePlayerFromCategory");
  
  TournamentDB* db = getScenario02(true);
  Tournament t(getSqliteFileName());
  
  // get a team and a category and assign 40 brand new players to it
  CatMngr* cmngr = Tournament::getCatMngr();
  PlayerMngr* pmngr = Tournament::getPlayerMngr();
  Category ms = cmngr->getCategory("MS");
  for (int i=0; i < 40; i++)
  {
    QString fn = "FirstName";
    QString ln = "LastName" + QString::number(i);
    CPPUNIT_ASSERT(pmngr->createNewPlayer(fn, ln, M, "t1") == OK);
    Player p = pmngr->getPlayer(fn, ln);
    CPPUNIT_ASSERT(cmngr->addPlayerToCategory(p, ms) == OK);
  }

  // set a valid match system and a valid configuration
  CPPUNIT_ASSERT(ms.setMatchSystem(MatchSystem::GroupsWithKO) == OK);
  CPPUNIT_ASSERT(ms.setMatchType(MatchType::Singles) == OK);
  GroupDef gd = GroupDef(4, 10); // 10 groups of four players each
  GroupDefList gdl;
  gdl.append(gd);
  KO_Config cfg = KO_Config(KO_Start::L16, false, gdl);
  CPPUNIT_ASSERT(cfg.isValid(40));
  CPPUNIT_ASSERT(ms.setParameter(GROUP_CONFIG, cfg.toString()) == true);
  
  // make sure the group can be frozen
  unique_ptr<Category> specialObj = ms.convertToSpecializedObject();
  CPPUNIT_ASSERT(specialObj->canFreezeConfig() == OK);
  
  // some db consistency checks before executing the actual "method under test"
  DbTab pairTab = (*db)[TAB_PAIRS];
  CPPUNIT_ASSERT(pairTab.length() == 0);
  QList<PlayerPair> ppList = ms.getPlayerPairs();
  CPPUNIT_ASSERT(ppList.count() == 40);
  for (int i=0; i<39; i++)
  {
    PlayerPair pp = ppList.at(i);
    CPPUNIT_ASSERT(pp.getPairId() == -1);
    CPPUNIT_ASSERT(pp.getPlayer1().getId() > 0);
    CPPUNIT_ASSERT(pp.hasPlayer2() == false);
  }
  
  CPPUNIT_ASSERT(ms.getState() == STAT_CAT_Config);
  
  // do the freeze
  CPPUNIT_ASSERT(cmngr->freezeConfig(ms) == OK);

  // some db consistency checks after executing the actual "method under test"
  CPPUNIT_ASSERT(pairTab.length() == 40);
  ppList = ms.getPlayerPairs();
  CPPUNIT_ASSERT(ppList.count() == 40);
  for (int i=0; i<39; i++)
  {
    PlayerPair pp = ppList.at(i);
    CPPUNIT_ASSERT(pp.getPairId() > 0);
    CPPUNIT_ASSERT(pp.getPlayer1().getId() > 0);
    CPPUNIT_ASSERT(pp.hasPlayer2() == false);
  }
  
  // check the actual state transition
  CPPUNIT_ASSERT(ms.getState() == STAT_CAT_Frozen);
  
  delete db;
  printEndMsg();
}

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    
