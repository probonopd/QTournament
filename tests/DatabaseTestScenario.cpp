/*
 * Copyright © 2014 Volker Knollmann
 * 
 * This work is free. You can redistribute it and/or modify it under the
 * terms of the Do What The Fuck You Want To Public License, Version 2,
 * as published by Sam Hocevar. See the COPYING file or visit
 * http://www.wtfpl.net/ for more details.
 * 
 * This program comes without any warranty. Use it at your own risk or
 * don't use it at all.
 */

#include "DatabaseTestScenario.h"
#include "DbTab.h"
#include "Tournament.h"
#include "TournamentDataDefs.h"
#include "TournamentDB.h"

#include <QtSql/QSqlQuery>
#include <QFile>
#include <QtSql/QSqlError>

// initialize some constants
const QString DatabaseTestScenario::DB_CONNECTION_NAME = "unittest";
const QString DatabaseTestScenario::DB_USER = "unittest";
const QString DatabaseTestScenario::DB_PASSWD = "unittest";
const QString DatabaseTestScenario::MYSQL_HOST = "localhost";
const QString DatabaseTestScenario::MYSQL_DB = "unittest";
const int DatabaseTestScenario::MYSQL_PORT = 3306;
const QString DatabaseTestScenario::SQLITE_DB = "SqliteTestDB.db";

using namespace dbOverlay;
using namespace QTournament;

QSqlDatabase DatabaseTestScenario::getDbConn(dbOverlay::GenericDatabase::DB_ENGINE t)
{
  if (QSqlDatabase::contains(DB_CONNECTION_NAME))
  {
    QSqlDatabase db = QSqlDatabase::database(DB_CONNECTION_NAME, true);

    // despite providing the database name using setDatabaseName,
    // you must issue a USE SQL-command before actually accessing the database
    if (t == dbOverlay::GenericDatabase::MYSQL)
    {
      QSqlQuery qry(db);
      execQueryAndDumpError(qry, "USE " + MYSQL_DB);
    }
    
    return db;

  }

  QSqlDatabase db;
  
  if (t == dbOverlay::GenericDatabase::SQLITE)
  {
    db = QSqlDatabase::addDatabase("QSQLITE", DB_CONNECTION_NAME);

    db.setDatabaseName(getSqliteFileName());
    
  } else {
    
    db = QSqlDatabase::addDatabase("QMYSQL", DB_CONNECTION_NAME);
    db.setHostName(MYSQL_HOST);
    db.setDatabaseName(MYSQL_DB);
    db.setPort(MYSQL_PORT);
    db.setUserName(DB_USER);
    db.setPassword(DB_PASSWD);
  }
  
  if (!db.open())
  {
    QSqlError err = db.lastError();
    log.warn("db.open() for " + db.hostName() + " using driver " + db.driverName() + " failed with: " + err.text());
    CPPUNIT_ASSERT(false);
  } else {
    log.info("db.open() for " + db.hostName() + " using driver " + db.driverName() + " succeeded!");
  }
  
  // despite providing the database name using setDatabaseName,
  // you must issue a USE SQL-command before actually accessing the database
  if (t == dbOverlay::GenericDatabase::MYSQL)
  {
    QSqlQuery qry(db);
    execQueryAndDumpError(qry, "USE " + MYSQL_DB);
  }
  
  return db;
}

//----------------------------------------------------------------------------
    
QSqlDatabase DatabaseTestScenario::getDbConn()
{
  if (QSqlDatabase::contains(DB_CONNECTION_NAME))
  {
    QSqlDatabase db = QSqlDatabase::database(DB_CONNECTION_NAME, true);

    // despite providing the database name using setDatabaseName,
    // you must issue a USE SQL-command before actually accessing the database
    if (db.driverName() == "QMYSQL")
    {
      QSqlQuery qry(db);
      execQueryAndDumpError(qry, "USE " + MYSQL_DB);
    }
    
    return db;

  }
  
  return getDbConn(dbOverlay::GenericDatabase::SQLITE);

}

//----------------------------------------------------------------------------

void DatabaseTestScenario::cleanupMysql()
{
  QSqlDatabase db = getDbConn(dbOverlay::GenericDatabase::MYSQL);
  
  db.exec("DROP DATABASE " + MYSQL_DB);
  db.exec("CREATE DATABASE " + MYSQL_DB);
}

//----------------------------------------------------------------------------

