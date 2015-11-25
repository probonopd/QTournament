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

#include <stdexcept>

#include "GenericDatabaseObject.h"
#include "TournamentDB.h"
#include "TabRow.h"

namespace QTournament
{

  /**
   * Constructor for a database object identified by table name and ID
   *
   * @param _tabName the name of the table the object refers to
   * @param _id the ID of the object in the table
   *
   */
GenericDatabaseObject::GenericDatabaseObject(TournamentDB* _db, const QString& _tabName, int _id)
: db(_db), row(dbOverlay::TabRow(_db, _tabName, _id))
{
}

//----------------------------------------------------------------------------

  GenericDatabaseObject::GenericDatabaseObject(TournamentDB* _db, dbOverlay::TabRow _row)
  :db(_db), row(_row)
  {
  }


//----------------------------------------------------------------------------

  int GenericDatabaseObject::getId() const
  {
    return row.getId();
  }

//----------------------------------------------------------------------------

  // THIS FUNCTION WILL FAIL IF THE DATABASE
  // HAS NO "State" COLUMN OR IF THE VALUE IS
  // NOT SET!!!
  OBJ_STATE GenericDatabaseObject::getState() const
  {
    int stateInt = row[GENERIC_STATE_FIELD_NAME].toInt();
    
    return static_cast<OBJ_STATE>(stateInt);
  }

//----------------------------------------------------------------------------

  // THIS FUNCTION WILL FAIL IF THE DATABASE
  // HAS NO "State" COLUMN
  void GenericDatabaseObject::setState(OBJ_STATE newState) const
  {
    int stateInt = static_cast<int>(newState);
    row.update(GENERIC_STATE_FIELD_NAME, stateInt);
  }

//----------------------------------------------------------------------------

  // THIS FUNCTION WILL FAIL IF THE DATABASE
  // HAS NO "SeqNum" COLUMN OR IF THE COLUMN IS NULL
  int GenericDatabaseObject::getSeqNum() const
  {
    return row[GENERIC_SEQNUM_FIELD_NAME].toInt();
  }
    
//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

}
