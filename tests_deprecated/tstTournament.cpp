/* 
 * File:   tstTournament.cpp
 * Author: volker
 * 
 * Created on March 2, 2014, 4:50 PM
 */

#include <stdexcept>

#include "tstTournament.h"
#include "Tournament.h"
#include "TournamentDataDefs.h"
#include "KeyValueTab.h"

using namespace QTournament;

void tstTournament::testConstructor()
{
  printStartMsg("tstTournament::testConstructor");
  
  // create a new, empty tournament file
  TournamentSettings cfg;
  cfg.organizingClub = "club";
  cfg.tournamentName = "TestTournament";
  cfg.useTeams = true;
  
  QString tFileName = getSqliteFileName();
  CPPUNIT_ASSERT(!(sqliteFileExists()));
  Tournament t(tFileName, cfg);
  CPPUNIT_ASSERT(sqliteFileExists());
  
  // make sure the config values have been set
  TournamentDB db(getSqliteFileName(), false);
  KeyValueTab kvt = KeyValueTab::getTab(&db, TabCfg);
  CPPUNIT_ASSERT(kvt.getString(CfgKey_TnmtOrga) == "club");
  CPPUNIT_ASSERT(kvt.getString(CfgKey_TnmtName) == "TestTournament");
  CPPUNIT_ASSERT(kvt.getBool(CfgKey_UseTeams));
  CPPUNIT_ASSERT(kvt.getInt(CfgKey_DbVersion) == DB_VERSION);
  
  // try to create a new file although it's already existing
  Tournament* pTnmt;
  CPPUNIT_ASSERT_THROW(pTnmt = new Tournament(tFileName, cfg), std::invalid_argument);
  
  // open a file that's not existing
  CPPUNIT_ASSERT_THROW(pTnmt = new Tournament("sd,fnsdf"), std::invalid_argument);
  
  // open an existing file
  pTnmt = new Tournament(tFileName);
  delete pTnmt;
  
  printEndMsg();
}

//----------------------------------------------------------------------------

