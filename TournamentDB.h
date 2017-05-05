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

#include <SqliteOverlay/SqliteDatabase.h>
#include <SqliteOverlay/Transaction.h>

#include "TournamentDataDefs.h"
#include "TournamentErrorCodes.h"

namespace QTournament
{

  enum class TransactionState
  {
    Started,
    AlreadyRunning,
    Failed,
  };

  class TournamentDB : public SqliteOverlay::SqliteDatabase
  {
    friend class SqliteOverlay::SqliteDatabase;

  public:
    static unique_ptr<TournamentDB> createNew(const QString& fName, const TournamentSettings& cfg, ERR* err=nullptr);
    static unique_ptr<TournamentDB> openExisting(const QString& fName, ERR* err=nullptr);

    virtual void populateTables();
    virtual void populateViews();
    void createIndices();

    tuple<int, int> getVersion();

    bool isCompatibleDatabaseVersion();

    bool needsConversion();
    bool convertToLatestDatabaseVersion();

    // Transaction handling; should primarily be used
    // by the TransactioGuard
    bool isTransactionRunning() const;
    TransactionState beginNewTransaction(int* dbErr = nullptr);
    bool commitRunningTransaction(int* dbErr = nullptr);
    bool rollbackRunningTransaction(int* dbErr = nullptr);

    class TransactionGuard
    {
    public:
      TransactionGuard(TournamentDB* _db, bool _commitOnDestruction = false);
      ~TransactionGuard();
      bool commit(int* dbErr = nullptr);
      bool rollback(int* dbErr = nullptr);

    private:
      TournamentDB* db;
      bool commitOnDestruction;
    };

    unique_ptr<TransactionGuard> acquireTransactionGuard(bool commitOnDestruction, bool* isDbErr = nullptr, bool* transRunning = nullptr);

  private:
    TournamentDB(string fName, bool createNew);

    unique_ptr<SqliteOverlay::Transaction> curTrans;
  };

}

#endif	/* TOURNAMENTDB_H */