void DatabaseTestScenario::removeDbConn()
{
  if (QSqlDatabase::contains(DB_CONNECTION_NAME))
  {
    QSqlDatabase::removeDatabase(DB_CONNECTION_NAME);
    CPPUNIT_ASSERT(!QSqlDatabase::contains(DB_CONNECTION_NAME));
    qDebug() << "Removed database " + DB_CONNECTION_NAME + " from connection pool";
  } else {
    qDebug() << "Weird: could not remove database " + DB_CONNECTION_NAME + " from connection pool, because it's not existing";
  }
}

//----------------------------------------------------------------------------

void DatabaseTestScenario::tearDown()
{
  BasicTestClass::tearDown();
  removeDbConn();
  DbTab::clearTabCache();
  
  qDebug() << "----------- DatabaseTestScenario tearDown() finished! -----------";
}

//----------------------------------------------------------------------------

QString DatabaseTestScenario::getSqliteFileName()
{
  return genTestFilePath(SQLITE_DB);
}

//----------------------------------------------------------------------------
    
bool DatabaseTestScenario::sqliteFileExists()
{
  QFile f(getSqliteFileName());
  return f.exists();
}

//----------------------------------------------------------------------------

void DatabaseTestScenario::prepScenario01(bool useTeams)
{
  TournamentSettings cfg;
  cfg.organizingClub = "club";
  cfg.tournamentName = "name";
  cfg.useTeams = useTeams;
  
  CPPUNIT_ASSERT(!sqliteFileExists());
  Tournament t(getSqliteFileName(), cfg);
  CPPUNIT_ASSERT(sqliteFileExists());
  
  t.close();
  
  // FIX: add scenario initialization code here!

}

//----------------------------------------------------------------------------

void DatabaseTestScenario::execQueryAndDumpError(QSqlQuery& qry, const QString& sqlStatement)
{
  bool ok;
  if (sqlStatement.length() != 0)
  {
    // simple statement
    ok = qry.exec(sqlStatement);
  } else {
    // prepared statement, prepared externally
    ok = qry.exec();
  }
  
  if (!ok)
  {
    QString msg = "The following SQL query failed: " + QString("\n");
    msg += "     " + qry.lastQuery() + QString("\n");
    msg += "     Error: " + qry.lastError().text()  + QString("\n");
    log.warn(msg);
    CPPUNIT_ASSERT(false);
  } else {
    
    int result;
    if (qry.isSelect())
    {
      result = qry.size();
    }
    else
    {
      result = qry.numRowsAffected();
    }
    
    QString msg = "The following SQL query was successfully executed: " + QString("\n");
    msg += "     " + qry.lastQuery() + QString("\n");
    msg += "     Rows affected: " + QString::number(result)  + QString("\n");
    log.info(msg);  

  }
}

//----------------------------------------------------------------------------

TournamentDB* DatabaseTestScenario::getScenario01(bool useTeams)
{
  prepScenario01(useTeams);
  return new TournamentDB(getSqliteFileName(), false);
}

//----------------------------------------------------------------------------
    
void DatabaseTestScenario::prepScenario02(bool useTeams)
{
  prepScenario01(useTeams);
  Tournament t(getSqliteFileName());
  
  // create a team some dummy players
  CPPUNIT_ASSERT(Tournament::getTeamMngr()->createNewTeam("t1") == OK);
  CPPUNIT_ASSERT(Tournament::getPlayerMngr()->createNewPlayer("f", "l1", M, "t1") == OK);
  CPPUNIT_ASSERT(Tournament::getPlayerMngr()->createNewPlayer("f", "l2", F, "t1") == OK);
  CPPUNIT_ASSERT(Tournament::getPlayerMngr()->createNewPlayer("f", "l3", M, "t1") == OK);
  CPPUNIT_ASSERT(Tournament::getPlayerMngr()->createNewPlayer("f", "l4", F, "t1") == OK);
  CPPUNIT_ASSERT(Tournament::getPlayerMngr()->createNewPlayer("f", "l5", M, "t1") == OK);
  CPPUNIT_ASSERT(Tournament::getPlayerMngr()->createNewPlayer("f", "l6", F, "t1") == OK);
  
  // create one category of every kind
  CatMngr* cmngr = Tournament::getCatMngr();
  CPPUNIT_ASSERT(cmngr->createNewCategory("MS") == OK);
  Category ms = cmngr->getCategory("MS");
  CPPUNIT_ASSERT(ms.setMatchType(SINGLES) == OK);
  CPPUNIT_ASSERT(ms.setSex(M) == OK);
  
  CPPUNIT_ASSERT(cmngr->createNewCategory("MD") == OK);
  Category md = cmngr->getCategory("MD");
  CPPUNIT_ASSERT(md.setMatchType(DOUBLES) == OK);
  CPPUNIT_ASSERT(md.setSex(M) == OK);
  
  CPPUNIT_ASSERT(cmngr->createNewCategory("LS") == OK);
  Category ls = cmngr->getCategory("LS");
  CPPUNIT_ASSERT(ls.setMatchType(SINGLES) == OK);
  CPPUNIT_ASSERT(ls.setSex(F) == OK);
  
  CPPUNIT_ASSERT(cmngr->createNewCategory("LD") == OK);
  Category ld = cmngr->getCategory("LD");
  CPPUNIT_ASSERT(ld.setMatchType(DOUBLES) == OK);
  CPPUNIT_ASSERT(ld.setSex(F) == OK);
  
  CPPUNIT_ASSERT(cmngr->createNewCategory("MX") == OK);
  Category mx = cmngr->getCategory("MX");
  CPPUNIT_ASSERT(mx.setMatchType(MIXED) == OK);
  CPPUNIT_ASSERT(mx.setSex(M) == OK);   // shouldn't matter at all
}

