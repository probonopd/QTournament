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

#include <exception>

#include <assert.h>

#include <QStringList>
#include <QHash>

#include "ExternalPlayerDB.h"
#include <SqliteOverlay/KeyValueTab.h>
#include <SqliteOverlay/TableCreator.h>
#include <SqliteOverlay/ClausesAndQueries.h>

#include "HelperFunc.h"
#include "TournamentDataDefs.h"

namespace QTournament
{

  opExternalPlayerDatabaseEntry ExternalPlayerDB::row2upEntry(const SqliteOverlay::TabRow& r) const
  {
    auto sexValue = r.getInt2(EPD_PL_SEX);
    SEX sex = sexValue.has_value() ? static_cast<SEX>(*sexValue) : DONT_CARE;
    return ExternalPlayerDatabaseEntry{r.id(), stdString2QString(r[EPD_PL_FNAME]), stdString2QString(r[EPD_PL_LNAME]),
          sex};

  }

  //----------------------------------------------------------------------------

  ExternalPlayerDB::ExternalPlayerDB(const std::string& fname, SqliteOverlay::OpenMode om)
    :SqliteOverlay::SqliteDatabase(fname, om)
  {

  }

  //----------------------------------------------------------------------------

  std::optional<ExternalPlayerDB> ExternalPlayerDB::createNew(const QString& fname)
  {
    // try to create the new database
    try
    {
      return ExternalPlayerDB{fname.toUtf8().constData(), SqliteOverlay::OpenMode::ForceNew};
    }
    catch (std::invalid_argument&)
    {
      return {};  // file already exists or could not be created
    }
    catch (SqliteOverlay::GenericSqliteException&)
    {
      return {};  // some SQLite error
    }
  }

  //----------------------------------------------------------------------------

  std::optional<ExternalPlayerDB> ExternalPlayerDB::openExisting(const QString& fname)
  {
    // try to open the existing database
    try
    {
      auto extDb = ExternalPlayerDB{fname.toUtf8().constData(), SqliteOverlay::OpenMode::OpenExisting_RW};

      // check the database version
      SqliteOverlay::KeyValueTab cfg{extDb, TAB_EPD_CFG};
      int actualDbVersion = cfg.getInt(CFG_KEY_EPD_DB_VERSION);
      if (actualDbVersion != EXT_PLAYER_DB_VERSION)
      {
        return {};
      }

      return extDb;
    }
    catch (std::invalid_argument&)
    {
      return {};  // file does not exist or could not be accessed
    }
    catch (SqliteOverlay::GenericSqliteException&)
    {
      return {};  // some SQLite error
    }

  }

  //----------------------------------------------------------------------------

  void ExternalPlayerDB::populateTables()
  {
    // Generate the key-value-table with the database version
    createNewKeyValueTab(TAB_EPD_CFG);

    // Generate the table for the players
    SqliteOverlay::TableCreator tc;
    tc.addCol(EPD_PL_FNAME, SqliteOverlay::ColumnDataType::Text, SqliteOverlay::ConflictClause::NotUsed, SqliteOverlay::ConflictClause::Abort);
    tc.addCol(EPD_PL_LNAME, SqliteOverlay::ColumnDataType::Text, SqliteOverlay::ConflictClause::NotUsed, SqliteOverlay::ConflictClause::Abort);
    tc.addCol(EPD_PL_SEX, SqliteOverlay::ColumnDataType::Integer, SqliteOverlay::ConflictClause::NotUsed, SqliteOverlay::ConflictClause::NotUsed);
    tc.createTableAndResetCreator(*this, TAB_EPD_PLAYER);
  }

  //----------------------------------------------------------------------------

  void ExternalPlayerDB::populateViews()
  {
  }

  //----------------------------------------------------------------------------

  ExternalPlayerDatabaseEntryList ExternalPlayerDB::searchForMatchingPlayers(const QString& substring)
  {
    // we need at least three characters for searching
    QString s = substring.trimmed();
    if (s.length() < 3) return ExternalPlayerDatabaseEntryList();

    // create a specific SQL statement that returns all
    // rows that contain the substring
    const std::string sql = "SELECT id FROM " + TAB_EPD_PLAYER + " WHERE " +
                            EPD_PL_FNAME + " LIKE ?1 or " +
                            EPD_PL_LNAME + " LIKE ?2 " +
                            "ORDER BY " + EPD_PL_LNAME + " ASC, " + EPD_PL_FNAME + " ASC";
    const std::string pattern = "%" + QString2StdString(substring) + "%";

    auto stmt = prepStatement(sql);
    stmt.bind(1, pattern);
    stmt.bind(2, pattern);

    ExternalPlayerDatabaseEntryList result;
    for (stmt.step(); stmt.hasData(); stmt.step())
    {
      int id = stmt.getInt(0);

      auto player = row2upEntry(SqliteOverlay::TabRow{*this, TAB_EPD_PLAYER, id, true});
      if (player.has_value()) result.push_back(*player);
    }

    return result;
  }

