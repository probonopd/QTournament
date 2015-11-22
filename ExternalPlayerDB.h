#ifndef EXTERNALPLAYERDB_H
#define	EXTERNALPLAYERDB_H

#include <memory>
#include <tuple>
#include <functional>

#include <QHash>

#include "GenericDatabase.h"
#include "DbTab.h"

#include "TournamentDataDefs.h"

using namespace dbOverlay;

namespace QTournament
{

#define EXT_PLAYER_DB_VERSION 1
#define MIN_REQUIRED_EXT_PLAYER_DB_VERSION 1
#define TAB_EPD_CFG QString("Config")
#define CFG_KEY_EPD_DB_VERSION QString("DatabaseVersion")

#define TAB_EPD_PLAYER QString("Player")
#define EPD_PL_FNAME QString("FirstName")
#define EPD_PL_LNAME QString("LastName")
#define EPD_PL_SEX QString("Sex")

  class ExternalPlayerDatabaseEntry
  {
  public:
    ExternalPlayerDatabaseEntry(int _id, const QString& _fname, const QString& _lname, SEX _sex = DONT_CARE);
    ExternalPlayerDatabaseEntry(const QString& _fname, const QString& _lname, SEX _sex = DONT_CARE);
    QString getLastname() const;
    QString getFirstname() const;
    QString getDisplayName() const;
    int getId();

  protected:
    int id;
    QString fName;
    QString lName;
    SEX sex;
    static std::function<bool (ExternalPlayerDatabaseEntry&, ExternalPlayerDatabaseEntry&)> getPlayerSortFunction_byName();
  };
  typedef unique_ptr<ExternalPlayerDatabaseEntry> upExternalPlayerDatabaseEntry;
  typedef QList<ExternalPlayerDatabaseEntry> ExternalPlayerDatabaseEntryList;

  //----------------------------------------------------------------------------

  class ExternalPlayerDB : public dbOverlay::GenericDatabase
  {
  public:
    static unique_ptr<ExternalPlayerDB> createNew(const QString& fname);
    static unique_ptr<ExternalPlayerDB> openExisting(const QString& fname);
    virtual void populateTables();
    virtual void populateViews();

    ExternalPlayerDatabaseEntryList searchForMatchingPlayers(const QString& substring) const;
    upExternalPlayerDatabaseEntry getPlayer(int id);
    upExternalPlayerDatabaseEntry storeNewPlayer(const ExternalPlayerDatabaseEntry& newPlayer) const;

  private:
    ExternalPlayerDB(QString fName, bool createNew);
    DbTab playerTab;
  };
  typedef unique_ptr<ExternalPlayerDB> upExternalPlayerDB;
}

#endif	/* EXTERNALPLAYERDB_H */

