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
#include <QStringList>

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
    cols << CAT_BRACKET_VIS_DATA + " VARCHAR(50)";
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
    cols << MA_START_TIME + " VARCHAR(50)";
    cols << MA_ADDITIONAL_CALL_TIMES + " VARCHAR(50)";  // up 3 times in seconds since epoch as string
    cols << MA_FINISH_TIME + " VARCHAR(50)";
    cols << MA_PAIR1_SYMBOLIC_VAL + " INTEGER";
    cols << MA_PAIR2_SYMBOLIC_VAL + " INTEGER";
    cols << MA_WINNER_RANK + " INTEGER";
    cols << MA_LOSER_RANK + " INTEGER";
    tableCreationHelper(TAB_MATCH, cols);

    // Generate a table with ranking information
    cols.clear();
    cols << RA_ROUND + " INTEGER";
    cols << genForeignKeyClause(RA_PAIR_REF, TAB_PAIRS);
    cols << genForeignKeyClause(RA_CAT_REF, TAB_CATEGORY);  // this eases searching in the tab. theoretically, the category could be derived from the player pair
    cols << RA_GRP_NUM + " INTEGER";   // this eases searching in the tab. theoretically, the group number could be derived from the player pair
    cols << RA_GAMES_WON  + " INTEGER";
    cols << RA_GAMES_LOST  + " INTEGER";
    cols << RA_MATCHES_WON  + " INTEGER";
    cols << RA_MATCHES_LOST  + " INTEGER";
    cols << RA_MATCHES_DRAW  + " INTEGER";
    cols << RA_POINTS_WON  + " INTEGER";
    cols << RA_POINTS_LOST  + " INTEGER";
    cols << RA_RANK  + " INTEGER";
    tableCreationHelper(TAB_RANKING, cols);

    // Generate a table with bracket visualization data
    cols.clear();
    cols << genForeignKeyClause(BV_MATCH_REF, TAB_MATCH);
    cols << genForeignKeyClause(BV_CAT_REF, TAB_CATEGORY);
    cols << BV_PAGE + " INTEGER";
    cols << BV_GRID_X0 + " INTEGER";
    cols << BV_GRID_Y0 + " INTEGER";
    cols << BV_SPAN_Y + " INTEGER";
    cols << BV_ORIENTATION + " INTEGER";
    cols << BV_TERMINATOR + " INTEGER";
    cols << BV_INITIAL_RANK1 + " INTEGER";
    cols << BV_INITIAL_RANK2 + " INTEGER";
    cols << BV_Y_PAGEBREAK_SPAN + " INTEGER";
    cols << BV_NEXT_PAGE_NUM + " INTEGER";
    cols << BV_TERMINATOR_OFFSET_Y + " INTEGER";
    cols << BV_ELEMENT_ID + " INTEGER";
    cols << BV_NEXT_MATCH_POS_FOR_WINNER + " INTEGER";
    cols << BV_NEXT_MATCH_POS_FOR_LOSER + " INTEGER";
    cols << BV_NEXT_LOSER_MATCH + " INTEGER";
    cols << BV_NEXT_WINNER_MATCH + " INTEGER";
    cols << genForeignKeyClause(BV_PAIR1_REF, TAB_PAIRS);
    cols << genForeignKeyClause(BV_PAIR2_REF, TAB_PAIRS);
    tableCreationHelper(TAB_BRACKET_VIS, cols);
}

//----------------------------------------------------------------------------

void TournamentDB::populateViews()
{

}

//----------------------------------------------------------------------------

