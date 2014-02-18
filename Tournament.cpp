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

/**
 * Constructor for a new, empty tournament file
 * 
 * @param fName name of the file to create; the file may not exists
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
}

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
}


}
