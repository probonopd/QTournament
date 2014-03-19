/* 
 * File:   GenericDatabaseObject.h
 * Author: nyoknvk1
 *
 * Created on 18. Februar 2014, 14:25
 */

#ifndef GENERICDATABASEOBJECT_H
#define	GENERICDATABASEOBJECT_H

#include <QString>
#include "TabRow.h"
#include "TournamentDB.h"

namespace QTournament
{

  class GenericDatabaseObject
  {
  public:
    GenericDatabaseObject (TournamentDB* _db, const QString& _tabName, int _id);
    GenericDatabaseObject (TournamentDB* _db, dbOverlay::TabRow _row);
    int getId () const;

    inline bool operator== (const GenericDatabaseObject& other) const
    {
      return (other.row == row);
    }

    inline bool operator!= (const GenericDatabaseObject& other) const
    {
      return (!(this->operator == (other)));
    }
    
    OBJ_STATE getState() const;
    void setState(OBJ_STATE newState);

  protected:
    TournamentDB* db;
    dbOverlay::TabRow row;

  };
}

#endif	/* GENERICDATABASEOBJECT_H */
