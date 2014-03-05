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
    GenericDatabaseObject (dbOverlay::TabRow _row);
    int getId ();

    inline bool operator== (const GenericDatabaseObject& other) const
    {
      return (other.row == row);
    }

    inline bool operator!= (const GenericDatabaseObject& other) const
    {
      return (!(this->operator == (other)));
    }

  protected:
    dbOverlay::TabRow row;

  };
}

#endif	/* GENERICDATABASEOBJECT_H */

