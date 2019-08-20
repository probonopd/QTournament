/*
 *    This is QTournament, a badminton tournament management program.
 *    Copyright (C) 2014 - 2017  Volker Knollmann
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

#include <tuple>
#include <regex>

#include <QString>
#include <QStringList>
#include <QFile>

#include <SqliteOverlay/TableCreator.h>
#include <SqliteOverlay/KeyValueTab.h>
#include <SqliteOverlay/DbTab.h>
#include <SqliteOverlay/TabRow.h>

#include <Sloppy/String.h>
#include <Sloppy/CSV.h>

#include "TournamentDB.h"
#include "TournamentDataDefs.h"
#include "HelperFunc.h"
#include "TournamentErrorCodes.h"
#include "OnlineMngr.h"

using namespace std;

namespace QTournament
{

  TournamentDB::TournamentDB()
    :SqliteOverlay::SqliteDatabase()
  {
    TournamentSettings cfg{"-----", "-----", RefereeMode::None, true};
    initBlankDb(cfg);

    // initialize the internal instance of the online manager
    //
    // FIX ME: server name and API url hard coded
    om = make_unique<OnlineMngr>(this);
  }

  //----------------------------------------------------------------------------

  TournamentDB::TournamentDB(const string& fName, const TournamentSettings& cfg)
    : SqliteOverlay::SqliteDatabase(fName, SqliteOverlay::OpenMode::ForceNew)
  {
    initBlankDb(cfg);

    // initialize the internal instance of the online manager
    //
    // FIX ME: server name and API url hard coded
    om = make_unique<OnlineMngr>(this);
  }

  //----------------------------------------------------------------------------

  TournamentDB::TournamentDB(const string& fName)
  {
    if (!isCompatibleDatabaseVersion())
    {
      throw TournamentException{
        "TournamentDB::openExisting()",
        "file " + fName + " is incompatible with this version of QTournament",
        ERR::IncompatibleFileFormat
      };
    }

    // initialize the internal instance of the online manager
    //
    // FIX ME: server name and API url hard coded
    om = make_unique<OnlineMngr>(this);
  }

  //----------------------------------------------------------------------------

  void TournamentDB::populateTables()
  {
    using cdt = SqliteOverlay::ColumnDataType;
    using cc = SqliteOverlay::ConflictClause;
    using ca = SqliteOverlay::ConsistencyAction;

    SqliteOverlay::TableCreator tc;
    
    // Generate the key-value-table with the tournament config
    createNewKeyValueTab(TAB_CFG);
    
    // Generate the table for the courts
    tc.addCol(GENERIC_NAME_FIELD_NAME, cdt::Text, cc::Abort, cc::Abort);
    tc.addCol(GENERIC_STATE_FIELD_NAME, cdt::Integer, cc::NotUsed, cc::Abort);
    tc.addCol(GENERIC_SEQNUM_FIELD_NAME, cdt::Integer, cc::Abort, cc::Abort);
    tc.addCol(CO_NUMBER, cdt::Integer, cc::Abort, cc::Abort);
    tc.addCol(CO_IS_MANUAL_ASSIGNMENT, cdt::Integer, cc::Abort, cc::Abort);
    tc.createTableAndResetCreator(*this, TAB_COURT);
    
    // Generate the table holding the teams
    tc.addCol(GENERIC_NAME_FIELD_NAME, cdt::Text, cc::Abort, cc::Abort);
    tc.addCol(GENERIC_SEQNUM_FIELD_NAME, cdt::Integer, cc::Abort, cc::Abort);
    tc.createTableAndResetCreator(*this, TAB_TEAM);
    
    // Generate the table hosting the players
    tc.addCol(PL_FNAME, cdt::Text, cc::NotUsed, cc::Abort);
    tc.addCol(PL_LNAME, cdt::Text, cc::NotUsed, cc::Abort);
    tc.addCol(GENERIC_STATE_FIELD_NAME, cdt::Integer, cc::NotUsed, cc::Abort);
    tc.addCol(PL_SEX, cdt::Integer, cc::NotUsed, cc::Abort);
    tc.addCol(GENERIC_SEQNUM_FIELD_NAME, cdt::Integer, cc::Abort, cc::Abort);
    tc.addCol(PL_Referee_COUNT, cdt::Integer, cc::NotUsed, cc::Abort, 0);
    tc.addForeignKey(PL_TEAM_REF, TAB_TEAM, ca::Cascade, ca::Cascade, cc::NotUsed, cc::Abort);
    tc.createTableAndResetCreator(*this, TAB_PLAYER);
    
    // Generate the table holding the category data
    tc.addCol(GENERIC_NAME_FIELD_NAME, cdt::Text, cc::Abort, cc::Abort);
    tc.addCol(GENERIC_STATE_FIELD_NAME, cdt::Integer, cc::NotUsed, cc::Abort);
    tc.addCol(GENERIC_SEQNUM_FIELD_NAME, cdt::Integer, cc::Abort, cc::Abort);
    tc.addCol(CAT_MatchType, cdt::Integer, cc::NotUsed, cc::Abort);
    tc.addCol(CAT_Sex, cdt::Integer, cc::NotUsed, cc::Abort);
    tc.addCol(CAT_Sys, cdt::Integer, cc::NotUsed, cc::Abort);
    tc.addCol(CAT_AcceptDraw, cdt::Integer, cc::NotUsed, cc::Abort);
    tc.addCol(CAT_WinScore, cdt::Integer, cc::NotUsed, cc::Abort);
    tc.addCol(CAT_DrawScore, cdt::Integer, cc::NotUsed, cc::Abort);
    tc.addCol(CAT_GroupConfig, cdt::Text, cc::NotUsed, cc::NotUsed);
    tc.addCol(CAT_BracketVisData, cdt::Text, cc::NotUsed, cc::NotUsed);
    tc.addCol(CAT_RoundRobinIterations, cdt::Integer, cc::NotUsed, cc::Abort, 1);
    tc.createTableAndResetCreator(*this, TAB_CATEGORY);
    
    // Generate the table holding the player-to-category mapping
    tc.addForeignKey(P2C_PLAYER_REF, TAB_PLAYER, ca::Cascade, ca::Cascade, cc::NotUsed, cc::Abort);
    tc.addForeignKey(P2C_CAT_REF, TAB_CATEGORY, ca::Cascade, ca::Cascade, cc::NotUsed, cc::Abort);
    tc.createTableAndResetCreator(*this, TAB_P2C);
    
    // Generate the table holding the player pairs
    tc.addForeignKey(PAIRS_PLAYER1_REF, TAB_PLAYER, ca::Cascade, ca::Cascade, cc::NotUsed, cc::Abort);
    tc.addForeignKey(PAIRS_PLAYER2_REF, TAB_PLAYER, ca::Cascade, ca::Cascade, cc::NotUsed, cc::NotUsed);
    tc.addForeignKey(PAIRS_CONFIGREF, TAB_CATEGORY, ca::Cascade, ca::Cascade, cc::NotUsed, cc::Abort);
    tc.addCol(PAIRS_GRP_NUM, cdt::Integer, cc::NotUsed, cc::Abort);
    tc.addCol(PAIRS_INITIAL_RANK, cdt::Integer, cc::NotUsed, cc::Abort);
    tc.createTableAndResetCreator(*this, TAB_PAIRS);
    
    // Generate a table for the match groups
    tc.addForeignKey(MG_CAT_REF, TAB_CATEGORY, ca::Cascade, ca::Cascade, cc::NotUsed, cc::Abort);
    tc.addCol(GENERIC_STATE_FIELD_NAME, cdt::Integer, cc::NotUsed, cc::Abort);
    tc.addCol(GENERIC_SEQNUM_FIELD_NAME, cdt::Integer, cc::Abort, cc::Abort);
    tc.addCol(MG_ROUND, cdt::Integer, cc::NotUsed, cc::Abort);
    tc.addCol(MG_GRP_NUM, cdt::Integer, cc::NotUsed, cc::Abort);
    tc.addCol(MG_STAGE_SEQ_NUM, cdt::Integer, cc::NotUsed, cc::NotUsed);
    tc.createTableAndResetCreator(*this, TAB_MATCH_GROUP);

    // Generate a table for matches
    tc.addCol(GENERIC_STATE_FIELD_NAME, cdt::Integer, cc::NotUsed, cc::Abort);
    tc.addCol(GENERIC_SEQNUM_FIELD_NAME, cdt::Integer, cc::Abort, cc::Abort);
    tc.addForeignKey(MA_GRP_REF, TAB_MATCH_GROUP, ca::Cascade, ca::Cascade, cc::NotUsed, cc::Abort);
    tc.addCol(MA_NUM, cdt::Integer, cc::Abort, cc::Abort);
    tc.addForeignKey(MA_PAIR1_REF, TAB_PAIRS, ca::Cascade, ca::Cascade, cc::NotUsed, cc::Abort);
    tc.addForeignKey(MA_PAIR2_REF, TAB_PAIRS, ca::Cascade, ca::Cascade, cc::NotUsed, cc::Abort);
    tc.addForeignKey(MA_ACTUAL_PLAYER1A_REF, TAB_PLAYER, ca::Cascade, ca::Cascade, cc::NotUsed, cc::NotUsed);
    tc.addForeignKey(MA_ACTUAL_PLAYER1B_REF, TAB_PLAYER, ca::Cascade, ca::Cascade, cc::NotUsed, cc::NotUsed);
    tc.addForeignKey(MA_ACTUAL_PLAYER2A_REF, TAB_PLAYER, ca::Cascade, ca::Cascade, cc::NotUsed, cc::NotUsed);
    tc.addForeignKey(MA_ACTUAL_PLAYER2B_REF, TAB_PLAYER, ca::Cascade, ca::Cascade, cc::NotUsed, cc::NotUsed);
    tc.addCol(MA_RESULT, cdt::Text, cc::NotUsed, cc::NotUsed);
    tc.addForeignKey(MA_COURT_REF, TAB_COURT, ca::Cascade, ca::Cascade, cc::NotUsed, cc::Abort);
    tc.addCol(MA_START_TIME, cdt::Text, cc::NotUsed, cc::NotUsed);
    tc.addCol(MA_ADDITIONAL_CALL_TIMES, cdt::Text, cc::NotUsed, cc::NotUsed);  // up 3 times in seconds since epoch as string
    tc.addCol(MA_FINISH_TIME, cdt::Text, cc::NotUsed, cc::NotUsed);
    tc.addCol(MA_PAIR1_SYMBOLIC_VAL, cdt::Integer, cc::NotUsed, cc::NotUsed);
    tc.addCol(MA_PAIR2_SYMBOLIC_VAL, cdt::Integer, cc::NotUsed, cc::NotUsed);
    tc.addCol(MA_WINNER_RANK, cdt::Integer, cc::NotUsed, cc::NotUsed);
    tc.addCol(MA_LOSER_RANK, cdt::Integer, cc::NotUsed, cc::NotUsed);
    tc.addCol(MA_RefereeMode, cdt::Integer, cc::NotUsed, cc::Abort, -1);
    tc.addForeignKey(MA_REFEREE_REF, TAB_PLAYER, ca::Cascade, ca::Cascade, cc::NotUsed, cc::NotUsed);
    tc.createTableAndResetCreator(*this, TAB_MATCH);

    // Generate a table with ranking information
    tc.addCol(RA_ROUND, cdt::Integer, cc::NotUsed, cc::Abort);
    tc.addForeignKey(RA_PAIR_REF, TAB_PAIRS, ca::Cascade, ca::Cascade, cc::NotUsed, cc::Abort);
    tc.addForeignKey(RA_CAT_REF, TAB_CATEGORY, ca::Cascade, ca::Cascade, cc::NotUsed, cc::Abort); // this eases searching in the tab. theoretically, the category could be derived from the player pair
    tc.addCol(RA_GRP_NUM, cdt::Integer, cc::NotUsed, cc::Abort); // this eases searching in the tab. theoretically, the group number could be derived from the player pair
    tc.addCol(RA_GAMES_WON, cdt::Integer, cc::NotUsed, cc::Abort);
    tc.addCol(RA_GAMES_LOST, cdt::Integer, cc::NotUsed, cc::Abort);
    tc.addCol(RA_MATCHES_WON, cdt::Integer, cc::NotUsed, cc::Abort);
    tc.addCol(RA_MATCHES_LOST, cdt::Integer, cc::NotUsed, cc::Abort);
    tc.addCol(RA_MATCHES_DRAW, cdt::Integer, cc::NotUsed, cc::Abort);
    tc.addCol(RA_POINTS_WON, cdt::Integer, cc::NotUsed, cc::Abort);
    tc.addCol(RA_POINTS_LOST, cdt::Integer, cc::NotUsed, cc::Abort);
    tc.addCol(RA_RANK, cdt::Integer, cc::NotUsed, cc::Abort);
    tc.createTableAndResetCreator(*this, TAB_MatchSystem);

    // Generate a table with bracket visualization data
    tc.addForeignKey(BV_MATCH_REF, TAB_MATCH, ca::Cascade, ca::Cascade, cc::Abort, cc::Abort);
    tc.addForeignKey(BV_CONFIGREF, TAB_CATEGORY, ca::Cascade, ca::Cascade, cc::NotUsed, cc::Abort);
    tc.addCol(BV_PAGE, cdt::Integer, cc::NotUsed, cc::Abort);
    tc.addCol(BV_GRID_X0, cdt::Integer, cc::NotUsed, cc::Abort);
    tc.addCol(BV_GRID_Y0, cdt::Integer, cc::NotUsed, cc::Abort);
    tc.addCol(BV_SPAN_Y, cdt::Integer, cc::NotUsed, cc::Abort);
    tc.addCol(BV_ORIENTATION, cdt::Integer, cc::NotUsed, cc::Abort);
    tc.addCol(BV_TERMINATOR, cdt::Integer, cc::NotUsed, cc::Abort);
    tc.addCol(BV_INITIAL_RANK1, cdt::Integer, cc::NotUsed, cc::Abort);
    tc.addCol(BV_INITIAL_RANK2, cdt::Integer, cc::NotUsed, cc::Abort);
    tc.addCol(BV_Y_PAGEBREAK_SPAN, cdt::Integer, cc::NotUsed, cc::Abort);
    tc.addCol(BV_NEXT_PAGE_NUM, cdt::Integer, cc::NotUsed, cc::Abort);
    tc.addCol(BV_TERMINATOR_OFFSET_Y, cdt::Integer, cc::NotUsed, cc::Abort);
    tc.addCol(BV_ELEMENT_ID, cdt::Integer, cc::NotUsed, cc::Abort);
    tc.addCol(BV_NEXT_MATCH_POS_FOR_WINNER, cdt::Integer, cc::NotUsed, cc::Abort);
    tc.addCol(BV_NEXT_MATCH_POS_FOR_LOSER, cdt::Integer, cc::NotUsed, cc::Abort);
    tc.addCol(BV_NEXT_LOSER_MATCH, cdt::Integer, cc::NotUsed, cc::Abort);
    tc.addCol(BV_NEXT_WINNER_MATCH, cdt::Integer, cc::NotUsed, cc::Abort);
    tc.addForeignKey(BV_PAIR1_REF, TAB_PAIRS, ca::Cascade, ca::Cascade, cc::NotUsed, cc::Abort);
    tc.addForeignKey(BV_PAIR2_REF, TAB_PAIRS, ca::Cascade, ca::Cascade, cc::NotUsed, cc::Abort);
    tc.createTableAndResetCreator(*this, TAB_BRACKET_VIS);
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

    Sloppy::StringList colList{PL_LNAME, PL_FNAME};
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
    indexCreationHelper(TAB_PAIRS, PAIRS_CONFIGREF);

    indexCreationHelper(TAB_MATCH_GROUP, MG_CAT_REF);
    indexCreationHelper(TAB_MATCH_GROUP, MG_GRP_NUM);
    indexCreationHelper(TAB_MATCH_GROUP, MG_ROUND);
    indexCreationHelper(TAB_MATCH_GROUP, MG_STAGE_SEQ_NUM, true);

    indexCreationHelper(TAB_MATCH, MA_GRP_REF);
    indexCreationHelper(TAB_MATCH, GENERIC_SEQNUM_FIELD_NAME, true);
    indexCreationHelper(TAB_MATCH, GENERIC_STATE_FIELD_NAME);
    indexCreationHelper(TAB_MATCH, MA_PAIR1_REF);
    indexCreationHelper(TAB_MATCH, MA_PAIR2_REF);

    indexCreationHelper(TAB_MatchSystem, RA_PAIR_REF);
    indexCreationHelper(TAB_MatchSystem, RA_CAT_REF);
    indexCreationHelper(TAB_MatchSystem, RA_ROUND);

    indexCreationHelper(TAB_BRACKET_VIS, BV_CONFIGREF);
    indexCreationHelper(TAB_BRACKET_VIS, BV_MATCH_REF);
    indexCreationHelper(TAB_BRACKET_VIS, BV_PAIR1_REF);
    indexCreationHelper(TAB_BRACKET_VIS, BV_PAIR2_REF);

    //indexCreationHelper(TAB_, );
  }

  //----------------------------------------------------------------------------

  std::tuple<int, int> TournamentDB::getVersion()
  {
    SqliteOverlay::KeyValueTab cfg{*this, TAB_CFG};

    // return an error if no version information is stored in the database
    if (!cfg.hasKey(CFG_KEY_DB_VERSION))
    {
      return make_tuple(-1, -1);
    }

    // get the raw version string
    auto _versionString = cfg.getString2(CFG_KEY_DB_VERSION);
    if (!_versionString.has_value())
    {
      return make_tuple(-1, -1);
    }
    if (_versionString->empty())
    {
      return make_tuple(-1, -1);
    }

    // try to split it into major / minor
    static const std::regex reMajorMinor{"(\\d+)\\.(\\d+)"};
    std::smatch sm;
    if (!regex_match(*_versionString, sm, reMajorMinor))
    {
      return make_tuple(-1, -1);
    }
    if (sm.size() != 3)  // index 0 = whole string, 1 = major, 2 = minor
    {
      return make_tuple(-1, -1);
    }
    int major = stoi(sm[1]);
    int minor = stoi(sm[2]);

    return make_tuple(major, minor);
  }

  //----------------------------------------------------------------------------

  bool TournamentDB::isCompatibleDatabaseVersion()
  {
    // get the current file format version
    auto[major, minor] = getVersion();

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
    auto[major, minor] = getVersion();

    // condition: major version number must
    // match the compiled in version and the minor
    // version number must be less then
    // the version at compile time
    return ((major == DB_VERSION_MAJOR) && (minor >= 0) && (minor < DB_VERSION_MINOR));
  }

  //----------------------------------------------------------------------------

  bool TournamentDB::convertToLatestDatabaseVersion()
  {
    // FIX: adapt to new database schema; drop support for old database versions
    return false;
  }

  //----------------------------------------------------------------------------

  OnlineMngr* TournamentDB::getOnlineManager()
  {
    return om.get();
  }

  //----------------------------------------------------------------------------

  std::tuple<string, int> TournamentDB::tableDataToCSV(const string& tabName, const std::vector<Sloppy::estring>& colNames, int rowId) const
  {
    std::vector<int> v = (rowId < 0) ? std::vector<int>{} : std::vector<int>{rowId,};
    return tableDataToCSV(tabName, colNames, v);
  }

  //----------------------------------------------------------------------------

  std::tuple<string, int> TournamentDB::tableDataToCSV(const string& tabName, const std::vector<Sloppy::estring>& colNames, const std::vector<int>& rowList) const
  {
    SqliteOverlay::DbTab tab{*this, tabName, false};

    // build a specific query that fetches all columns at
    // once which has a higher performance than issuing
    // one dedicated SELECT for each column
    Sloppy::estring baseSql = "SELECT %1 FROM %2";
    Sloppy::estring comSepColNames{colNames, ","};
    baseSql.arg(comSepColNames);
    baseSql.arg(tabName);

    try
    {
      string result;
      int totalCount = 0;
      if (rowList.empty())  // get all rows
      {
        auto stmt = prepStatement(baseSql);
        auto csvTab = stmt.toCSV(false);
        result = csvTab.asString(false, Sloppy::CSV_StringRepresentation::QuotedAndEscaped);
        totalCount = csvTab.size();
      } else {
        for (int rowId : rowList)
        {
          // if the rowId is > 0, it indicates an insert
          // or update and thus we have to fetch the data
          if (rowId > 0)
          {
            string sql = baseSql + " WHERE id=" + to_string(rowId);
            auto stmt = prepStatement(sql);
            stmt.step();
            auto csvRow = stmt.toCSV_currentRowOnly();
            result += csvRow.asString(Sloppy::CSV_StringRepresentation::QuotedAndEscaped) + "\n";
          } else {
            // negative rowIDs indicate "deletion" and
            // we simple put them on an otherwise empty line
            result += to_string(rowId) + "\n";
          }
          ++totalCount;
        }
      }

      return make_tuple(result, totalCount);
    }
    catch (...)
    {
      return make_tuple("", -1);
    }
  }

  //----------------------------------------------------------------------------

  string TournamentDB::getSyncStringForTable(const string& tabName, const std::vector<Sloppy::estring>& colNames, int rowId) const
  {
    std::vector<int> v = (rowId < 0) ? std::vector<int>{} : std::vector<int>{rowId,};
    return getSyncStringForTable(tabName, colNames, v);
  }

  //----------------------------------------------------------------------------

  string TournamentDB::getSyncStringForTable(const string& tabName, const std::vector<Sloppy::estring>& colNames, std::vector<int> rowList) const
  {
    int cnt;
    string data;
    tie(data, cnt) = tableDataToCSV(tabName, colNames, rowList);
    if (cnt < 0) return "";  // error

    Sloppy::estring header = "%1:%2\n%3\n";
    header.arg(tabName);
    header.arg(cnt);
    header.arg(Sloppy::estring{colNames, ","});

    return header + data;
  }

  //----------------------------------------------------------------------------

  void TournamentDB::initBlankDb(const TournamentSettings& cfg)
  {
    populateTables();
    populateTables();
    createIndices();

    // initialize the database
    auto cfgTab = SqliteOverlay::KeyValueTab(*this, TAB_CFG);
    Sloppy::estring dbVersion = "%1.%2";
    dbVersion.arg(DB_VERSION_MAJOR);
    dbVersion.arg(DB_VERSION_MINOR);
    cfgTab.set(CFG_KEY_DB_VERSION, dbVersion);
    cfgTab.set(CFG_KEY_TNMT_NAME, QString2StdString(cfg.tournamentName));
    cfgTab.set(CFG_KEY_TNMT_ORGA, QString2StdString(cfg.organizingClub));
    cfgTab.set(CFG_KEY_USE_TEAMS, cfg.useTeams);
    cfgTab.set(CFG_KEY_REFEREE_TEAM_ID, -1);
    cfgTab.set(CFG_KEY_DEFAULT_RefereeMode, static_cast<int>(cfg.refereeMode));
  }

  //----------------------------------------------------------------------------

  TournamentDB createNew(const QString& fName, const TournamentSettings& cfg)
  {
    const std::string stdName = QString2StdString(fName);

    try
    {
      return TournamentDB{stdName, cfg};
    } catch (std::invalid_argument) {
      throw TournamentException{"TournamentDB::createNew()", "file " + stdName + " already exists", ERR::FileAlreadyExists};
    } catch (...) {
      throw TournamentException{"TournamentDB::createNew()", "Error when instantiating the new database object", ERR::DatabaseError};
    }
  }

  //----------------------------------------------------------------------------

  TournamentDB openExisting(const QString& fName)
  {
    const std::string stdName = QString2StdString(fName);

    try
    {
      return TournamentDB{stdName};
    } catch (std::invalid_argument) {
      throw TournamentException{"TournamentDB::openExisting()", "file " + stdName + " does not exist", ERR::FileNotExisting};
    } catch (TournamentException) {
      throw;
    } catch (...) {
      throw TournamentException{"TournamentDB::openExisting()", "Error when instantiating the new database object", ERR::DatabaseError};
    }
  }

}
