/* 
 * File:   Tournament.cpp
 * Author: nyoknvk1
 * 
 * Created on 18. Februar 2014, 12:31
 */

#include "Tournament.h"
#include "HelperFunc.h"
#include "KeyValueTab.h"
#include <QString>
#include <QFile>
#include <stdexcept>

using namespace dbOverlay;

namespace QTournament
{
  TeamMngr* Tournament::tm = NULL;
  CatMngr* Tournament::cm = NULL;
  PlayerMngr* Tournament::pm = NULL;
  TeamListModel* Tournament::tlm = NULL;
  PlayerTableModel* Tournament::ptm = NULL;

/**
 * Constructor for a new, empty tournament file
 * 
 * @param fName name of the file to create; the file may not exist
 * @param cfg initial configuration settings for the application
 */
Tournament::Tournament(const QString& fName, const TournamentSettings& cfg)
: db(TournamentDB(":memory:", true))    // dummy initializer for satisfying the compiler
{
    // Check whether the file exists
    QFile f(fName);
    if (f.exists())
    {
        QString msg = "Tournament ctor: ";
        msg += "file " + fName + " already exists in the file system!";
        throw std::invalid_argument(dbOverlay::QString2String(msg));
    }
    
    // create a new, blank database
    db = TournamentDB(fName, true);

    // initialize the database
    KeyValueTab cfgTab = KeyValueTab::getTab(&db, TAB_CFG);
    cfgTab.set(CFG_KEY_DB_VERSION, DB_VERSION);
    cfgTab.set(CFG_KEY_TNMT_NAME, cfg.tournamentName);
    cfgTab.set(CFG_KEY_TNMT_ORGA, cfg.organizingClub);
    cfgTab.set(CFG_KEY_USE_TEAMS, cfg.useTeams);
    
    // initialize the various managers / handlers
    initManagers();
    
    // always initialize the managers before the models,
    // because the models connect to signals emitted by the
    // managers
    initModels();
}

//----------------------------------------------------------------------------
    
/**
 * Constructor for an existing tournament file
 * 
 * @param fName name of the file to create; the file may not exist
 */
Tournament::Tournament(const QString& fName)
: db(TournamentDB(":memory:", true))    // dummy initializer for satisfying the compiler
{
    // Check whether the file exists
    QFile f(fName);
    if (!(f.exists()))
    {
        QString msg = "Tournament ctor: ";
        msg += "file " + fName + " does not exist in the file system!";
        throw std::invalid_argument(dbOverlay::QString2String(msg));
    }
    
    // open an existing database
    db = TournamentDB(fName, false);
    
    // initialize the various managers / handlers and models
    initManagers();
    initModels();
}

//----------------------------------------------------------------------------

void Tournament::initManagers()
{
    tm = new TeamMngr(&db);
    cm = new CatMngr(&db);
    pm = new PlayerMngr(&db);
}

//----------------------------------------------------------------------------

void Tournament::initModels()
{
  tlm = new TeamListModel(&db);
  ptm = new PlayerTableModel(&db);
}

//----------------------------------------------------------------------------

void Tournament::close()
{
  // announce that we're about to close
  // IMPORTANT: the mamagers and models should
  // stll be valid when emitting the signal,
  // so that receivers can properly disconnect
  emit tournamentClosed();
    
  delete tm;
  delete cm;
  delete pm;
  
  delete tlm;
  
  tm = NULL;
  cm = NULL;
  pm = NULL;
  tlm = NULL;
  
  db.close();
}

//----------------------------------------------------------------------------

Tournament::~Tournament()
{
  close();
}

//----------------------------------------------------------------------------

TeamMngr* Tournament::getTeamMngr()
{
  return tm;
}

//----------------------------------------------------------------------------

CatMngr* Tournament::getCatMngr()
{
  return cm;
}

//----------------------------------------------------------------------------

PlayerMngr* Tournament::getPlayerMngr()
{
  return pm;
}

//----------------------------------------------------------------------------

TeamListModel* Tournament::getTeamListModel()
{
  return tlm;
}

//----------------------------------------------------------------------------

PlayerTableModel* Tournament::getPlayerTableModel()
{
  return ptm;
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
    

}
