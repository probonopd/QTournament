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
        NOT_USING_TEAMS,
        INVALID_SEX,
        INVALID_TEAM,
        CATEGORY_CLOSED_FOR_MORE_PLAYERS,
        PLAYER_NOT_SUITABLE,
        PLAYER_ALREADY_IN_CATEGORY,
        PLAYER_NOT_IN_CATEGORY,
        PLAYER_NOT_REMOVABLE_FROM_CATEGORY
    };
}

#endif	/* TOURNAMENTERRORCODES_H */

