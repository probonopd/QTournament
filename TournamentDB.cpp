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
    QString seqNumFieldDef = GENERIC_SEQNUM_FIELD_NAME + " INTEGER";
    
    // Generate the key-value-table with the tournament config
    dbOverlay::KeyValueTab::getTab(this, TAB_CFG);
    
    // Generate the table for the courts
    cols.clear();
    cols << nameFieldDef;
    cols << stateFieldDef;
    cols << seqNumFieldDef;
    cols << CO_NUMBER + " INTEGER";
    cols << CO_IS_MANUAL_ASSIGNMENT + " INTEGER";
    tableCreationHelper(TAB_COURT, cols);
    
    // Generate the table holding the teams
    cols.clear();
    cols << nameFieldDef;
    cols << seqNumFieldDef;
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
    cols << seqNumFieldDef;
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
    cols << seqNumFieldDef;
    cols << CAT_WIN_SCORE + " INTEGER";
    cols << CAT_DRAW_SCORE + " INTEGER";
    cols << CAT_GROUP_CONFIG + " VARCHAR(50)";
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
    
    // Generate the table holding the player pairs
    cols.clear();
    cols << genForeignKeyClause(PAIRS_PLAYER1_REF, TAB_PLAYER);
    cols << genForeignKeyClause(PAIRS_PLAYER2_REF, TAB_PLAYER);
    cols << genForeignKeyClause(PAIRS_CAT_REF, TAB_CATEGORY);
    cols << PAIRS_GRP_NUM + " INTEGER";
    cols << PAIRS_INITIAL_RANK + " INTEGER";
    tableCreationHelper(TAB_PAIRS, cols);
    
    // Generate a table for the match groups
    cols.clear();
    cols << genForeignKeyClause(MG_CAT_REF, TAB_CATEGORY);
    cols << MG_ROUND + " INTEGER";
    cols << MG_GRP_NUM + " INTEGER";
    cols << seqNumFieldDef;
    cols << stateFieldDef;
    cols << MG_STAGE_SEQ_NUM + " INTEGER";
    tableCreationHelper(TAB_MATCH_GROUP, cols);
    
    // Generate a table for matches
    cols.clear();
    cols << genForeignKeyClause(MA_GRP_REF, TAB_MATCH_GROUP);
    cols << MA_NUM + " INTEGER";
    cols << stateFieldDef;
    cols << seqNumFieldDef;
    cols << genForeignKeyClause(MA_PAIR1_REF, TAB_PAIRS);
    cols << genForeignKeyClause(MA_PAIR2_REF, TAB_PAIRS);
    cols << genForeignKeyClause(MA_ACTUAL_PLAYER1A_REF, TAB_PLAYER);
    cols << genForeignKeyClause(MA_ACTUAL_PLAYER1B_REF, TAB_PLAYER);
    cols << genForeignKeyClause(MA_ACTUAL_PLAYER2A_REF, TAB_PLAYER);
    cols << genForeignKeyClause(MA_ACTUAL_PLAYER2B_REF, TAB_PLAYER);
    cols << MA_RESULT + " VARCHAR(50)";
    cols << genForeignKeyClause(MA_COURT_REF, TAB_COURT);
    tableCreationHelper(TAB_MATCH, cols);
}

void TournamentDB::populateViews()
{
    
}

}
