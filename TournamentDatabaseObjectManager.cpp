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

#include <SqliteOverlay/ClausesAndQueries.h>
#include <SqliteOverlay/TabRow.h>

#include "TournamentDatabaseObjectManager.h"
#include "HelperFunc.h"

namespace QTournament
{

  /**
   *  Fixes the sequence number column after a row has been inserted.
   *
   *  It relies on the following conditions and WILL FAIL otherwise:
   *    * The table has a column for the sequence number
   *    * The sequence number of the newly inserted row AND ONLY OF THIS ROW is Null
   *    * Except for the new row, all other sequence numbers in the table are correct
   *
   */
  void TournamentDatabaseObjectManager::fixSeqNumberAfterInsert(const SqliteOverlay::DbTab& otherTab) const
  {
    auto r = otherTab.getSingleRowByColumnValueNull(GENERIC_SEQNUM_FIELD_NAME);
    
    int newSeqNum = otherTab.length() - 1;
    
    r.update(GENERIC_SEQNUM_FIELD_NAME, newSeqNum);
  }

  //----------------------------------------------------------------------------

  void TournamentDatabaseObjectManager::fixSeqNumberAfterInsert() const
  {
    fixSeqNumberAfterInsert(tab);
  }

  //----------------------------------------------------------------------------

  void TournamentDatabaseObjectManager::fixSeqNumberAfterDelete(int deletedSeqNum) const
  {
    fixSeqNumberAfterDelete(tab, deletedSeqNum);
  }

//----------------------------------------------------------------------------

  void TournamentDatabaseObjectManager::fixSeqNumberAfterDelete(const SqliteOverlay::DbTab& otherTab, int deletedSeqNum) const
  {
    SqliteOverlay::WhereClause wc;
    wc.addCol(GENERIC_SEQNUM_FIELD_NAME, ">", deletedSeqNum);
    wc.setOrderColumn_Asc(GENERIC_SEQNUM_FIELD_NAME);

    // make sure we update all rows in a single transaction
    auto trans = db.get().startTransaction(DefaultTransactionType);

    auto rows = otherTab.getRowsByWhereClause(wc);
    
    // re-number all items behind the deleted item
    int nextSeqNum = deletedSeqNum;
    for (const auto& row : rows)
    {
      row.update(GENERIC_SEQNUM_FIELD_NAME, nextSeqNum);
      ++nextSeqNum;
    }

    trans.commit();
  }

//----------------------------------------------------------------------------

  string TournamentDatabaseObjectManager::getSyncString(int rowId) const
  {
    vector<int> v = (rowId > 0) ? vector<int>{rowId} : vector<int>{};
    return getSyncString(v);
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
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    
}
