
#include <exception>

#include "ExternalPlayerDB.h"
#include "KeyValueTab.h"
#include "TournamentDataDefs.h"

using namespace dbOverlay;

namespace QTournament
{

  ExternalPlayerDB::ExternalPlayerDB(QString fName, bool createNew)
    : GenericDatabase(fName, createNew), playerTab(getTab(TAB_EPD_PLAYER))
  {
    populateTables();
    populateViews();
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

    return result;
  }

  //----------------------------------------------------------------------------

  unique_ptr<ExternalPlayerDB> ExternalPlayerDB::openExisting(const QString& fname)
  {
    // try to open the existing database
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

    // check the database version
    auto cfg = KeyValueTab::getTab(result.get(), TAB_EPD_CFG);
    int actualDbVersion = cfg.getInt(CFG_KEY_EPD_DB_VERSION);
    if (actualDbVersion != EXT_PLAYER_DB_VERSION)
    {
      return nullptr;
    }

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

  ExternalPlayerDatabaseEntryList ExternalPlayerDB::searchForMatchingPlayers(const QString& substring) const
  {
    // we need at least three characters for searching
    QString s = substring.trimmed();
    if (s.length() < 3) return ExternalPlayerDatabaseEntryList();

    // create a WHERE clause that matches any name with the substring in it
    QString where = EPD_PL_FNAME + " LIKE '%" + s + "%' or ";
    where += EPD_PL_LNAME + " LIKE '%" + s + "%'";
    where += "ORDER BY " + EPD_PL_LNAME + " ASC, " + EPD_PL_FNAME + " ASC";

    // search for names matching this pattern
    DbTab::CachingRowIterator it = playerTab.getRowsByWhereClause(where);
    ExternalPlayerDatabaseEntryList result;
    while (!(it.isEnd()))
    {
      TabRow r = *it;
      ExternalPlayerDatabaseEntry entry{r.getId(), r[EPD_PL_FNAME].toString(),
            r[EPD_PL_LNAME].toString(),
            r[EPD_PL_SEX].isNull() ? DONT_CARE : static_cast<SEX>(r[EPD_PL_SEX].toInt())};

      result.push_back(entry);

      ++it;
     }
      return result;
    }

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
