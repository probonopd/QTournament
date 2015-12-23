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

#include "SqliteOverlay/TableCreator.h"
#include "SqliteOverlay/KeyValueTab.h"

#include "TournamentDB.h"
#include "TournamentDataDefs.h"
#include "HelperFunc.h"

namespace QTournament
{

TournamentDB::TournamentDB(string fName, bool createNew)
  : SqliteOverlay::SqliteDatabase(fName, createNew)
{
}

void TournamentDB::populateTables()
{
    SqliteOverlay::TableCreator tc{this};
    
    // Generate the key-value-table with the tournament config
    SqliteOverlay::KeyValueTab::getTab(this, TAB_CFG);
    
    // Generate the table for the courts
    tc.addVarchar(GENERIC_NAME_FIELD_NAME, MAX_NAME_LEN);
    tc.addInt(GENERIC_STATE_FIELD_NAME);
    tc.addInt(GENERIC_SEQNUM_FIELD_NAME);
    tc.addInt(CO_NUMBER);
    tc.addInt(CO_IS_MANUAL_ASSIGNMENT);
    tc.createTableAndResetCreator(TAB_COURT);
    
    // Generate the table holding the teams
    tc.addVarchar(GENERIC_NAME_FIELD_NAME, MAX_NAME_LEN);
    tc.addInt(GENERIC_SEQNUM_FIELD_NAME);
    tc.createTableAndResetCreator(TAB_TEAM);
    
    // Generate the table hosting the players
    tc.addVarchar(PL_FNAME, MAX_NAME_LEN);
    tc.addVarchar(PL_LNAME, MAX_NAME_LEN);
    tc.addInt(GENERIC_STATE_FIELD_NAME);
    tc.addInt(PL_SEX);
    tc.addInt(GENERIC_SEQNUM_FIELD_NAME);
    tc.addForeignKey(PL_TEAM_REF, TAB_TEAM);
    tc.createTableAndResetCreator(TAB_PLAYER);
    
    // Generate the table holding the category data
    tc.addVarchar(GENERIC_NAME_FIELD_NAME, MAX_NAME_LEN);
    tc.addInt(GENERIC_STATE_FIELD_NAME);
    tc.addInt(GENERIC_SEQNUM_FIELD_NAME);
    tc.addInt(CAT_MATCH_TYPE);
    tc.addInt(CAT_SEX);
    tc.addInt(CAT_SYS);
    tc.addInt(CAT_ACCEPT_DRAW);
    tc.addInt(CAT_WIN_SCORE);
    tc.addInt(CAT_DRAW_SCORE);
    tc.addVarchar(CAT_GROUP_CONFIG, 50);
    tc.addVarchar(CAT_BRACKET_VIS_DATA, 50);
    tc.createTableAndResetCreator(TAB_CATEGORY);
    
    // Generate the table holding the player-to-category mapping
    tc.addForeignKey(P2C_PLAYER_REF, TAB_PLAYER);
    tc.addForeignKey(P2C_CAT_REF, TAB_CATEGORY);
    tc.createTableAndResetCreator(TAB_P2C);
    
    // Generate the table holding the player pairs
    tc.addForeignKey(PAIRS_PLAYER1_REF, TAB_PLAYER);
    tc.addForeignKey(PAIRS_PLAYER2_REF, TAB_PLAYER);
    tc.addForeignKey(PAIRS_CAT_REF, TAB_CATEGORY);
    tc.addInt(PAIRS_GRP_NUM);
    tc.addInt(PAIRS_INITIAL_RANK);
    tc.createTableAndResetCreator(TAB_PAIRS);
    
    // Generate a table for the match groups
    tc.addForeignKey(MG_CAT_REF, TAB_CATEGORY);
    tc.addInt(GENERIC_STATE_FIELD_NAME);
    tc.addInt(GENERIC_SEQNUM_FIELD_NAME);
    tc.addInt(MG_ROUND);
    tc.addInt(MG_GRP_NUM);
    tc.addInt(MG_STAGE_SEQ_NUM);
    tc.createTableAndResetCreator(TAB_MATCH_GROUP);

    // Generate a table for matches
    tc.addInt(GENERIC_STATE_FIELD_NAME);
    tc.addInt(GENERIC_SEQNUM_FIELD_NAME);
    tc.addForeignKey(MA_GRP_REF, TAB_MATCH_GROUP);
    tc.addInt(MA_NUM);
    tc.addForeignKey(MA_PAIR1_REF, TAB_PAIRS);
    tc.addForeignKey(MA_PAIR2_REF, TAB_PAIRS);
    tc.addForeignKey(MA_ACTUAL_PLAYER1A_REF, TAB_PLAYER);
    tc.addForeignKey(MA_ACTUAL_PLAYER1B_REF, TAB_PLAYER);
    tc.addForeignKey(MA_ACTUAL_PLAYER2A_REF, TAB_PLAYER);
    tc.addForeignKey(MA_ACTUAL_PLAYER2B_REF, TAB_PLAYER);
    tc.addVarchar(MA_RESULT, 50);
    tc.addForeignKey(MA_COURT_REF, TAB_COURT);
    tc.addVarchar(MA_START_TIME, 50);
    tc.addVarchar(MA_ADDITIONAL_CALL_TIMES, 50);  // up 3 times in seconds since epoch as string
    tc.addVarchar(MA_FINISH_TIME, 50);
    tc.addInt(MA_PAIR1_SYMBOLIC_VAL);
    tc.addInt(MA_PAIR2_SYMBOLIC_VAL);
    tc.addInt(MA_WINNER_RANK);
    tc.addInt(MA_LOSER_RANK);
    tc.createTableAndResetCreator(TAB_MATCH);

    // Generate a table with ranking information
    tc.addInt(RA_ROUND);
    tc.addForeignKey(RA_PAIR_REF, TAB_PAIRS);
    tc.addForeignKey(RA_CAT_REF, TAB_CATEGORY); // this eases searching in the tab. theoretically, the category could be derived from the player pair
    tc.addInt(RA_GRP_NUM); // this eases searching in the tab. theoretically, the group number could be derived from the player pair
    tc.addInt(RA_GAMES_WON);
    tc.addInt(RA_GAMES_LOST);
    tc.addInt(RA_MATCHES_WON);
    tc.addInt(RA_MATCHES_LOST);
    tc.addInt(RA_MATCHES_DRAW);
    tc.addInt(RA_POINTS_WON);
    tc.addInt(RA_POINTS_LOST);
    tc.addInt(RA_RANK);
    tc.createTableAndResetCreator(TAB_RANKING);

    // Generate a table with bracket visualization data
    tc.addForeignKey(BV_MATCH_REF, TAB_MATCH);
    tc.addForeignKey(BV_CAT_REF, TAB_CATEGORY);
    tc.addInt(BV_PAGE);
    tc.addInt(BV_GRID_X0);
    tc.addInt(BV_GRID_Y0);
    tc.addInt(BV_SPAN_Y);
    tc.addInt(BV_ORIENTATION);
    tc.addInt(BV_TERMINATOR);
    tc.addInt(BV_INITIAL_RANK1);
    tc.addInt(BV_INITIAL_RANK2);
    tc.addInt(BV_Y_PAGEBREAK_SPAN);
    tc.addInt(BV_NEXT_PAGE_NUM);
    tc.addInt(BV_TERMINATOR_OFFSET_Y);
    tc.addInt(BV_ELEMENT_ID);
    tc.addInt(BV_NEXT_MATCH_POS_FOR_WINNER);
    tc.addInt(BV_NEXT_MATCH_POS_FOR_LOSER);
    tc.addInt(BV_NEXT_LOSER_MATCH);
    tc.addInt(BV_NEXT_WINNER_MATCH);
    tc.addForeignKey(BV_PAIR1_REF, TAB_PAIRS);
    tc.addForeignKey(BV_PAIR2_REF, TAB_PAIRS);
    tc.createTableAndResetCreator(TAB_BRACKET_VIS);
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

  SqliteOverlay::StringList colList{PL_LNAME, PL_FNAME};
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
  auto cfg = SqliteOverlay::KeyValueTab::getTab(this, TAB_CFG);

  // return an error if no version information is stored in the database
  if (!(cfg->hasKey(CFG_KEY_DB_VERSION)))
  {
    return make_tuple(-1, -1);
  }

  // get the raw version string
  auto _versionString = cfg->getString2(CFG_KEY_DB_VERSION);
  if (_versionString->isNull())
  {
    return make_tuple(-1, -1);
  }
  QString versionString = stdString2QString(_versionString->get());
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
  auto cfg = SqliteOverlay::KeyValueTab::getTab(this, TAB_CFG);
  cfg->set(CFG_KEY_DB_VERSION, QString2StdString(dbVersion));

  return true;
}

}
