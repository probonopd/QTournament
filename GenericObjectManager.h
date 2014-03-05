/* 
 * File:   GenericObjectManager.h
 * Author: volker
 *
 * Created on March 2, 2014, 8:27 PM
 */

#ifndef GENERICOBJECTMANAGER_H
#define	GENERICOBJECTMANAGER_H

#include "KeyValueTab.h"
#include "TournamentDB.h"

using namespace dbOverlay;

namespace QTournament
{
  class GenericObjectManager
  {
  public:
    GenericObjectManager (const TournamentDB& _db);

  protected:
    TournamentDB db;
    KeyValueTab cfg;
  };

}

#endif	/* GENERICOBJECTMANAGER_H */