//----------------------------------------------------------------------------
    
TournamentDB* DatabaseTestScenario::getScenario02(bool useTeams)
{
  prepScenario02(useTeams);
  return new TournamentDB(getSqliteFileName(), false);
}


//----------------------------------------------------------------------------

void DatabaseTestScenario::prepScenario03(bool useTeams)
{
  prepScenario02(useTeams);
  Tournament t(getSqliteFileName());
  CatMngr* cmngr = Tournament::getCatMngr();

  // get some objects to play with
  Player m1 = Tournament::getPlayerMngr()->getPlayer("f", "l1");
  Player f1 = Tournament::getPlayerMngr()->getPlayer("f", "l2");
  Player m2 = Tournament::getPlayerMngr()->getPlayer("f", "l3");
  Player f2 = Tournament::getPlayerMngr()->getPlayer("f", "l4");
  Player m3 = Tournament::getPlayerMngr()->getPlayer("f", "l5");
  Player f3 = Tournament::getPlayerMngr()->getPlayer("f", "l6");
  Category md = cmngr->getCategory("MD");
  Category mx = cmngr->getCategory("MX");
  Category ms = cmngr->getCategory("MS");

  // assign players to categories
  CPPUNIT_ASSERT(cmngr->addPlayerToCategory(m1, md) == OK);
  CPPUNIT_ASSERT(cmngr->addPlayerToCategory(m2, md) == OK);
  
  CPPUNIT_ASSERT(cmngr->addPlayerToCategory(m1, ms) == OK);
  CPPUNIT_ASSERT(cmngr->addPlayerToCategory(m2, ms) == OK);
  
  CPPUNIT_ASSERT(cmngr->addPlayerToCategory(m1, mx) == OK);
  CPPUNIT_ASSERT(cmngr->addPlayerToCategory(m2, mx) == OK);
  CPPUNIT_ASSERT(cmngr->addPlayerToCategory(m3, mx) == OK);
  CPPUNIT_ASSERT(cmngr->addPlayerToCategory(f1, mx) == OK);
  CPPUNIT_ASSERT(cmngr->addPlayerToCategory(f2, mx) == OK);
  CPPUNIT_ASSERT(cmngr->addPlayerToCategory(f3, mx) == OK);
}    

//----------------------------------------------------------------------------

TournamentDB* DatabaseTestScenario::getScenario03(bool useTeams)
{
  prepScenario03(useTeams);
  return new TournamentDB(getSqliteFileName(), false);
}

//----------------------------------------------------------------------------
    
