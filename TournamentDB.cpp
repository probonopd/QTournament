/* 
 * File:   TournamentDB.cpp
 * Author: nyoknvk1
 * 
 * Created on 18. Februar 2014, 12:33
 */

#include "TournamentDB.h"
#include "TournamentDataDefs.h"
#include "KeyValueTab.h"

using namespace dbOverlay;

namespace QTournament
{

TournamentDB::TournamentDB(QString fName, bool createNew)
: GenericDatabase(fName, createNew)
{
    populateTables();
    populateViews();
};

void TournamentDB::populateTables()
{
    QStringList cols;
    
    // Generate the key-value-table with the tournament config
    dbOverlay::KeyValueTab::getTab(this, TAB_CFG);
    
    // Generate the table holding the teams
    cols.clear();
    cols << GENERIC_NAME_FIELD_NAME + " VARCHAR(100)";
    //cols << TE_ + " ";
    //cols << TE_ + " ";
    //cols << TE_ + " ";
    //cols << TE_ + " ";
    //cols << TE_ + " ";
    //cols << TE_ + " ";
    tableCreationHelper(TAB_TEAM, cols);
    
    // Generate the table hosting the players
    cols.clear();
    cols << PL_FNAME + " VARCHAR(30)";
    cols << PL_LNAME + " VARCHAR(30)";
    cols << genForeignKeyClause(PL_TEAM_REF, TAB_TEAM);
    //cols << PL_ + " ";
    //cols << PL_ + " ";
    //cols << PL_ + " ";
    //cols << PL_ + " ";
    //cols << PL_ + " ";
    //cols << PL_ + " ";
    //cols << PL_ + " ";
    //cols << PL_ + " ";
    //cols << PL_ + " ";
    tableCreationHelper(TAB_PLAYER, cols);
}

void TournamentDB::populateViews()
{
    
}

}
