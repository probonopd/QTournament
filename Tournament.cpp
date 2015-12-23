/*
 *    This is QTournament, a badminton tournament management program.
 *    Copyright (C) 2014 - 2015  Volker Knollmann
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QString>
#include <QFile>
#include <stdexcept>

#include "SqliteOverlay/KeyValueTab.h"

#include "Tournament.h"
#include "HelperFunc.h"
#include "SignalRelay.h"
#include "models/CatTableModel.h"
#include "PlayerMngr.h"

namespace QTournament
{
  Tournament* Tournament::activeTournament = nullptr;

  Tournament::Tournament(unique_ptr<TournamentDB> dbHandle)
  {
    // Check whether the database handle is valid
    if (dbHandle == nullptr)
    {
      throw std::invalid_argument("Received nullptr for database connection in Tournament ctor");
    }

    // take ownership of the database object
    db = std::move(dbHandle);
    
    // initialize the various managers / handlers
    initManagers();
    
    // always initialize the managers before the models,
    // because the models connect to signals emitted by the
    // managers
    initModels();
  }

  //----------------------------------------------------------------------------

  /**
 * Creator for a new, empty tournament file
 *
 * @param fName name of the file to create; the file may not exist
 * @param cfg initial configuration settings for the application
 */
  unique_ptr<Tournament> Tournament::createNew(const QString& fName, const TournamentSettings& cfg, ERR* err)
  {
    // Check whether the file exists
    QFile f(fName);
    if (f.exists())
    {
      if (err != nullptr) *err = FILE_ALREADY_EXISTS;
      return nullptr;
    }

    // create a new, blank database
    auto newDb = SqliteOverlay::SqliteDatabase::get<TournamentDB>(QString2StdString(fName), true);
    newDb->setLogLevel(1);
    newDb->createIndices();

    // initialize the database
    auto cfgTab = SqliteOverlay::KeyValueTab::getTab(newDb.get(), TAB_CFG);
    QString dbVersion = "%1.%2";
    dbVersion = dbVersion.arg(DB_VERSION_MAJOR).arg(DB_VERSION_MINOR);
    cfgTab->set(CFG_KEY_DB_VERSION, QString2StdString(dbVersion));
    cfgTab->set(CFG_KEY_TNMT_NAME, QString2StdString(cfg.tournamentName));
    cfgTab->set(CFG_KEY_TNMT_ORGA, QString2StdString(cfg.organizingClub));
    cfgTab->set(CFG_KEY_USE_TEAMS, cfg.useTeams);

    // construct a new tournament object for this database
    auto newTnmt = unique_ptr<Tournament>(new Tournament(std::move(newDb)));

    // return the new tournament pointer
    if (err != nullptr) *err = OK;
    return newTnmt;
  }

  //----------------------------------------------------------------------------

  /**
 * Creator for an existing tournament file
 *
 * @param fName name of the file to create; the file must exist
 */
  unique_ptr<Tournament> Tournament::openExisting(const QString& fName, ERR* err)
  {
    // Check whether the file exists
    QFile f(fName);
    if (!(f.exists()))
    {
      if (err != nullptr) *err = FILE_NOT_EXISTING;
      return nullptr;
    }

    // open an existing database
    auto newDb = SqliteOverlay::SqliteDatabase::get<TournamentDB>(QString2StdString(fName), false);
    newDb->setLogLevel(1);

    // check file format compatibiliy
    if (!(newDb->isCompatibleDatabaseVersion()))
    {
      if (err != nullptr) *err = INCOMPATIBLE_FILE_FORMAT;
      return nullptr;

      // the newly created database object is automatically
      // destroyed when leaving the scope
    }

    // construct a new tournament object for this database
    auto newTnmt = unique_ptr<Tournament>(new Tournament(std::move(newDb)));

    // return the new tournament pointer
    if (err != nullptr) *err = OK;
    return newTnmt;
  }

  //----------------------------------------------------------------------------

  void Tournament::setActiveTournament(Tournament* newTnmt)
  {
    if (newTnmt == nullptr) return;

    // set the global Tournament-pointer to the new tournament
    activeTournament = newTnmt;
  }

  //----------------------------------------------------------------------------

  void Tournament::initManagers()
  {
    tm = make_unique<TeamMngr>(db.get());
    cm = make_unique<CatMngr>(db.get());
    pm = make_unique<PlayerMngr>(db.get());
    mm = make_unique<MatchMngr>(db.get());
    com = make_unique<CourtMngr>(db.get());
    rm = make_unique<RankingMngr>(db.get());
    repFab = make_unique<ReportFactory>(db.get());

    // wire some signals between managers
    connect(pm.get(), SIGNAL(playerStatusChanged(int, int, OBJ_STATE, OBJ_STATE)), mm.get(), SLOT(onPlayerStatusChanged(int, int, OBJ_STATE, OBJ_STATE)), Qt::DirectConnection);
  }

  //----------------------------------------------------------------------------

  void Tournament::initModels()
  {
    tlm = unique_ptr<TeamListModel>(new TeamListModel(this));
    ptm = unique_ptr<PlayerTableModel>(new PlayerTableModel(this));
    ctm = unique_ptr<CategoryTableModel>(new CategoryTableModel(this));
    mgm = unique_ptr<MatchGroupTableModel>(new MatchGroupTableModel(this));
    mam = unique_ptr<MatchTableModel>(new MatchTableModel(this));
    courtMod = unique_ptr<CourtTableModel>(new CourtTableModel(this));
  }

  //----------------------------------------------------------------------------

  void Tournament::close()
  {
    // announce that we're about to close
    // IMPORTANT: the mamagers and models should
    // stll be valid when emitting the signal,
    // so that receivers can properly disconnect
    emit tournamentClosed();
    
    tm.reset();
    cm.reset();
    pm.reset();
    mm.reset();
    com.reset();
    rm.reset();
    repFab.reset();

    tlm.reset();
    ptm.reset();
    ctm.reset();
    mgm.reset();
    mam.reset();
    courtMod.reset();

    db->close();
    db.reset();

    // invalidate the global pointer
    activeTournament = nullptr;

    //SignalRelay::cleanUp();
  }

  //----------------------------------------------------------------------------

  Tournament::~Tournament()
  {
    // just to be sure, although should have been
    // handled by close()
    if (db)
    {
      close();
    }
  }

  //----------------------------------------------------------------------------

  TeamMngr* Tournament::getTeamMngr()
  {
    return tm.get();
  }

  //----------------------------------------------------------------------------

  CatMngr* Tournament::getCatMngr()
  {
    return cm.get();
  }

  //----------------------------------------------------------------------------

  PlayerMngr* Tournament::getPlayerMngr()
  {
    return pm.get();
  }

  //----------------------------------------------------------------------------

  TeamListModel* Tournament::getTeamListModel()
  {
    return tlm.get();
  }

  //----------------------------------------------------------------------------

  PlayerTableModel* Tournament::getPlayerTableModel()
  {
    return ptm.get();
  }

  //----------------------------------------------------------------------------

  CategoryTableModel* Tournament::getCategoryTableModel()
  {
    return ctm.get();
  }

  //----------------------------------------------------------------------------

  MatchMngr* Tournament::getMatchMngr()
  {
    return mm.get();
  }

  //----------------------------------------------------------------------------

  MatchGroupTableModel* Tournament::getMatchGroupTableModel()
  {
    return mgm.get();
  }

  //----------------------------------------------------------------------------

  MatchTableModel* Tournament::getMatchTableModel()
  {
    return mam.get();
  }

  //----------------------------------------------------------------------------

  CourtMngr* Tournament::getCourtMngr()
  {
    return com.get();
  }

  //----------------------------------------------------------------------------

  CourtTableModel* Tournament::getCourtTableModel()
  {
    return courtMod.get();
  }

  //----------------------------------------------------------------------------

  RankingMngr* Tournament::getRankingMngr()
  {
    return rm.get();
  }

  //----------------------------------------------------------------------------

  ReportFactory* Tournament::getReportFactory()
  {
    return repFab.get();
  }

  //----------------------------------------------------------------------------

  TournamentDB*Tournament::getDatabaseHandle()
  {
    return db.get();
  }

  //----------------------------------------------------------------------------

  bool Tournament::hasActiveTournament()
  {
    return (activeTournament != nullptr);
  }

  //----------------------------------------------------------------------------

  Tournament*Tournament::getActiveTournament()
  {
    return activeTournament;
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


}
