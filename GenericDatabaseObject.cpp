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

  int GenericDatabaseObject::getId()
  {
    return row.getId();
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
    

}