  //----------------------------------------------------------------------------

  ExternalPlayerDatabaseEntryList ExternalPlayerDB::getAllPlayers()
  {
    SqliteOverlay::DbTab playerTab{*this, TAB_EPD_PLAYER, false};
    SqliteOverlay::WhereClause wc;
    wc.addCol("id", ">", 0);   // match all rows
    wc.setOrderColumn_Asc(EPD_PL_LNAME);  // sort by last name first
    wc.setOrderColumn_Asc(EPD_PL_FNAME);  // then by given name

    ExternalPlayerDatabaseEntryList result;
    for (auto it = SqliteOverlay::TabRowIterator{*this, TAB_EPD_PLAYER, wc}; it.hasData(); ++it)
    {
      auto tmp = row2upEntry(*it);

      if (tmp.has_value()) result.push_back(*tmp);   // the QList creates a copy internally
    }

    return result;
  }

  //----------------------------------------------------------------------------

  opExternalPlayerDatabaseEntry ExternalPlayerDB::getPlayer(int id)
  {
    try
    {
      SqliteOverlay::TabRow r{*this, TAB_EPD_PLAYER, id};
      return row2upEntry(r);
    }
    catch (std::invalid_argument&)
    {
      return {};
    }
  }

  //----------------------------------------------------------------------------

  opExternalPlayerDatabaseEntry ExternalPlayerDB::getPlayer(const QString& fname, const QString& lname)
  {
    SqliteOverlay::WhereClause w;
    w.addCol(EPD_PL_FNAME, QString2StdString(fname));
    w.addCol(EPD_PL_LNAME, QString2StdString(lname));

    SqliteOverlay::DbTab playerTab{*this, TAB_EPD_PLAYER, false};
    auto r = playerTab.getSingleRowByWhereClause2(w);
    if (r.has_value()) return row2upEntry(*r);

    return {};
  }

  //----------------------------------------------------------------------------

  opExternalPlayerDatabaseEntry ExternalPlayerDB::storeNewPlayer(const ExternalPlayerDatabaseEntry& newPlayer)
  {
    if (hasPlayer(newPlayer.getFirstname(), newPlayer.getLastname()))
    {
      return {};
    }

    SqliteOverlay::ColumnValueClause cvc;
    cvc.addCol(EPD_PL_FNAME, QString2StdString(newPlayer.getFirstname()));
    cvc.addCol(EPD_PL_LNAME, QString2StdString(newPlayer.getLastname()));

    if (newPlayer.getSex() != DONT_CARE)
    {
      cvc.addCol(EPD_PL_SEX, static_cast<int>(newPlayer.getSex()));
    }

    SqliteOverlay::DbTab playerTab{*this, TAB_EPD_PLAYER, false};
    int newId = playerTab.insertRow(cvc);

    return getPlayer(newId);
  }

  //----------------------------------------------------------------------------

  bool ExternalPlayerDB::hasPlayer(const QString& fname, const QString& lname)
  {
    auto tmp = getPlayer(fname, lname);

    return tmp.has_value();
  }

  //----------------------------------------------------------------------------

  bool ExternalPlayerDB::updatePlayerSexIfUndefined(int extPlayerId, SEX newSex)
  {
    // only permit updates with a defined sex
    if (newSex == DONT_CARE) return false;

    // check for a valid player ID
    auto pl = getPlayer(extPlayerId);
    if (!pl.has_value()) return false;

    // no modification if the player's sex is already defined
    if (pl->getSex() != DONT_CARE) return false;

    // update the player entry
    SqliteOverlay::DbTab playerTab{*this, TAB_EPD_PLAYER, false};
    playerTab[extPlayerId].update(EPD_PL_SEX, static_cast<int>(newSex));

    return true;
  }

  //----------------------------------------------------------------------------

