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
    QString stateFieldDef = GENERIC_STATE_FIELD_NAME + " INTEGER";
    
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
    cols << stateFieldDef;
    cols << PL_SEX + " INTEGER";
    //cols << PL_ + " ";
    //cols << PL_ + " ";
    //cols << PL_ + " ";
    //cols << PL_ + " ";
    //cols << PL_ + " ";
    //cols << PL_ + " ";
    //cols << PL_ + " ";
    //cols << PL_ + " ";
    //cols << PL_ + " ";
    cols << genForeignKeyClause(PL_TEAM_REF, TAB_TEAM);
    tableCreationHelper(TAB_PLAYER, cols);
    
    // Generate the table holding the category data
    cols.clear();
    cols << nameFieldDef;
    cols << CAT_MATCH_TYPE + " INTEGER";
    cols << CAT_SEX + " INTEGER";
    cols << CAT_SYS + " INTEGER";
    cols << CAT_ACCEPT_DRAW + " INTEGER";
    cols << stateFieldDef;
    //cols << CAT_ + " ";
    //cols << CAT_ + " ";
    //cols << CAT_ + " ";
    //cols << CAT_ + " ";
    //cols << CAT_ + " ";
    //cols << CAT_ + " ";
    //cols << CAT_ + " ";
    //cols << CAT_ + " ";
    //cols << CAT_ + " ";
    //cols << CAT_ + " ";
    tableCreationHelper(TAB_CATEGORY, cols);
    
    // Generate the table holding the player-to-category mapping
    cols.clear();
    cols << genForeignKeyClause(P2C_PLAYER_REF, TAB_PLAYER);
    cols << genForeignKeyClause(P2C_CAT_REF, TAB_CATEGORY);
    tableCreationHelper(TAB_P2C, cols);
}

void TournamentDB::populateViews()
{
    
}

}