void DatabaseTestScenario::prepScenario04(bool useTeams)
{
  prepScenario03(useTeams);
  Tournament t(getSqliteFileName());
  CatMngr* cmngr = Tournament::getCatMngr();

  // get some objects to play with
  Player m1 = Tournament::getPlayerMngr()->getPlayer("f", "l1");
  Player f1 = Tournament::getPlayerMngr()->getPlayer("f", "l2");
  Player m2 = Tournament::getPlayerMngr()->getPlayer("f", "l3");
  Player f2 = Tournament::getPlayerMngr()->getPlayer("f", "l4");
  Player m3 = Tournament::getPlayerMngr()->getPlayer("f", "l5");
  Player f3 = Tournament::getPlayerMngr()->getPlayer("f", "l6");
  Category mx = cmngr->getCategory("MX");
  Category ms = cmngr->getCategory("MS");

  // pair players for mixed doubles
  CPPUNIT_ASSERT(cmngr->pairPlayers(mx, m1, f1) == OK);
  CPPUNIT_ASSERT(cmngr->pairPlayers(mx, m2, f2) == OK);
  CPPUNIT_ASSERT(cmngr->pairPlayers(mx, m3, f3) == OK);

  // fake a valid category state
  TournamentDB db{getSqliteFileName(), false};
  TabRow catRow = db[TAB_CATEGORY][5];
  catRow.update(GENERIC_STATE_FIELD_NAME, static_cast<int>(STAT_CAT_IDLE));
  CPPUNIT_ASSERT(mx.getState() == STAT_CAT_IDLE);

  // create a match group in mixed doubles
  ERR e;
  MatchMngr* mm = Tournament::getMatchMngr();
  auto mg = mm->createMatchGroup(mx, 1, 1, &e);
  CPPUNIT_ASSERT(e == OK);

  // create an "empty" match in this group
  mm->createMatch(*mg, &e);
  CPPUNIT_ASSERT(e == OK);
}

//----------------------------------------------------------------------------

TournamentDB* DatabaseTestScenario::getScenario04(bool useTeams)
{
  prepScenario04(useTeams);
  return new TournamentDB(getSqliteFileName(), false);
}

//----------------------------------------------------------------------------

// A brand new scenario with a group of 40 players, singles, and
// match groups for round robin phase already generated
void DatabaseTestScenario::prepScenario05(bool useTeams)
{
  TournamentSettings tSettings;
  tSettings.organizingClub = "club";
  tSettings.tournamentName = "name";
  tSettings.useTeams = useTeams;

  CPPUNIT_ASSERT(!sqliteFileExists());
  Tournament t(getSqliteFileName(), tSettings);
  CPPUNIT_ASSERT(sqliteFileExists());

  TeamMngr* tmngr = Tournament::getTeamMngr();
  PlayerMngr* pmngr = Tournament::getPlayerMngr();
  CatMngr* cmngr = Tournament::getCatMngr();

  // create one team and one category
  tmngr->createNewTeam("Team 1");
  cmngr->createNewCategory("MS");
  Category ms = cmngr->getCategory("MS");
  ms.setMatchType(SINGLES);
  ms.setSex(M);


  // create 40 players and assign them to MS
  for (int i=0; i < 40; ++i)
  {
      ERR e = pmngr->createNewPlayer("First", QString::number(i+1), M, "Team 1");
      CPPUNIT_ASSERT(e == OK);

      // the player's ID can be derived from "i"
      Player p = pmngr->getPlayer(i+1);

      // assign the player to MS
      e = cmngr->addPlayerToCategory(p, ms);
      CPPUNIT_ASSERT(e == OK);
  }

  // create and set a valid group configuration for MS
  GroupDef d = GroupDef(5, 8);
  GroupDefList gdl;
  gdl.append(d);
  KO_Config cfg(QUARTER, false, gdl);
  ms.setParameter(GROUP_CONFIG, cfg.toString());

  // run the category
  unique_ptr<Category> specialCat = ms.convertToSpecializedObject();
  ERR e = cmngr->freezeConfig(ms);
  CPPUNIT_ASSERT(e == OK);

  // fake a list of player-pair-lists for the group assignments
  QList<PlayerPairList> ppListList;
  for (int grpNum=0; grpNum < 8; ++grpNum)
  {
      PlayerPairList thisGroup;
      for (int pNum=0; pNum < 5; ++pNum)
      {
          int id = (grpNum * 5) + pNum + 1;

          // in this special setting, the player's ID and the
          // ID of the PlayerPair should be identical
          Player p = pmngr->getPlayer(id);
          PlayerPair pp(p, id);
          thisGroup.append(pp);
      }
      ppListList.append(thisGroup);
  }

  // make sure the faked group assignment is valid
  e = specialCat->canApplyGroupAssignment(ppListList);
  CPPUNIT_ASSERT(e == OK);

  // prepare an empty list for the (not required) initial ranking
  PlayerPairList initialRanking;

  // actually run the category
  e = cmngr->startCategory(ms, ppListList, initialRanking);
  CPPUNIT_ASSERT(e == OK);

  // done
}

//----------------------------------------------------------------------------
    
TournamentDB* DatabaseTestScenario::getScenario05(bool useTeams)
{
  prepScenario05(useTeams);
  return new TournamentDB(getSqliteFileName(), false);
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
    

//----------------------------------------------------------------------------
    