  std::tuple<QList<int>, QList<int>, QHash<int,QString>, int> ExternalPlayerDB::bulkImportCSV(const QString& csv)
  {
    int errorCnt = 0;
    QList<int> newExtPlayerIds;
    QList<int> skippedPlayerIds;
    QHash<int,QString> extPlayerId2TeamName;

    for (QString line : csv.split("\n"))
    {
      // ignore empty lines
      line = line.trimmed();
      if (line.isEmpty()) continue;

      // ignore lines with less than to fields
      QStringList col = line.split(",");
      if (col.length() < 2)
      {
        ++errorCnt;
        continue;
      }

      // get the name information
      QString lName = col[0].trimmed();
      QString fName = col[1].trimmed();
      if ((lName.isEmpty()) || (fName.isEmpty()))
      {
        ++errorCnt;
        continue;
      }

      // get the player's sex, if provided
      SEX sex = DONT_CARE;
      if (col.length() > 2)
      {
        QString s = col[2].trimmed();
        if (s.toLower() == "m") sex = M;
        if (s.toLower() == "f") sex = F;
        if ((!(s.isEmpty())) && (sex == DONT_CARE))
        {
          // the provided value couldn't be recognized
          ++errorCnt;
          continue;
        }
      }

      // get the player's team name, if provided
      QString teamName = "";
      if (col.length() > 3)
      {
        teamName = col[3].trimmed();
      }

      // check if the player name already exists
      auto existingPlayer = getPlayer(fName, lName);
      if (existingPlayer.has_value())
      {
        int existingId = existingPlayer->getId();
        skippedPlayerIds.push_back(existingId);
        extPlayerId2TeamName[existingId] = teamName;
        continue;
      }

      // actually create the new entry
      ExternalPlayerDatabaseEntry entry{fName, lName, sex};
      auto newPlayer = storeNewPlayer(entry);
      if (!newPlayer.has_value()) ++errorCnt;

      int newExtPlayerId = newPlayer->getId();
      newExtPlayerIds.push_back(newExtPlayerId);
      extPlayerId2TeamName[newExtPlayerId] = teamName;
    }

    return std::make_tuple(newExtPlayerIds, skippedPlayerIds, extPlayerId2TeamName, errorCnt);
  }

  //----------------------------------------------------------------------------


  //----------------------------------------------------------------------------

  ExternalPlayerDatabaseEntry::ExternalPlayerDatabaseEntry(int _id, const QString& _fname, const QString& _lname, SEX _sex)
    :id(_id), fName(_fname.trimmed()), lName(_lname.trimmed()), sex(_sex)
  {
    if ((fName.isEmpty()) || (lName.isEmpty()))
    {
      throw std::invalid_argument("Received empty string in ctor for ExternalPlayerDatabaseEntry");
    }

    if (_id < 1)
    {
      throw std::invalid_argument("Received invalid ID in ctor for ExternalPlayerDatabaseEntry");
    }
  }

  //----------------------------------------------------------------------------

  ExternalPlayerDatabaseEntry::ExternalPlayerDatabaseEntry(const QString& _fname, const QString& _lname, SEX _sex)
    :id(-1), fName(_fname.trimmed()), lName(_lname.trimmed()), sex(_sex)
  {
    if ((fName.isEmpty()) || (lName.isEmpty()))
    {
      throw std::invalid_argument("Received empty string in ctor for ExternalPlayerDatabaseEntry");
    }
  }

  //----------------------------------------------------------------------------

  QString ExternalPlayerDatabaseEntry::getLastname() const
  {
    return lName;
  }

  //----------------------------------------------------------------------------

  QString ExternalPlayerDatabaseEntry::getFirstname() const
  {
    return fName;
  }

  //----------------------------------------------------------------------------

  QString ExternalPlayerDatabaseEntry::getDisplayName() const
  {
    return lName + ", " + fName;
  }

  std::function<bool (ExternalPlayerDatabaseEntry&, ExternalPlayerDatabaseEntry&)> ExternalPlayerDatabaseEntry::getPlayerSortFunction_byName()
  {
    return [](ExternalPlayerDatabaseEntry& e1, ExternalPlayerDatabaseEntry& e2) {
      // compare last names
      int cmpLast = QString::localeAwareCompare(e1.getLastname(), e2.getLastname());
      if (cmpLast < 0) return true;
      if (cmpLast > 0) return false;

      // identical last names, compare given names
      int cmpFirst = QString::localeAwareCompare(e1.getFirstname(), e2.getFirstname());
      if (cmpFirst < 0) return true;
      if (cmpFirst > 0) return false;

      // identical names. Shouldn't happen. Return the older entry first
      return (e1.id < e2.id);
    };
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

}
