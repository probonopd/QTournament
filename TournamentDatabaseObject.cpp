/*
 *    This is QTournament, a badminton tournament management program.
 *    Copyright (C) 2014 - 2019  Volker Knollmann
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

#include <SqliteOverlay/TabRow.h>

#include "TournamentDatabaseObject.h"
#include "TournamentDB.h"
#include "HelperFunc.h"

namespace QTournament
{


//----------------------------------------------------------------------------

  // THIS FUNCTION WILL FAIL IF THE DATABASE
  // HAS NO "State" COLUMN OR IF THE VALUE IS
  // NOT SET!!!
  ObjState TournamentDatabaseObject::getState() const
  {
    int stateInt = row.getInt(GenericStateFieldName);
    
    return static_cast<ObjState>(stateInt);
  }

//----------------------------------------------------------------------------

  // THIS FUNCTION WILL FAIL IF THE DATABASE
  // HAS NO "State" COLUMN
  void TournamentDatabaseObject::setState(ObjState newState) const
  {
    int stateInt = static_cast<int>(newState);
    row.update(GenericStateFieldName, stateInt);
  }

//----------------------------------------------------------------------------

  // THIS FUNCTION WILL FAIL IF THE DATABASE
  // HAS NO "SeqNum" COLUMN OR IF THE COLUMN IS NULL
  int TournamentDatabaseObject::getSeqNum() const
  {
    return row.getInt(GenericSeqnumFieldName);
  }
    
//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

}
