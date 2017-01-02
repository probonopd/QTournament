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
  void TournamentDatabaseObjectManager::fixSeqNumberAfterInsert(SqliteOverlay::DbTab* tabPtr) const
  {
    SqliteOverlay::DbTab* t = (tabPtr == nullptr) ? tab : tabPtr;
    auto r = t->getSingleRowByColumnValueNull(GENERIC_SEQNUM_FIELD_NAME);
    
    int newSeqNum = t->length() - 1;
    
    r.update(GENERIC_SEQNUM_FIELD_NAME, newSeqNum);
  }


//----------------------------------------------------------------------------

  void TournamentDatabaseObjectManager::fixSeqNumberAfterDelete(SqliteOverlay::DbTab* tabPtr, int deletedSeqNum) const
  {
    SqliteOverlay::DbTab* t = (tabPtr == nullptr) ? tab : tabPtr;

    SqliteOverlay::WhereClause wc;
    wc.addIntCol(GENERIC_SEQNUM_FIELD_NAME, ">", deletedSeqNum);
    wc.setOrderColumn_Asc(GENERIC_SEQNUM_FIELD_NAME);

    auto it = t->getRowsByWhereClause(wc);
    
    
    // re-number all items behind the deleted item
    int nextSeqNum = deletedSeqNum;
    while (!(it.isEnd()))
    {
      (*it).update(GENERIC_SEQNUM_FIELD_NAME, nextSeqNum);
      nextSeqNum++;
      ++it;
    }
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
    

//----------------------------------------------------------------------------
    
}
