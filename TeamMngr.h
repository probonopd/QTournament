/* 
 * File:   TeamMngr.h
 * Author: nyoknvk1
 *
 * Created on 18. Februar 2014, 14:04
 */

#ifndef TEAMMNGR_H
#define	TEAMMNGR_H

#include "TournamentDB.h"

namespace QTournament
{
class TeamMngr
{
public:
    TeamMngr(const TournamentDB& _db);
private:
    TournamentDB db;
};
}

#endif	/* TEAMMNGR_H */

