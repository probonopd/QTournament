
#include <exception>

#include <assert.h>

#include "ExternalPlayerDB.h"
#include "KeyValueTab.h"
#include "TournamentDataDefs.h"

using namespace dbOverlay;

namespace QTournament
{

  ExternalPlayerDB::ExternalPlayerDB(QString fName, bool createNew)
    : GenericDatabase(fName, createNew)
  {
    populateTables();
    populateViews();
  }

  //----------------------------------------------------------------------------

  upExternalPlayerDatabaseEntry ExternalPlayerDB::row2upEntry(const TabRow& r) const
  {
    ExternalPlayerDatabaseEntry* entry = new ExternalPlayerDatabaseEntry(
          r.getId(), r[EPD_PL_FNAME].toString(), r[EPD_PL_LNAME].toString(),
          r[EPD_PL_SEX].isNull() ? DONT_CARE : static_cast<SEX>(r[EPD_PL_SEX].toInt()));

    return upExternalPlayerDatabaseEntry(entry);
  }

  //----------------------------------------------------------------------------

  unique_ptr<ExternalPlayerDB> ExternalPlayerDB::createNew(const QString& fname)
  {
    // try to create the new database
    upExternalPlayerDB result;
    try
    {
      auto rawPointer = new ExternalPlayerDB(fname, true);
      result = upExternalPlayerDB(rawPointer);
    }
    catch (exception e)
    {
      return nullptr;
    }

    // write the database version to the file
    auto cfg = KeyValueTab::getTab(result.get(), TAB_EPD_CFG);
    cfg.set(CFG_KEY_EPD_DB_VERSION, EXT_PLAYER_DB_VERSION);

    result->setLogLevel(1);

    return result;
  }

  //----------------------------------------------------------------------------

  unique_ptr<ExternalPlayerDB> ExternalPlayerDB::openExisting(const QString& fname)
  {
    // try to open the existing database
    upExternalPlayerDB result;
    try
    {
      auto rawPointer = new ExternalPlayerDB(fname, false);
      result = upExternalPlayerDB(rawPointer);
    }
    catch (exception e)
    {
      return nullptr;
    }

    // check the database version
    auto cfg = KeyValueTab::getTab(result.get(), TAB_EPD_CFG);
    int actualDbVersion = cfg.getInt(CFG_KEY_EPD_DB_VERSION);
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
    QStringList cols;
    QString nameTypeDef = " VARCHAR(" + QString::number(MAX_NAME_LEN) + ")";
    QString nameFieldDef = GENERIC_NAME_FIELD_NAME + nameTypeDef;
    
    // Generate the key-value-table with the database version
    dbOverlay::KeyValueTab::getTab(this, TAB_EPD_CFG);

    // Generate the table for the players
    cols.clear();
    cols << EPD_PL_FNAME + nameTypeDef;
    cols << EPD_PL_LNAME + nameTypeDef;;
    cols << EPD_PL_SEX + " INTEGER";
    tableCreationHelper(TAB_EPD_PLAYER, cols);
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
    QString where = EPD_PL_FNAME + " LIKE '%" + s + "%' or ";
    where += EPD_PL_LNAME + " LIKE '%" + s + "%'";
    where += "ORDER BY " + EPD_PL_LNAME + " ASC, " + EPD_PL_FNAME + " ASC";

    // search for names matching this pattern
    DbTab playerTab = getTab(TAB_EPD_PLAYER);
    DbTab::CachingRowIterator it = playerTab.getRowsByWhereClause(where);
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
    DbTab playerTab = getTab(TAB_EPD_PLAYER);
    if (playerTab.getMatchCountForColumnValue("id", id) != 1)
    {
      return nullptr;
    }

    TabRow r = playerTab[id];

    return row2upEntry(r);
  }

  //----------------------------------------------------------------------------

  upExternalPlayerDatabaseEntry ExternalPlayerDB::getPlayer(const QString& fname, const QString& lname)
  {
    QString where = "%1 = '%2' AND %3 = '%4'";
    where = where.arg(EPD_PL_FNAME).arg(fname);
    where = where.arg(EPD_PL_LNAME).arg(lname);

    DbTab playerTab = getTab(TAB_EPD_PLAYER);
    if (playerTab.getMatchCountForWhereClause(where) != 1)
    {
      return nullptr;
    }

    TabRow r = playerTab.getSingleRowByWhereClause(where);

    return row2upEntry(r);
  }

  //----------------------------------------------------------------------------

  upExternalPlayerDatabaseEntry ExternalPlayerDB::storeNewPlayer(const ExternalPlayerDatabaseEntry& newPlayer)
  {
    if (hasPlayer(newPlayer.getFirstname(), newPlayer.getLastname()))
    {
      return nullptr;
    }

    QVariantList qvl;
    qvl << EPD_PL_FNAME << newPlayer.getFirstname();
    qvl << EPD_PL_LNAME << newPlayer.getLastname();

    if (newPlayer.getSex() != DONT_CARE)
    {
      qvl << EPD_PL_SEX << static_cast<int>(newPlayer.getSex());
    }

    DbTab playerTab = getTab(TAB_EPD_PLAYER);
    int newId = playerTab.insertRow(qvl);
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
    DbTab playerTab = getTab(TAB_EPD_PLAYER);
    playerTab[extPlayerId].update(EPD_PL_SEX, static_cast<int>(newSex));

    return true;
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
