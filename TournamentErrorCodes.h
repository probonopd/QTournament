/* 
 * File:   TournamentErrorCodes.h
 * Author: nyoknvk1
 *
 * Created on 18. Februar 2014, 14:17
 */

#ifndef TOURNAMENTERRORCODES_H
#define	TOURNAMENTERRORCODES_H

namespace QTournament
{
    enum ERR {
        OK = 0,
        INVALID_NAME = -10000,
        NAME_EXISTS,
        NOT_USING_TEAMS
    };
}

#endif	/* TOURNAMENTERRORCODES_H */

