/*
 *    This is QTournament, a badminton tournament management program.
 *    Copyright (C) 2014 - 2015  Volker Knollmann
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
        CATEGORY_UNFREEZEABLE,
        CATEGORY_NEEDS_NO_GROUP_ASSIGNMENTS,
        INVALID_PLAYER_COUNT,
        CATEGORY_NEEDS_NO_SEEDING,
        MATCH_GROUP_NOT_CONFIGURALE_ANYMORE,
        MATCH_NOT_CONFIGURALE_ANYMORE,
        PLAYER_ALREADY_ASSIGNED_TO_OTHER_MATCH_IN_THE_SAME_ROUND_AND_CATEGORY,
        CATEGORY_STILL_CONFIGURABLE,
        GROUP_NUMBER_MISMATCH,
        WRONG_STATE,
        MATCH_GROUP_ALREADY_CLOSED,
        MATCH_GROUP_EMPTY,
        MATCH_GROUP_NOT_UNSTAGEABLE,
        COURT_NUMBER_EXISTS,
        NO_MATCH_AVAIL,
        NO_COURT_AVAIL,
        ONLY_MANUAL_COURT_AVAIL,
        MATCH_NOT_RUNNABLE,
        COURT_BUSY,
        COURT_DISABLED,
        NO_MATCH_RESULT_SET,
        INVALID_MATCH_RESULT_FOR_CATEGORY_SETTINGS,
        INCONSISTENT_MATCH_RESULT_STRING,
        MATCH_NOT_RUNNING,
        NO_COURT_ASSIGNED,
        PLAYER_NOT_IDLE,
        PLAYER_NOT_PLAYING,
        ROUND_NOT_FINISHED,
        RANKING_ALREADY_EXISTS,
        MISSING_RANKING_ENTRIES,
        INVALID_SEEDING_LIST,
        INVALID_PLAYER_PAIR,
        INVALID_MATCH_LINK,
        INVALID_RANK,
        PLAYER_ALREADY_IN_MATCHES,
        NOT_ALL_PLAYERS_REGISTERED,
        EPD__NOT_FOUND,
        EPD__NOT_OPENED,
        EPD__NOT_CONFIGURED,
        EPD__CREATION_FAILED,
        EPD__NO_SEX_FOR_PLAYER_DEFINED,
        EPD__INVALID_DATABASE_NAME,
        FILE_ALREADY_EXISTS,
        FILE_NOT_EXISTING,
        INCOMPATIBLE_FILE_FORMAT
    };
}

#endif	/* TOURNAMENTERRORCODES_H */

