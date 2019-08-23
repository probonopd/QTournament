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
    om = make_unique<OnlineMngr>(*this);
  }

  //----------------------------------------------------------------------------

  TournamentDB::TournamentDB(const string& fName, const TournamentSettings& cfg)
    : SqliteOverlay::SqliteDatabase(fName, SqliteOverlay::OpenMode::ForceNew)
  {
    initBlankDb(cfg);

    // initialize the internal instance of the online manager
    //
    // FIX ME: server name and API url hard coded
    om = make_unique<OnlineMngr>(*this);
  }

  //----------------------------------------------------------------------------

  TournamentDB::TournamentDB(const string& fName)
    :SqliteOverlay::SqliteDatabase(fName, SqliteOverlay::OpenMode::OpenExisting_RW)
  {
    if (!isCompatibleDatabaseVersion())
    {
      throw TournamentException{
        "TournamentDB::openExisting()",
        "file " + fName + " is incompatible with this version of QTournament",
        Error::IncompatibleFileFormat
      };
    }

    // initialize the internal instance of the online manager
    //
    // FIX ME: server name and API url hard coded
    om = make_unique<OnlineMngr>(*this);
  }

  //----------------------------------------------------------------------------

  void TournamentDB::populateTables()
  {
    using cdt = SqliteOverlay::ColumnDataType;
    using cc = SqliteOverlay::ConflictClause;
    using ca = SqliteOverlay::ConsistencyAction;

    SqliteOverlay::TableCreator tc;
    
    // Generate the key-value-table with the tournament config
    createNewKeyValueTab(TabCfg);
    
    // Generate the table for the courts
    tc.addCol(GenericNameFieldName, cdt::Text, cc::NotUsed, cc::Abort);
    tc.addCol(GenericStateFieldName, cdt::Integer, cc::NotUsed, cc::Abort);
    tc.addCol(GenericSeqnumFieldName, cdt::Integer, cc::Abort, cc::Abort);
    tc.addCol(CO_Number, cdt::Integer, cc::Abort, cc::Abort);
    tc.addCol(CO_IsManualAssignment, cdt::Integer, cc::NotUsed, cc::Abort);
    tc.createTableAndResetCreator(*this, TabCourt);
    
    // Generate the table holding the teams
    tc.addCol(GenericNameFieldName, cdt::Text, cc::Abort, cc::Abort);
    tc.addCol(GenericSeqnumFieldName, cdt::Integer, cc::Abort, cc::Abort);
    tc.createTableAndResetCreator(*this, TabTeam);
    
    // Generate the table hosting the players
    tc.addCol(PL_Fname, cdt::Text, cc::NotUsed, cc::Abort);
    tc.addCol(PL_Lname, cdt::Text, cc::NotUsed, cc::Abort);
    tc.addCol(GenericStateFieldName, cdt::Integer, cc::NotUsed, cc::Abort);
    tc.addCol(PL_Sex, cdt::Integer, cc::NotUsed, cc::Abort);
    tc.addCol(GenericSeqnumFieldName, cdt::Integer, cc::Abort, cc::Abort);
    tc.addCol(PL_RefereeCount, cdt::Integer, cc::NotUsed, cc::Abort, 0);
    tc.addForeignKey(PL_TeamRef, TabTeam, ca::Cascade, ca::Cascade, cc::NotUsed, cc::Abort);
    tc.createTableAndResetCreator(*this, TabPlayer);
    
    // Generate the table holding the category data
    tc.addCol(GenericNameFieldName, cdt::Text, cc::Abort, cc::Abort);
    tc.addCol(GenericStateFieldName, cdt::Integer, cc::NotUsed, cc::Abort);
    tc.addCol(GenericSeqnumFieldName, cdt::Integer, cc::Abort, cc::Abort);
    tc.addCol(CAT_MatchType, cdt::Integer, cc::NotUsed, cc::Abort);
    tc.addCol(CAT_Sex, cdt::Integer, cc::NotUsed, cc::Abort);
    tc.addCol(CAT_Sys, cdt::Integer, cc::NotUsed, cc::Abort);
    tc.addCol(CAT_AcceptDraw, cdt::Integer, cc::NotUsed, cc::Abort);
    tc.addCol(CAT_WinScore, cdt::Integer, cc::NotUsed, cc::Abort);
    tc.addCol(CAT_DrawScore, cdt::Integer, cc::NotUsed, cc::Abort);
    tc.addCol(CAT_GroupConfig, cdt::Text, cc::NotUsed, cc::NotUsed);
    tc.addCol(CAT_BracketVisData, cdt::Text, cc::NotUsed, cc::NotUsed);
    tc.addCol(CAT_RoundRobinIterations, cdt::Integer, cc::NotUsed, cc::Abort, 1);
    tc.createTableAndResetCreator(*this, TabCategory);
    
    // Generate the table holding the player-to-category mapping
    tc.addForeignKey(P2C_PlayerRef, TabPlayer, ca::Cascade, ca::Cascade, cc::NotUsed, cc::Abort);
    tc.addForeignKey(P2C_CatRef, TabCategory, ca::Cascade, ca::Cascade, cc::NotUsed, cc::Abort);
    tc.createTableAndResetCreator(*this, TabP2C);
    
    // Generate the table holding the player pairs
    tc.addForeignKey(Pairs_Player1Ref, TabPlayer, ca::Cascade, ca::Cascade, cc::NotUsed, cc::Abort);
    tc.addForeignKey(Pairs_Player2Ref, TabPlayer, ca::Cascade, ca::Cascade, cc::NotUsed, cc::NotUsed);
    tc.addForeignKey(Pairs_CatRef, TabCategory, ca::Cascade, ca::Cascade, cc::NotUsed, cc::Abort);
    tc.addCol(Pairs_GrpNum, cdt::Integer, cc::NotUsed, cc::Abort);
    tc.addCol(Pairs_InitialRank, cdt::Integer, cc::NotUsed, cc::Abort);
    tc.createTableAndResetCreator(*this, TabPairs);
    
    // Generate a table for the match groups
    tc.addForeignKey(MG_CatRef, TabCategory, ca::Cascade, ca::Cascade, cc::NotUsed, cc::Abort);
    tc.addCol(GenericStateFieldName, cdt::Integer, cc::NotUsed, cc::Abort);
    tc.addCol(GenericSeqnumFieldName, cdt::Integer, cc::Abort, cc::Abort);
    tc.addCol(MG_Round, cdt::Integer, cc::NotUsed, cc::Abort);
    tc.addCol(MG_GrpNum, cdt::Integer, cc::NotUsed, cc::Abort);
    tc.addCol(MG_StageSeqNum, cdt::Integer, cc::NotUsed, cc::NotUsed);
    tc.createTableAndResetCreator(*this, TabMatchGroup);

    // Generate a table for matches
    tc.addCol(GenericStateFieldName, cdt::Integer, cc::NotUsed, cc::Abort);
    tc.addCol(GenericSeqnumFieldName, cdt::Integer, cc::Abort, cc::Abort);
    tc.addForeignKey(MA_GrpRef, TabMatchGroup, ca::Cascade, ca::Cascade, cc::NotUsed, cc::Abort);
    tc.addCol(MA_Num, cdt::Integer, cc::Abort, cc::NotUsed);
    tc.addForeignKey(MA_Pair1Ref, TabPairs, ca::Cascade, ca::Cascade, cc::NotUsed, cc::NotUsed);
    tc.addForeignKey(MA_Pair2Ref, TabPairs, ca::Cascade, ca::Cascade, cc::NotUsed, cc::NotUsed);
    tc.addForeignKey(MA_ActualPlayer1aRef, TabPlayer, ca::Cascade, ca::Cascade, cc::NotUsed, cc::NotUsed);
    tc.addForeignKey(MA_ActualPlayer1bRef, TabPlayer, ca::Cascade, ca::Cascade, cc::NotUsed, cc::NotUsed);
    tc.addForeignKey(MA_ActualPlayer2aRef, TabPlayer, ca::Cascade, ca::Cascade, cc::NotUsed, cc::NotUsed);
    tc.addForeignKey(MA_ActualPlayer2bRef, TabPlayer, ca::Cascade, ca::Cascade, cc::NotUsed, cc::NotUsed);
    tc.addCol(MA_Result, cdt::Text, cc::NotUsed, cc::NotUsed);
    tc.addForeignKey(MA_CourtRef, TabCourt, ca::Cascade, ca::Cascade, cc::NotUsed, cc::NotUsed);
    tc.addCol(MA_StartTime, cdt::Text, cc::NotUsed, cc::NotUsed);
    tc.addCol(MA_AdditionalCallTimes, cdt::Text, cc::NotUsed, cc::NotUsed);  // up 3 times in seconds since epoch as string
    tc.addCol(MA_FinishTime, cdt::Text, cc::NotUsed, cc::NotUsed);
    tc.addCol(MA_Pair1SymbolicVal, cdt::Integer, cc::NotUsed, cc::NotUsed);
    tc.addCol(MA_Pair2SymbolicVal, cdt::Integer, cc::NotUsed, cc::NotUsed);
    tc.addCol(MA_WinnerRank, cdt::Integer, cc::NotUsed, cc::NotUsed);
    tc.addCol(MA_LoserRank, cdt::Integer, cc::NotUsed, cc::NotUsed);
    tc.addCol(MA_RefereeMode, cdt::Integer, cc::NotUsed, cc::Abort, -1);
    tc.addForeignKey(MA_RefereeRef, TabPlayer, ca::Cascade, ca::Cascade, cc::NotUsed, cc::NotUsed);
    tc.createTableAndResetCreator(*this, TabMatch);

    // Generate a table with ranking information
    tc.addCol(RA_Round, cdt::Integer, cc::NotUsed, cc::Abort);
    tc.addForeignKey(RA_PairRef, TabPairs, ca::Cascade, ca::Cascade, cc::NotUsed, cc::Abort);
    tc.addForeignKey(RA_CatRef, TabCategory, ca::Cascade, ca::Cascade, cc::NotUsed, cc::Abort); // this eases searching in the tab. theoretically, the category could be derived from the player pair
    tc.addCol(RA_GrpNum, cdt::Integer, cc::NotUsed, cc::Abort); // this eases searching in the tab. theoretically, the group number could be derived from the player pair
    tc.addCol(RA_GamesWon, cdt::Integer, cc::NotUsed, cc::Abort);
    tc.addCol(RA_GamesLost, cdt::Integer, cc::NotUsed, cc::Abort);
    tc.addCol(RA_MatchesWon, cdt::Integer, cc::NotUsed, cc::Abort);
    tc.addCol(RA_MatchesLost, cdt::Integer, cc::NotUsed, cc::Abort);
    tc.addCol(RA_MatchesDraw, cdt::Integer, cc::NotUsed, cc::Abort);
    tc.addCol(RA_PointsWon, cdt::Integer, cc::NotUsed, cc::Abort);
    tc.addCol(RA_PointsLost, cdt::Integer, cc::NotUsed, cc::Abort);
    tc.addCol(RA_Rank, cdt::Integer, cc::NotUsed, cc::Abort);
    tc.createTableAndResetCreator(*this, TabMatchSystem);

    // Generate a table with bracket visualization data
    tc.addForeignKey(BV_MatchRef, TabMatch, ca::Cascade, ca::Cascade, cc::Abort, cc::Abort);
    tc.addForeignKey(BV_CatRef, TabCategory, ca::Cascade, ca::Cascade, cc::NotUsed, cc::Abort);
    tc.addCol(BV_Page, cdt::Integer, cc::NotUsed, cc::Abort);
    tc.addCol(BV_GridX0, cdt::Integer, cc::NotUsed, cc::Abort);
    tc.addCol(BV_GridY0, cdt::Integer, cc::NotUsed, cc::Abort);
    tc.addCol(BV_SpanY, cdt::Integer, cc::NotUsed, cc::Abort);
    tc.addCol(BV_Orientation, cdt::Integer, cc::NotUsed, cc::Abort);
    tc.addCol(BV_Terminator, cdt::Integer, cc::NotUsed, cc::Abort);
    tc.addCol(BV_InitialRank1, cdt::Integer, cc::NotUsed, cc::Abort);
    tc.addCol(BV_InitialRank2, cdt::Integer, cc::NotUsed, cc::Abort);
    tc.addCol(BV_YPagebreakSpan, cdt::Integer, cc::NotUsed, cc::Abort);
    tc.addCol(BV_NextPageNum, cdt::Integer, cc::NotUsed, cc::Abort);
    tc.addCol(BV_Terminator_OFFSET_Y, cdt::Integer, cc::NotUsed, cc::Abort);
    tc.addCol(BV_ElementId, cdt::Integer, cc::NotUsed, cc::Abort);
    tc.addCol(BV_NextMatchPosForWinner, cdt::Integer, cc::NotUsed, cc::Abort);
    tc.addCol(BV_NextMatchPosForLoser, cdt::Integer, cc::NotUsed, cc::Abort);
    tc.addCol(BV_NextLoserMatch, cdt::Integer, cc::NotUsed, cc::Abort);
    tc.addCol(BV_NextWinnerMatch, cdt::Integer, cc::NotUsed, cc::Abort);
    tc.addForeignKey(BV_Pair1Ref, TabPairs, ca::Cascade, ca::Cascade, cc::NotUsed, cc::Abort);
    tc.addForeignKey(BV_Pair2Ref, TabPairs, ca::Cascade, ca::Cascade, cc::NotUsed, cc::Abort);
    tc.createTableAndResetCreator(*this, TabBracketVis);
  }

  //----------------------------------------------------------------------------

  void TournamentDB::populateViews()
  {

  }

  //----------------------------------------------------------------------------

  void TournamentDB::createIndices()
  {
    indexCreationHelper(TabCourt, GenericNameFieldName);
    indexCreationHelper(TabCourt, GenericSeqnumFieldName, true);

    indexCreationHelper(TabTeam, GenericNameFieldName, true);
    indexCreationHelper(TabTeam, GenericSeqnumFieldName, true);

    Sloppy::StringList colList{PL_Lname, PL_Fname};
    indexCreationHelper(TabPlayer, GenericSeqnumFieldName, true);
    indexCreationHelper(TabPlayer, GenericStateFieldName);
    indexCreationHelper(TabPlayer, PL_Fname);
    indexCreationHelper(TabPlayer, PL_Lname);
    indexCreationHelper(TabPlayer, "Player_CombinedNames", colList, true);

    indexCreationHelper(TabCategory, GenericNameFieldName, true);
    indexCreationHelper(TabCategory, GenericSeqnumFieldName, true);
    indexCreationHelper(TabCategory, GenericStateFieldName);

    indexCreationHelper(TabP2C, P2C_CatRef);
    indexCreationHelper(TabP2C, P2C_PlayerRef);

    indexCreationHelper(TabPairs, Pairs_Player1Ref);
    indexCreationHelper(TabPairs, Pairs_Player1Ref);
    indexCreationHelper(TabPairs, Pairs_CatRef);

    indexCreationHelper(TabMatchGroup, MG_CatRef);
    indexCreationHelper(TabMatchGroup, MG_GrpNum);
    indexCreationHelper(TabMatchGroup, MG_Round);
    indexCreationHelper(TabMatchGroup, MG_StageSeqNum, true);

    indexCreationHelper(TabMatch, MA_GrpRef);
    indexCreationHelper(TabMatch, GenericSeqnumFieldName, true);
    indexCreationHelper(TabMatch, GenericStateFieldName);
    indexCreationHelper(TabMatch, MA_Pair1Ref);
    indexCreationHelper(TabMatch, MA_Pair2Ref);

    indexCreationHelper(TabMatchSystem, RA_PairRef);
    indexCreationHelper(TabMatchSystem, RA_CatRef);
    indexCreationHelper(TabMatchSystem, RA_Round);

    indexCreationHelper(TabBracketVis, BV_CatRef);
    indexCreationHelper(TabBracketVis, BV_MatchRef);
    indexCreationHelper(TabBracketVis, BV_Pair1Ref);
    indexCreationHelper(TabBracketVis, BV_Pair2Ref);

    //indexCreationHelper(TAB_, );
  }

  //----------------------------------------------------------------------------

  std::tuple<int, int> TournamentDB::getVersion()
  {
    SqliteOverlay::KeyValueTab cfg{*this, TabCfg};

    // return an error if no version information is stored in the database
    if (!cfg.hasKey(CfgKey_DbVersion))
    {
      return make_tuple(-1, -1);
    }

    // get the raw version string
    auto _versionString = cfg.getString2(CfgKey_DbVersion);
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
    return ((major == DbVersionMajor) && (minor >= 0) && (minor <= DbVersionMinor));
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
    return ((major == DbVersionMajor) && (minor >= 0) && (minor < DbVersionMinor));
  }

  //----------------------------------------------------------------------------

  bool TournamentDB::convertToLatestDatabaseVersion()
  {
    // FIX: adapt to new database schema; drop support for old database versions
    return false;
  }

  //----------------------------------------------------------------------------

  OnlineMngr* TournamentDB::getOnlineManager() const
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
    populateViews();
    createIndices();

    // initialize the database
    auto cfgTab = SqliteOverlay::KeyValueTab(*this, TabCfg);
    Sloppy::estring dbVersion = "%1.%2";
    dbVersion.arg(DbVersionMajor);
    dbVersion.arg(DbVersionMinor);
    cfgTab.set(CfgKey_DbVersion, dbVersion.toStdString());
    cfgTab.set(CfgKey_TnmtName, QString2StdString(cfg.tournamentName));
    cfgTab.set(CfgKey_TnmtOrga, QString2StdString(cfg.organizingClub));
    cfgTab.set(CfgKey_UseTeams, cfg.useTeams);
    cfgTab.set(CfgKey_RefereeTeamId, -1);
    cfgTab.set(CfgKey_DefaultRefereemode, static_cast<int>(cfg.refereeMode));
  }

  //----------------------------------------------------------------------------

  TournamentDB createNew(const QString& fName, const TournamentSettings& cfg)
  {
    const std::string stdName = QString2StdString(fName);

    try
    {
      return TournamentDB{stdName, cfg};
    } catch (std::invalid_argument) {
      throw TournamentException{"TournamentDB::createNew()", "file " + stdName + " already exists", Error::FileAlreadyExists};
    } catch (...) {
      throw TournamentException{"TournamentDB::createNew()", "Error when instantiating the new database object", Error::DatabaseError};
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
      throw TournamentException{"TournamentDB::openExisting()", "file " + stdName + " does not exist", Error::FileNotExisting};
    } catch (TournamentException) {
      throw;
    } catch (...) {
      throw TournamentException{"TournamentDB::openExisting()", "Error when instantiating the new database object", Error::DatabaseError};
    }
  }

}
