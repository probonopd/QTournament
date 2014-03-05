/* 
 * File:   Tournament.h
 * Author: nyoknvk1
 *
 * Created on 18. Februar 2014, 12:31
 */

#ifndef TOURNAMENT_H
#define	TOURNAMENT_H

#include <QString>

#include "TournamentDataDefs.h"
#include "TournamentDB.h"
#include "KeyValueTab.h"
#include "TeamMngr.h"

using namespace dbOverlay;

namespace QTournament
{
  class Tournament
  {
  public:
    Tournament (const QString& fName, const TournamentSettings& cfg);
    Tournament (const QString& fName);
    TeamMngr getTeamMngr ();
    void close();

  private:
    TournamentDB db;
    TeamMngr tm;
    void initManagers ();

  };

}

#endif	/* TOURNAMENT_H */

