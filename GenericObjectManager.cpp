/*
 *    This is QTournament, a badminton tournament management program.
 *    Copyright (C) 2014 - 2015  Volker Knollmann
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

#include <QtCore/qmetatype.h>

#include "GenericObjectManager.h"

using namespace dbOverlay;

namespace QTournament
{

  GenericObjectManager::GenericObjectManager(TournamentDB* _db)
  :db(_db), cfg(KeyValueTab::getTab(db, TAB_CFG))
  {

  }

//----------------------------------------------------------------------------

  /**
   *  Fixes the sequence number column after a row has been inserted.
   *
   *  It relies on the following conditions and WILL FAIL otherwise:
   *    * The table has a column for the sequence number
   *    * The sequence number of the newly inserted row AND ONLY OF THIS ROW is Null
   *    * Except for the new row, all other sequence numbers in the table are correct
   *
   */
  void GenericObjectManager::fixSeqNumberAfterInsert(const QString& tabName) const
  {
    TabRow r = db->getTab(tabName).getSingleRowByColumnValue(GENERIC_SEQNUM_FIELD_NAME, DB_NULL);
    
    int newSeqNum = db->getTab(tabName).length() - 1;
    
    r.update(GENERIC_SEQNUM_FIELD_NAME, newSeqNum);
  }


//----------------------------------------------------------------------------

  void GenericObjectManager::fixSeqNumberAfterDelete(const QString& tabName, int deletedSeqNum) const
  {
    QString where = GENERIC_SEQNUM_FIELD_NAME + " > ? ORDER BY " + GENERIC_SEQNUM_FIELD_NAME + " ASC";
    
    QVariantList qvl;
    qvl << deletedSeqNum;
    DbTab::CachingRowIterator it = db->getTab(tabName).getRowsByWhereClause(where, qvl);
    
    
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

  TournamentDB* GenericObjectManager::getDatabaseHandle()
  {
    return db;
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