void TournamentDB::createIndices()
{
  indexCreationHelper(TAB_COURT, GENERIC_NAME_FIELD_NAME);
  indexCreationHelper(TAB_COURT, GENERIC_SEQNUM_FIELD_NAME, true);

  indexCreationHelper(TAB_TEAM, GENERIC_NAME_FIELD_NAME, true);
  indexCreationHelper(TAB_TEAM, GENERIC_SEQNUM_FIELD_NAME, true);

  QStringList colList{PL_LNAME, PL_FNAME};
  indexCreationHelper(TAB_PLAYER, GENERIC_SEQNUM_FIELD_NAME, true);
  indexCreationHelper(TAB_PLAYER, GENERIC_STATE_FIELD_NAME);
  indexCreationHelper(TAB_PLAYER, PL_FNAME);
  indexCreationHelper(TAB_PLAYER, PL_LNAME);
  indexCreationHelper(TAB_PLAYER, "Player_CombinedNames", colList, true);

  indexCreationHelper(TAB_CATEGORY, GENERIC_NAME_FIELD_NAME, true);
  indexCreationHelper(TAB_CATEGORY, GENERIC_SEQNUM_FIELD_NAME, true);
  indexCreationHelper(TAB_CATEGORY, GENERIC_STATE_FIELD_NAME);

  indexCreationHelper(TAB_P2C, P2C_CAT_REF);
  indexCreationHelper(TAB_P2C, P2C_PLAYER_REF);

  indexCreationHelper(TAB_PAIRS, PAIRS_PLAYER1_REF);
  indexCreationHelper(TAB_PAIRS, PAIRS_PLAYER1_REF);
  indexCreationHelper(TAB_PAIRS, PAIRS_CAT_REF);

  indexCreationHelper(TAB_MATCH_GROUP, MG_CAT_REF);
  indexCreationHelper(TAB_MATCH_GROUP, MG_GRP_NUM);
  indexCreationHelper(TAB_MATCH_GROUP, MG_ROUND);
  indexCreationHelper(TAB_MATCH_GROUP, MG_STAGE_SEQ_NUM, true);

  indexCreationHelper(TAB_MATCH, MA_GRP_REF);
  indexCreationHelper(TAB_MATCH, GENERIC_SEQNUM_FIELD_NAME, true);
  indexCreationHelper(TAB_MATCH, GENERIC_STATE_FIELD_NAME);
  indexCreationHelper(TAB_MATCH, MA_PAIR1_REF);
  indexCreationHelper(TAB_MATCH, MA_PAIR2_REF);

  indexCreationHelper(TAB_RANKING, RA_PAIR_REF);
  indexCreationHelper(TAB_RANKING, RA_CAT_REF);
  indexCreationHelper(TAB_RANKING, RA_ROUND);

  indexCreationHelper(TAB_BRACKET_VIS, BV_CAT_REF);
  indexCreationHelper(TAB_BRACKET_VIS, BV_MATCH_REF);
  indexCreationHelper(TAB_BRACKET_VIS, BV_PAIR1_REF);
  indexCreationHelper(TAB_BRACKET_VIS, BV_PAIR2_REF);

  //indexCreationHelper(TAB_, );
}

//----------------------------------------------------------------------------

tuple<int, int> TournamentDB::getVersion()
{
  KeyValueTab cfg = KeyValueTab::getTab(this, TAB_CFG);

  // return an error if no version information is stored in the database
  if (!(cfg.hasKey(CFG_KEY_DB_VERSION)))
  {
    return make_tuple(-1, -1);
  }

  // get the raw version string
  QString versionString = cfg.getString(CFG_KEY_DB_VERSION);
  if (versionString.isEmpty())
  {
    return make_tuple(-1, -1);
  }

  // try to split it into major / minor
  int major;
  int minor;
  bool isOkay;
  auto col = versionString.split(".");
  major = col[0].toInt(&isOkay);
  if (!isOkay)
  {
    return make_tuple(-1, -1);
  }
  if (col.length() > 1)
  {
    minor = col[1].toInt(&isOkay);
    if (!isOkay)
    {
      return make_tuple(major, -1);
    }
  } else {
    // this is a workaround for old databases
    // that did only store a major version number
    // and no minor version number
    minor = 0;
  }

  return make_tuple(major, minor);
}

//----------------------------------------------------------------------------

bool TournamentDB::isCompatibleDatabaseVersion()
{
  // get the current file format version
  int major;
  int minor;
  tie(major, minor) = getVersion();

  // condition: major version number must
  // match the compiled in version and the minor
  // version number must be less or equal to
  // the version at compile time
  return ((major == DB_VERSION_MAJOR) && (minor >= 0) && (minor <= DB_VERSION_MINOR));
}

//----------------------------------------------------------------------------

bool TournamentDB::needsConversion()
{
  // get the current file format version
  int major;
  int minor;
  tie(major, minor) = getVersion();

  // condition: major version number must
  // match the compiled in version and the minor
  // version number must be less then
  // the version at compile time
  return ((major == DB_VERSION_MAJOR) && (minor >= 0) && (minor < DB_VERSION_MINOR));
}

//----------------------------------------------------------------------------

bool TournamentDB::convertToLatestDatabaseVersion()
{
  // get the current file format version
  int major;
  int minor;
  tie(major, minor) = getVersion();

  if (major != DB_VERSION_MAJOR) return false;
  if (minor < 0) return false;
  if (minor > DB_VERSION_MINOR) return false;

  // convert from 2.0 to 2.1
  if (minor == 0)
  {
    createIndices();
    minor = 1;
  }

  // store the new database version
  QString dbVersion = "%1.%2";
  dbVersion = dbVersion.arg(DB_VERSION_MAJOR);
  dbVersion = dbVersion.arg(minor);
  KeyValueTab cfg = KeyValueTab::getTab(this, TAB_CFG);
  cfg.set(CFG_KEY_DB_VERSION, dbVersion);

  return true;
}

}
