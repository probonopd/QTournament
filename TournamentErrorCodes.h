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
        PLAYER_NOT_REMOVABLE_FROM_CATEGORY,
        PLAYER_ALREADY_PAIRED,
        NO_CATEGORY_FOR_PAIRING,
        CATEGORY_NOT_CONFIGURALE_ANYMORE,
        PLAYERS_NOT_A_PAIR,
        PLAYERS_IDENTICAL,
        INVALID_ID,
        INVALID_RECONFIG,
        INVALID_ROUND,
        INVALID_GROUP_NUM,
        MATCH_GROUP_EXISTS,
        NO_SUCH_MATCH_GROUP,
        CONFIG_ALREADY_FROZEN,
        UNPAIRED_PLAYERS,
        INVALID_KO_CONFIG,
        CATEGORY_NOT_YET_FROZEN,
        CATEGORY_UNFREEZEABLE
    };
}

#endif	/* TOURNAMENTERRORCODES_H */

