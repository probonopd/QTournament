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

#include "SqliteOverlay/TabRow.h"

#include "GenericDatabaseObject.h"
#include "TournamentDB.h"
#include "HelperFunc.h"

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
  : SqliteOverlay::GenericDatabaseObject(_db, QString2StdString(_tabName), _id)
{
}

//----------------------------------------------------------------------------

  GenericDatabaseObject::GenericDatabaseObject(TournamentDB* _db, SqliteOverlay::TabRow _row)
    : SqliteOverlay::GenericDatabaseObject(_db, _row)
  {
  }


//----------------------------------------------------------------------------

  // THIS FUNCTION WILL FAIL IF THE DATABASE
  // HAS NO "State" COLUMN OR IF THE VALUE IS
  // NOT SET!!!
  OBJ_STATE GenericDatabaseObject::getState() const
  {
    int stateInt = row.getInt(GENERIC_STATE_FIELD_NAME);
    
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
    return row.getInt(GENERIC_SEQNUM_FIELD_NAME);
  }
    
//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

}
