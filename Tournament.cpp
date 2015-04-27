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
  TeamMngr* Tournament::tm = nullptr;
  CatMngr* Tournament::cm = nullptr;
  PlayerMngr* Tournament::pm = nullptr;
  TeamListModel* Tournament::tlm = nullptr;
  PlayerTableModel* Tournament::ptm = nullptr;
  CategoryTableModel* Tournament::ctm = nullptr;
  MatchMngr* Tournament::mm = nullptr;
  MatchGroupTableModel* Tournament::mgm = nullptr;
  MatchTableModel* Tournament::mam = nullptr;
  CourtMngr* Tournament::com = nullptr;
  CourtTableModel* Tournament::courtMod = nullptr;
  RankingMngr* Tournament::rm = nullptr;
  ReportFactory* Tournament::repFab = nullptr;

/**
 * Constructor for a new, empty tournament file
 * 
 * @param fName name of the file to create; the file may not exist
 * @param cfg initial configuration settings for the application
 */
Tournament::Tournament(const QString& fName, const TournamentSettings& cfg)
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
    db = new TournamentDB(fName, true);
    db->setLogLevel(1);

    // initialize the database
    KeyValueTab cfgTab = KeyValueTab::getTab(db, TAB_CFG);
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
 * @param fName name of the file to create; the file must exist
 */
Tournament::Tournament(const QString& fName)
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
    db = new TournamentDB(fName, false);
    db->setLogLevel(1);
    
    // initialize the various managers / handlers
    initManagers();
    
    // always initialize the managers before the models,
    // because the models connect to signals emitted by the
    // managers
    initModels();
}

//----------------------------------------------------------------------------

void Tournament::initManagers()
{
    tm = new TeamMngr(db);
    cm = new CatMngr(db);
    pm = new PlayerMngr(db);
    mm = new MatchMngr(db);
    com = new CourtMngr(db);
    rm = new RankingMngr(db);
    repFab = new ReportFactory(db);

    // wire some signals between managers
    connect(pm, &PlayerMngr::playerStatusChanged, mm, &MatchMngr::onPlayerStatusChanged, Qt::DirectConnection);
}

//----------------------------------------------------------------------------

void Tournament::initModels()
{
  tlm = new TeamListModel(db);
  ptm = new PlayerTableModel(db);
  ctm = new CategoryTableModel(db);
  mgm = new MatchGroupTableModel(db);
  mam = new MatchTableModel(db);
  courtMod = new CourtTableModel(db);
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
  delete mm;
  delete com;
  delete rm;
  delete repFab;
  
  delete tlm;
  delete ptm;
  delete ctm;
  delete mgm;
  delete mam;
  delete courtMod;
  
  tm = nullptr;
  cm = nullptr;
  pm = nullptr;
  mm = nullptr;
  tlm = nullptr;
  ptm = nullptr;
  ctm = nullptr;
  mgm = nullptr;
  mam = nullptr;
  com = nullptr;
  courtMod = nullptr;
  rm = nullptr;
  repFab = nullptr;
  
  db->close();
  delete db;
  db = nullptr;
}

//----------------------------------------------------------------------------

Tournament::~Tournament()
{
  // close will be called separately by the UI
  //close();

  // just to be sure, although should have been
  // handled by close()
  if (db)
  {
    db->close();
    delete db;
    db = nullptr;
  }
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

CategoryTableModel* Tournament::getCategoryTableModel()
{
  return ctm;
}

//----------------------------------------------------------------------------
    
MatchMngr* Tournament::getMatchMngr()
{
  return mm;
}

//----------------------------------------------------------------------------

MatchGroupTableModel* Tournament::getMatchGroupTableModel()
{
  return mgm;
}

//----------------------------------------------------------------------------
    
MatchTableModel* Tournament::getMatchTableModel()
{
  return mam;
}

//----------------------------------------------------------------------------

CourtMngr* Tournament::getCourtMngr()
{
  return com;
}

//----------------------------------------------------------------------------

CourtTableModel* Tournament::getCourtTableModel()
{
  return courtMod;
}

//----------------------------------------------------------------------------

RankingMngr* Tournament::getRankingMngr()
{
  return rm;
}

//----------------------------------------------------------------------------

ReportFactory* Tournament::getReportFactory()
{
  return repFab;
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
    

}
