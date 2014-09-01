/* 
 * File:   GenericDatabaseObject.cpp
 * Author: nyoknvk1
 * 
 * Created on 18. Februar 2014, 14:25
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