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

#ifndef TOURNAMENTDB_H
#define	TOURNAMENTDB_H

#include <tuple>
#include <string>
#include <vector>
#include <memory>

#include <SqliteOverlay/SqliteDatabase.h>
#include <SqliteOverlay/Transaction.h>

#include "TournamentDataDefs.h"
#include "TournamentErrorCodes.h"
#include "OnlineMngr.h"

namespace QTournament
{
  // the default transaction type for all transactional database operations
  static constexpr SqliteOverlay::TransactionType DefaultTransactionType{SqliteOverlay::TransactionType::Immediate};

  class TournamentDB : public SqliteOverlay::SqliteDatabase
  {
    friend class SqliteOverlay::SqliteDatabase;

  public:
    /** \brief Ctor for an empty, dummy in-memory tournament database with default settings
     */
    TournamentDB();

    /** \brief Ctor for creating a new datbase file with user provided settings
     */
    TournamentDB(const std::string& fName, const TournamentSettings& cfg);

    /** \brief Ctor for opening an existing datbase
     */
    TournamentDB(const std::string& fName);

    ~TournamentDB() override = default;

    void populateTables() override;
    void populateViews() override;
    void createIndices();

    std::tuple<int, int> getVersion();

    bool isCompatibleDatabaseVersion();

    bool needsConversion();
    bool convertToLatestDatabaseVersion();

    // access to the tournament-wide instance of the OnlineMngr
    OnlineMngr* getOnlineManager() const;

    // conversion to CSV for syncing with the server
    std::tuple<std::string,int> tableDataToCSV(const std::string& tabName, const std::vector<Sloppy::estring>& colNames, int rowId=-1) const;
    std::tuple<std::string,int> tableDataToCSV(const std::string& tabName, const std::vector<Sloppy::estring>& colNames, const std::vector<int>& rowList) const;
    std::string getSyncStringForTable(const std::string& tabName, const std::vector<Sloppy::estring>& colNames, int rowId=-1) const;
    std::string getSyncStringForTable(const std::string& tabName, const std::vector<Sloppy::estring>& colNames, std::vector<int> rowList) const;

  protected:
    void initBlankDb(const TournamentSettings& cfg);

  private:

    std::unique_ptr<OnlineMngr> om;
  };

  /** \brief Creates a new, empty tournament database with a given file name
   *
   * \throws TournamentException with error code FileAlreadyExists if a file of the provided name already exists
   *
   * \throws TournamentException with error code Error::DatabaseError if the new database could not be created
   *
   * \returns the newly created database
   */
  TournamentDB createNew(const QString& fName, const TournamentSettings& cfg);

  /** \brief Opens an existing tournament database with a given file name
   *
   * \throws TournamentException with error code FileNotExisting if there is no such file
   *
   * \throws TournamentException with error code IncompatibleFileFormat if the valid is invalid or incompatible
   *
   * \returns a handle for the database file
   */
  TournamentDB openExisting(const QString& fName);
}

#endif	/* TOURNAMENTDB_H */

