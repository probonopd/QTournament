/*
 *    This is QTournament, a badminton tournament management program.
 *    Copyright (C) 2014 - 2017  Volker Knollmann
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

#ifndef TOURNAMENTDATADEFS_H
#define	TOURNAMENTDATADEFS_H

#include <QString>

namespace QTournament
{
#define DB_VERSION_MAJOR 3
#define DB_VERSION_MINOR 0
#define MIN_REQUIRED_DB_VERSION 3

//----------------------------------------------------------------------------

#define GENERIC_NAME_FIELD_NAME "Name"
#define MAX_NAME_LEN 50
  
#define GENERIC_STATE_FIELD_NAME "ObjState"
  
#define GENERIC_SEQNUM_FIELD_NAME "SequenceNumber"

//----------------------------------------------------------------------------

#define MAX_GROUP_SIZE 50
#define MAX_GROUP_COUNT 50
  
//----------------------------------------------------------------------------

#define MAX_RANDOMIZATION_ROUNDS 100    // for group assignments, player pairs, etc.

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    
    
#define DB_NULL QVariant::Int

//----------------------------------------------------------------------------
    
#define TAB_CFG "Config"
#define CFG_KEY_DB_VERSION "DatabaseVersion"
#define CFG_KEY_TNMT_NAME "TournamentName"
#define CFG_KEY_TNMT_ORGA "OrganizingClub"
#define CFG_KEY_USE_TEAMS "UseTeams"
#define CFG_KEY_EXT_PLAYER_DB "ExternalPlayerDatabase"
#define CFG_KEY_DEFAULT_RefereeMode "DefaultRefereeMode"
#define CFG_KEY_REFEREE_TEAM_ID "RefereeTeamId"
#define CFG_KEY_KEYSTORE "Keystore"
#define CFG_KEY_REGISTRATION_TIMESTAMP "RegistrationTimestamp"
//#define CFG_KEY_ ""

//----------------------------------------------------------------------------
    
#define TAB_PLAYER "Player"
#define PL_FNAME "FirstName"
#define PL_LNAME "LastName"
#define PL_TEAM_REF "TeamRefId"
#define PL_SEX "Sex"
#define PL_Referee_COUNT "RefereeCount"
//#define PLAYING ""
//#define PLAYING ""
//#define PLAYING ""
//#define PLAYING ""
//#define PLAYING ""
//#define PLAYING ""
//#define PLAYING ""
//#define PLAYING ""
//#define PLAYING ""
//#define PLAYING ""
//#define PLAYING ""
//#define PLAYING ""
//#define PLAYING ""
//#define PLAYING ""
//#define PLAYING ""
//#define PLAYING ""
//#define PLAYING ""

//----------------------------------------------------------------------------
    
#define TAB_TEAM "Team"
//#define TE_ ""

//----------------------------------------------------------------------------
    
#define TAB_COURT "Court"
#define CO_NUMBER "Number"
#define CO_IS_MANUAL_ASSIGNMENT "IsManualAssignment"
//#define CO_ ""
//#define CO_ ""
//#define CO_ ""
//#define CO_ ""
//#define CO_ ""

//----------------------------------------------------------------------------
    
#define TAB_CATEGORY "Category"
#define CAT_MatchType "MatchType"
#define CAT_Sex "Sex"
#define CAT_Sys "System"
#define CAT_AcceptDraw "AcceptDraw"
#define CAT_WinScore "WinScore"
#define CAT_DrawScore "DrawScore"
#define CAT_GroupConfig "GroupConfig"
#define CAT_BracketVisData "BracketVisData"
#define CAT_RoundRobinIterations "RoundRobinIterations"
//#define CAT_ ""
  
//----------------------------------------------------------------------------

#define TAB_P2C "Player2Category"
#define P2C_PLAYER_REF "PlayerRefId"
#define P2C_CAT_REF "CategoryRefId"
  
//----------------------------------------------------------------------------

#define TAB_PAIRS "PlayerPair"
#define PAIRS_PLAYER1_REF "Player1RefId"
#define PAIRS_PLAYER2_REF "Player2RefId"
#define PAIRS_CONFIGREF "CategoryRefId"
#define PAIRS_GRP_NUM "GroupNumber"
#define PAIRS_INITIAL_RANK "InitialRank"
  
//----------------------------------------------------------------------------

#define TAB_MATCH "Match"
#define MA_GRP_REF  "MatchGroupRefId"
#define MA_NUM  "Number"
#define MA_PAIR1_REF  "PlayerPair1RefId"
#define MA_PAIR2_REF  "PlayerPair2RefId"
#define MA_ACTUAL_PLAYER1A_REF  "ActualPlayer1aRefId"
#define MA_ACTUAL_PLAYER1B_REF  "ActualPlayer1bRefId"
#define MA_ACTUAL_PLAYER2A_REF  "ActualPlayer2aRefId"
#define MA_ACTUAL_PLAYER2B_REF  "ActualPlayer2bRefId"
#define MA_RESULT  "Result"
#define MA_COURT_REF  "CourtRefId"
#define MA_START_TIME  "StartTime"
#define MA_ADDITIONAL_CALL_TIMES  "CallTimes"
#define MA_FINISH_TIME  "FinishTime"
#define MA_PAIR1_SYMBOLIC_VAL  "PlayerPair1SymbolicValue"
#define MA_PAIR2_SYMBOLIC_VAL  "PlayerPair2SymbolicValue"
#define MA_WINNER_RANK  "WinnerRank"
#define MA_LOSER_RANK  "LoserRank"
#define MA_RefereeMode  "RefereeMode"
#define MA_REFEREE_REF  "RefereeRefId"
//#define MA_  ""
//#define MA_  ""
//#define MA_  ""
//#define MA_  ""
  
//----------------------------------------------------------------------------

#define TAB_MATCH_GROUP "MatchGroup"
#define MG_CAT_REF  "CategoryRefId"
#define MG_ROUND  "Round"
#define MG_GRP_NUM  "RoundRobinGroupNumber"
#define MG_STAGE_SEQ_NUM  "StageSequenceNumber"
//#define MG_  ""
//#define MG_  ""
//#define MG_  ""
//#define MG_  ""
//#define MG_  ""
//#define MG_  ""
//#define MG_  ""
//#define MG_  ""
//#define MG_  ""
//#define MG_  ""
//#define MG_  ""
//#define MG_  ""
//#define MG_  ""
//#define MG_  ""
//#define MG_  ""
//#define MG_  ""
//#define MG_  ""
//#define MG_  ""
//#define MG_  ""
  
//----------------------------------------------------------------------------

#define TAB_MatchSystem "Ranking"
#define RA_ROUND  "Round"
#define RA_PAIR_REF  "PlayerPairRef"
#define RA_GAMES_WON  "GamesWon"
#define RA_GAMES_LOST  "GamesLost"
#define RA_MATCHES_WON  "MatchesWon"
#define RA_MATCHES_LOST  "MatchesLost"
#define RA_MATCHES_DRAW  "MatchesDraw"
#define RA_POINTS_WON  "PointsWon"
#define RA_POINTS_LOST  "PointsLost"
#define RA_RANK  "Rank"
#define RA_CAT_REF  "CategoryRef"
#define RA_GRP_NUM  "MatchGroupNumber"
//#define RA_  ""
//#define RA_  ""
//#define RA_  ""

//----------------------------------------------------------------------------

#define TAB_BRACKET_VIS "BracketVisualization"
#define BV_MATCH_REF "MatchRefId"
#define BV_CONFIGREF "CategoryRefId"
#define BV_PAGE "PageNumber"
#define BV_GRID_X0 "GridX0"
#define BV_GRID_Y0 "GridY0"
#define BV_SPAN_Y "SpanY"
#define BV_ORIENTATION "Orientation"
#define BV_TERMINATOR "Terminator"
#define BV_INITIAL_RANK1 "InitialRank1"
#define BV_INITIAL_RANK2 "InitialRank2"
#define BV_PAIR1_REF "PlayerPair1RefId"
#define BV_PAIR2_REF "PlayerPair2RefId"
#define BV_Y_PAGEBREAK_SPAN "PagebreakOffsetY"
#define BV_NEXT_PAGE_NUM "NextPageNum"
#define BV_TERMINATOR_OFFSET_Y "TerminatorOffset"
#define BV_ELEMENT_ID "BracketElementId"
#define BV_NEXT_WINNER_MATCH "NextWinnerMatch"
#define BV_NEXT_LOSER_MATCH "NextLoserMatch"
#define BV_NEXT_MATCH_POS_FOR_WINNER "NextMatchPosForWinner"
#define BV_NEXT_MATCH_POS_FOR_LOSER "NextMatchPosForLoser"
//#define BV_ ""
//#define BV_ ""

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

  
//----------------------------------------------------------------------------

  
//----------------------------------------------------------------------------

  
//----------------------------------------------------------------------------

  enum class CatParameter {
    AllowDraw,
    WinScore,
    DrawScore,
    GroupConfig,
    RoundRobinIterations
    
  };
  
//----------------------------------------------------------------------------

  enum class CatAddState {
    WrongSex,         // Player may not join because the sex doesn't fit
    CanJoin,          // Player can join the category now and participate in matches
    CatClosed,        // Player WOULD fit, but the category doesn't accept any more players
    AlreadyMember     // Player is already assigned to this category
  };
  
//----------------------------------------------------------------------------

  //
  // IMPORTANT: only append new states AT THE END of the list or otherwise the
  // state IDs change and we break compatibility with older database versions!
  //
  enum class ObjState {
    PL_Idle,
    PL_Playing,
    PL_WaitForRegistration,  // player has to report to match control before considered "ready"
    CAT_Config,
    CAT_Frozen,  // intermediate state in which the category can be configured for the first round
    CAT_Idle,
    CAT_Playing,  // at least one match in this category is currently being played
    CAT_Finalized, // no more rounds or matches to come
    CAT_WaitForIntermediateSeeding,  // we need user-controlled seeding / match generation before we can continue
    MG_Config,   // Match group has been created, matches can still be added or removed
    MG_Frozen,   // No more adding or removing of matches; match group CANNOT be staged because earlier rounds/groups have to be staged / scheduled first
    MG_Idle,     // No more adding or removing of matches; match group can be staged and waits for being staged
    MG_Staged,     // No more adding or removing of matches; match group is selected to be scheduled and waits for being scheduled
    MG_Scheduled, // Match numbers have been assigned
    MG_Finished,  // All matches in this group are finished
    MA_Incomplete,   // Match is not yet fully defined (e.g., player names or match number are missing)
    MA_Fuzzy,        // Player names are defined by symbolic values (e.g., winner of match XYZ); match number is assigned; match cannot be called
    MA_Waiting,      // Player names and match number are assigned but match cannot be called because earlier rounds have to be played first
    MA_Ready,        // Opponents and match number are fully defined and all players are idle; match can be called
    MA_Busy,         // Opponents and match number are fully defined but some players are busy; match cannot be called
    MA_Running,      // The match is currently being played
    MA_Finished,     // The match is finished and the result has been entered
    MA_Postponed,    // The match is postponed and cannot be called
    CO_Avail,        // The court is empty and can be assigned to a match
    CO_Busy,         // The court is being used by a match
    CO_Disabled,     // The court cannot be used (temporarily)
    PL_Referee,      // Player is currently acting as a referee/umpire for a match
  };
  
//----------------------------------------------------------------------------
  
  enum class MatchType {
    Singles,
    Doubles,
    Mixed
  };  
  
//----------------------------------------------------------------------------
  enum SEX
  {
    M,
    F,
    DONT_CARE
  };  
  
//----------------------------------------------------------------------------
    
  enum class MatchSystem {
    SwissLadder,
    GroupsWithKO,
    Randomize,
    Ranking,
    SingleElim,
    RoundRobin,
  };
  
//----------------------------------------------------------------------------

  enum class RefereeMode {
    None = 0,           // no umpire for the match
    HandWritten,    // the umpire's name is inserted manually be the user on the match sheet
    AllPlayers,    // the umpire is selected upon match start among all players
    RecentFinishers,  // the umpire is selected upon match start among the recent match finishers
    SpecialTeam,    // the umpire is selected upon match start among the members of a special team (--> dedicated umpires)
    UseDefault = -1,  // use the current tournament default
  };

//----------------------------------------------------------------------------

  class TournamentSettings
  {
  public:
    QString tournamentName;
    QString organizingClub;
    RefereeMode refereeMode;
    bool useTeams;
  } ;

//----------------------------------------------------------------------------

  enum KO_Start {
    Final,
    Semi,
    Quarter,
    L16
  };

//----------------------------------------------------------------------------

  // Special group numbers for MatchGroup objects
  // Positive numbers indicate the number of the round robin group within a category,
  // Negative numbers have a special meaning for MatchGroups of non-round-robin matches
  constexpr int GroupNum_Final = -1;
  constexpr int GroupNum_Semi = -2;
  constexpr int GroupNum_Quarter = -4;
  constexpr int GroupNum_L16 = -8;
  constexpr int GroupNum_Iteration = -100;  // just a normal round in Swiss Ladder, Random Matches, ...
  
  // A special group number indicating that the match is
  // not in any group at all
  constexpr int GroupNum_NotAssigned = -1;

  // A special value that matches any players group
  // Is only used for return values of functions
  constexpr int AnyPlayersGroupNumber = -200;

  //----------------------------------------------------------------------------

  // A special match number, indicating that a match number
  // has not yet been assigned
  constexpr int MatchNumNotAssigned = -1;

//----------------------------------------------------------------------------

  enum OnlineRegState
  {
    Off,         // no password set, not online, nothing
    HasKeys,     // password for remote communication set by user
    RegSent,     // registration filed with server
    Pending,     // registration email confirmed by user; registration not confirmed by admin
    Good,        // tournament registered and activated
    Expired      // no more synching with server, tournament is in the past
  };

//----------------------------------------------------------------------------

  enum RemoteTransportResult
  {
    NoConnectionToServer,    // no low-level TCP-connection possible
    TimeoutAfterRequest,     // request sent but no reply received
    CryptoError,             // signature not valid
    MsgCorrupted,            // good signature but invalid content
    SessionExpired,          // the server refused the session token
    Okay
  };

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
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    
}

#endif	/* TOURNAMENTDATADEFS_H */

