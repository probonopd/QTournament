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
    QString nameTypeDef = " VARCHAR(" + QString::number(MAX_NAME_LEN) + ")";
    QString nameFieldDef = GENERIC_NAME_FIELD_NAME + nameTypeDef;
    
    // Generate the key-value-table with the tournament config
    dbOverlay::KeyValueTab::getTab(this, TAB_CFG);
    
    // Generate the table holding the teams
    cols.clear();
    cols << nameFieldDef;
    //cols << TE_ + " ";
    //cols << TE_ + " ";
    //cols << TE_ + " ";
    //cols << TE_ + " ";
    //cols << TE_ + " ";
    //cols << TE_ + " ";
    tableCreationHelper(TAB_TEAM, cols);
    
    // Generate the table hosting the players
    cols.clear();
    cols << PL_FNAME + nameTypeDef;
    cols << PL_LNAME + nameTypeDef;;
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
