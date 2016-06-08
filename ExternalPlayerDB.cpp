/*
 *    This is QTournament, a badminton tournament management program.
 *    Copyright (C) 2014 - 2016  Volker Knollmann
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
#include "KeyValueTab.h"
#include "TableCreator.h"
#include "ClausesAndQueries.h"

#include "HelperFunc.h"
#include "TournamentDataDefs.h"

namespace QTournament
{

  upExternalPlayerDatabaseEntry ExternalPlayerDB::row2upEntry(const SqliteOverlay::TabRow& r) const
  {
    auto sexValue = r.getInt2(EPD_PL_SEX);
    ExternalPlayerDatabaseEntry* entry = new ExternalPlayerDatabaseEntry(
          r.getId(), stdString2QString(r[EPD_PL_FNAME]), stdString2QString(r[EPD_PL_LNAME]),
          sexValue->isNull() ? DONT_CARE : static_cast<SEX>(sexValue->get()));

    return upExternalPlayerDatabaseEntry(entry);
  }

  //----------------------------------------------------------------------------

  ExternalPlayerDB::ExternalPlayerDB(const string& fname, bool createNew)
    :SqliteOverlay::SqliteDatabase(fname, createNew)
  {

  }

  //----------------------------------------------------------------------------

  unique_ptr<ExternalPlayerDB> ExternalPlayerDB::createNew(const QString& fname)
  {
    // try to create the new database
    upExternalPlayerDB result = SqliteDatabase::get<ExternalPlayerDB>(fname.toUtf8().constData(), true);
    if (result == nullptr) return nullptr;

    // write the database version to the file
    auto cfg = SqliteOverlay::KeyValueTab::getTab(result.get(), TAB_EPD_CFG, true);
    cfg->set(CFG_KEY_EPD_DB_VERSION, EXT_PLAYER_DB_VERSION);

    result->setLogLevel(1);

    return result;
  }

  //----------------------------------------------------------------------------

  unique_ptr<ExternalPlayerDB> ExternalPlayerDB::openExisting(const QString& fname)
  {
    // try to open the existing database
    upExternalPlayerDB result = SqliteDatabase::get<ExternalPlayerDB>(fname.toUtf8().constData(), false);
    if (result == nullptr) return nullptr;

    // check the database version
    auto cfg = SqliteOverlay::KeyValueTab::getTab(result.get(), TAB_EPD_CFG);
    int actualDbVersion = cfg->getInt(CFG_KEY_EPD_DB_VERSION);
    if (actualDbVersion != EXT_PLAYER_DB_VERSION)
    {
      return nullptr;
    }

    result->setLogLevel(1);

    return result;
  }

  //----------------------------------------------------------------------------

  void ExternalPlayerDB::populateTables()
  {
    // Generate the key-value-table with the database version
    SqliteOverlay::KeyValueTab::getTab(this, TAB_EPD_CFG);

    // Generate the table for the players
    SqliteOverlay::TableCreator tc{this};
    tc.addVarchar(EPD_PL_FNAME, MAX_NAME_LEN);
    tc.addVarchar(EPD_PL_LNAME, MAX_NAME_LEN);
    tc.addInt(EPD_PL_SEX);
    tc.createTableAndResetCreator(TAB_EPD_PLAYER);
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

    // create a WHERE clause that matches any name with the substring in it
    string _s = QString2StdString(s);
    string where = EPD_PL_FNAME + " LIKE '%" + _s + "%' or ";
    where += EPD_PL_LNAME + " LIKE '%" + _s + "%'";
    where += "ORDER BY " + EPD_PL_LNAME + " ASC, " + EPD_PL_FNAME + " ASC";

    // search for names matching this pattern
    SqliteOverlay::DbTab* playerTab = getTab(TAB_EPD_PLAYER);
    auto it = playerTab->getRowsByWhereClause(where);
    ExternalPlayerDatabaseEntryList result;
    while (!(it.isEnd()))
    {
      auto tmp = row2upEntry(*it);

      result.push_back(*tmp);   // the QList creates a copy internally

      ++it;

      // the object behind tmp is automatically deleted
      // when we leave this scope.
     }
    return result;
  }

  //----------------------------------------------------------------------------

  ExternalPlayerDatabaseEntryList ExternalPlayerDB::getAllPlayers()
  {
    SqliteOverlay::DbTab* playerTab = getTab(TAB_EPD_PLAYER);
    SqliteOverlay::WhereClause wc;
    wc.addIntCol("id", ">", 0);   // match all rows
    wc.setOrderColumn_Asc(EPD_PL_LNAME);  // sort by last name first
    wc.setOrderColumn_Asc(EPD_PL_FNAME);  // then by given name
    auto it = playerTab->getRowsByWhereClause(wc);
    ExternalPlayerDatabaseEntryList result;
    while (!(it.isEnd()))
    {
      auto tmp = row2upEntry(*it);

      result.push_back(*tmp);   // the QList creates a copy internally

      ++it;

      // the object behind tmp is automatically deleted
      // when we leave this scope.
    }
    return result;
  }

  //----------------------------------------------------------------------------

  upExternalPlayerDatabaseEntry ExternalPlayerDB::getPlayer(int id)
  {
    auto playerTab = getTab(TAB_EPD_PLAYER);
    if (playerTab->getMatchCountForColumnValue("id", id) != 1)
    {
      return nullptr;
    }

    auto r = playerTab->operator [](id);

    return row2upEntry(r);
  }

  //----------------------------------------------------------------------------

  upExternalPlayerDatabaseEntry ExternalPlayerDB::getPlayer(const QString& fname, const QString& lname)
  {
    SqliteOverlay::WhereClause w;
    w.addStringCol(EPD_PL_FNAME, QString2StdString(fname));
    w.addStringCol(EPD_PL_LNAME, QString2StdString(lname));

    auto playerTab = getTab(TAB_EPD_PLAYER);
    if (playerTab->getMatchCountForWhereClause(w) != 1)
    {
      return nullptr;
    }

    auto r = playerTab->getSingleRowByWhereClause(w);

    return row2upEntry(r);
  }

  //----------------------------------------------------------------------------

  upExternalPlayerDatabaseEntry ExternalPlayerDB::storeNewPlayer(const ExternalPlayerDatabaseEntry& newPlayer)
  {
    if (hasPlayer(newPlayer.getFirstname(), newPlayer.getLastname()))
    {
      return nullptr;
    }

    SqliteOverlay::ColumnValueClause cvc;
    cvc.addStringCol(EPD_PL_FNAME, QString2StdString(newPlayer.getFirstname()));
    cvc.addStringCol(EPD_PL_LNAME, QString2StdString(newPlayer.getLastname()));

    if (newPlayer.getSex() != DONT_CARE)
    {
      cvc.addIntCol(EPD_PL_SEX, static_cast<int>(newPlayer.getSex()));
    }

    auto playerTab = getTab(TAB_EPD_PLAYER);
    int newId = playerTab->insertRow(cvc);
    assert(newId > 0);

    return getPlayer(newId);
  }

  //----------------------------------------------------------------------------

  bool ExternalPlayerDB::hasPlayer(const QString& fname, const QString& lname)
  {
    auto tmp = getPlayer(fname, lname);

    return (tmp != nullptr);
  }

  //----------------------------------------------------------------------------

  bool ExternalPlayerDB::updatePlayerSexIfUndefined(int extPlayerId, SEX newSex)
  {
    // only permit updates with a defined sey
    if (newSex == DONT_CARE) return false;

    // check for a valid player ID
    auto pl = getPlayer(extPlayerId);
    if (pl == nullptr) return false;

    // no modification if the player's sex is already defined
    if (pl->getSex() != DONT_CARE) return false;

    // update the player entry
    auto playerTab = getTab(TAB_EPD_PLAYER);
    playerTab->operator [](extPlayerId).update(EPD_PL_SEX, static_cast<int>(newSex));

    return true;
  }

  //----------------------------------------------------------------------------

  tuple<QList<int>, QList<int>, QHash<int,QString>, int> ExternalPlayerDB::bulkImportCSV(const QString& csv)
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
      if (existingPlayer != nullptr)
      {
        int existingId = existingPlayer->getId();
        skippedPlayerIds.push_back(existingId);
        extPlayerId2TeamName[existingId] = teamName;
        continue;
      }

      // actually create the new entry
      ExternalPlayerDatabaseEntry entry{fName, lName, sex};
      auto newPlayer = storeNewPlayer(entry);
      if (newPlayer == nullptr) ++errorCnt;

      int newExtPlayerId = newPlayer->getId();
      newExtPlayerIds.push_back(newExtPlayerId);
      extPlayerId2TeamName[newExtPlayerId] = teamName;
    }

    return make_tuple(newExtPlayerIds, skippedPlayerIds, extPlayerId2TeamName, errorCnt);
  }

  //----------------------------------------------------------------------------


  //----------------------------------------------------------------------------

  ExternalPlayerDatabaseEntry::ExternalPlayerDatabaseEntry(int _id, const QString& _fname, const QString& _lname, SEX _sex)
    :id(_id), fName(_fname.trimmed()), lName(_lname.trimmed()), sex(_sex)
  {
    if ((fName.isEmpty()) || (lName.isEmpty()))
    {
      throw invalid_argument("Received empty string in ctor for ExternalPlayerDatabaseEntry");
    }

    if (_id < 1)
    {
      throw invalid_argument("Received invalid ID in ctor for ExternalPlayerDatabaseEntry");
    }
  }

  //----------------------------------------------------------------------------

  ExternalPlayerDatabaseEntry::ExternalPlayerDatabaseEntry(const QString& _fname, const QString& _lname, SEX _sex)
    :id(-1), fName(_fname.trimmed()), lName(_lname.trimmed()), sex(_sex)
  {
    if ((fName.isEmpty()) || (lName.isEmpty()))
    {
      throw invalid_argument("Received empty string in ctor for ExternalPlayerDatabaseEntry");
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
