/* 
 * File:   TournamentDataDefs.h
 * Author: nyoknvk1
 *
 * Created on 18. Februar 2014, 12:48
 */

#ifndef TOURNAMENTDATADEFS_H
#define	TOURNAMENTDATADEFS_H

#include <QString>

namespace QTournament
{
#define DB_VERSION 2
#define MIN_REQUIRED_DB_VERSION 2

//----------------------------------------------------------------------------

#define GENERIC_NAME_FIELD_NAME QString("Name")
#define MAX_NAME_LEN 50
  
#define GENERIC_STATE_FIELD_NAME QString("ObjState")
  
#define GENERIC_SEQNUM_FIELD_NAME QString("SequenceNumber")

//----------------------------------------------------------------------------

#define MAX_GROUP_SIZE 50
#define MAX_GROUP_COUNT 50
  
//----------------------------------------------------------------------------

#define MAX_RANDOMIZATION_ROUNDS 100    // for group assignments, player pairs, etc.

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    
    
#define DB_NULL QVariant::Int

//----------------------------------------------------------------------------
    
#define TAB_CFG QString("Config")
#define CFG_KEY_DB_VERSION QString("DatabaseVersion")
#define CFG_KEY_TNMT_NAME QString("TournamentName")
#define CFG_KEY_TNMT_ORGA QString("OrganizingClub")
#define CFG_KEY_USE_TEAMS QString("UseTeams")
//#define CFG_KEY_ QString("")
//#define CFG_KEY_ QString("")
//#define CFG_KEY_ QString("")
//#define CFG_KEY_ QString("")
//#define CFG_KEY_ QString("")
//#define CFG_KEY_ QString("")

//----------------------------------------------------------------------------
    
#define TAB_PLAYER QString("Player")
#define PL_FNAME QString("FirstName")
#define PL_LNAME QString("LastName")
#define PL_TEAM_REF QString("TeamRefId")
#define PL_SEX QString("Sex")
//#define PL_ QString("")
//#define PL_ QString("")
//#define PL_ QString("")
//#define PL_ QString("")
//#define PL_ QString("")
//#define PL_ QString("")
//#define PL_ QString("")
//#define PL_ QString("")
//#define PL_ QString("")
//#define PL_ QString("")
//#define PL_ QString("")
//#define PL_ QString("")
//#define PL_ QString("")
//#define PL_ QString("")
//#define PL_ QString("")
//#define PL_ QString("")
//#define PL_ QString("")
//#define PL_ QString("")

//----------------------------------------------------------------------------
    
#define TAB_TEAM QString("Team")
//#define TE_ QString("")

//----------------------------------------------------------------------------
    
#define TAB_COURT QString("Court")
#define CO_NUMBER QString("Number")
#define CO_IS_MANUAL_ASSIGNMENT QString("IsManualAssignment")
//#define CO_ QString("")
//#define CO_ QString("")
//#define CO_ QString("")
//#define CO_ QString("")
//#define CO_ QString("")

//----------------------------------------------------------------------------
    
#define TAB_CATEGORY QString("Category")
#define CAT_MATCH_TYPE QString("MatchType")
#define CAT_SEX QString("Sex")
#define CAT_SYS QString("System")
#define CAT_ACCEPT_DRAW QString("AcceptDraw")
#define CAT_WIN_SCORE QString("WinScore")
#define CAT_DRAW_SCORE QString("DrawScore")
#define CAT_GROUP_CONFIG QString("GroupConfig")
#define CAT_BRACKET_VIS_DATA QString("BracketVisData")
//#define CAT_ QString("")
//#define CAT_ QString("")
//#define CAT_ QString("")
//#define CAT_ QString("")
//#define CAT_ QString("")
//#define CAT_ QString("")
//#define CAT_ QString("")
//#define CAT_ QString("")
//#define CAT_ QString("")
//#define CAT_ QString("")
//#define CAT_ QString("")
  
//----------------------------------------------------------------------------

#define TAB_P2C QString("Player2Category")
#define P2C_PLAYER_REF QString("PlayerRefId")
#define P2C_CAT_REF QString("CategoryRefId")
  
//----------------------------------------------------------------------------

#define TAB_PAIRS QString("PlayerPair")
#define PAIRS_PLAYER1_REF QString("Player1RefId")
#define PAIRS_PLAYER2_REF QString("Player2RefId")
#define PAIRS_CAT_REF QString("CategoryRefId")
#define PAIRS_GRP_NUM QString("GroupNumber")
#define PAIRS_INITIAL_RANK QString("InitialRank")
  
//----------------------------------------------------------------------------

#define TAB_MATCH QString("Match")
#define MA_GRP_REF  QString("MatchGroupRefId")
#define MA_NUM  QString("Number")
#define MA_PAIR1_REF  QString("PlayerPair1RefId")
#define MA_PAIR2_REF  QString("PlayerPair2RefId")
#define MA_ACTUAL_PLAYER1A_REF  QString("ActualPlayer1aRefId")
#define MA_ACTUAL_PLAYER1B_REF  QString("ActualPlayer1bRefId")
#define MA_ACTUAL_PLAYER2A_REF  QString("ActualPlayer2aRefId")
#define MA_ACTUAL_PLAYER2B_REF  QString("ActualPlayer2bRefId")
#define MA_RESULT  QString("Result")
#define MA_COURT_REF  QString("CourtRefId")
#define MA_START_TIME  QString("StartTime")
#define MA_ADDITIONAL_CALL_TIMES  QString("CallTimes")
#define MA_FINISH_TIME  QString("FinishTime")
#define MA_PAIR1_SYMBOLIC_VAL  QString("PlayerPair1SymbolicValue")
#define MA_PAIR2_SYMBOLIC_VAL  QString("PlayerPair2SymbolicValue")
#define MA_WINNER_RANK  QString("WinnerRank")
#define MA_LOSER_RANK  QString("LoserRank")
//#define MA_  QString("")
//#define MA_  QString("")
//#define MA_  QString("")
//#define MA_  QString("")
//#define MA_  QString("")
//#define MA_  QString("")
  
//----------------------------------------------------------------------------

#define TAB_MATCH_GROUP QString("MatchGroup")
#define MG_CAT_REF  QString("CategoryRefId")
#define MG_ROUND  QString("Round")
#define MG_GRP_NUM  QString("RoundRobinGroupNumber")
#define MG_STAGE_SEQ_NUM  QString("StageSequenceNumber")
//#define MG_  QString("")
//#define MG_  QString("")
//#define MG_  QString("")
//#define MG_  QString("")
//#define MG_  QString("")
//#define MG_  QString("")
//#define MG_  QString("")
//#define MG_  QString("")
//#define MG_  QString("")
//#define MG_  QString("")
//#define MG_  QString("")
//#define MG_  QString("")
//#define MG_  QString("")
//#define MG_  QString("")
//#define MG_  QString("")
//#define MG_  QString("")
//#define MG_  QString("")
//#define MG_  QString("")
//#define MG_  QString("")
  
//----------------------------------------------------------------------------

#define TAB_RANKING QString("Ranking")
#define RA_ROUND  QString("Round")
#define RA_PAIR_REF  QString("PlayerPairRef")
#define RA_GAMES_WON  QString("GamesWon")
#define RA_GAMES_LOST  QString("GamesLost")
#define RA_MATCHES_WON  QString("MatchesWon")
#define RA_MATCHES_LOST  QString("MatchesLost")
#define RA_MATCHES_DRAW  QString("MatchesDraw")
#define RA_POINTS_WON  QString("PointsWon")
#define RA_POINTS_LOST  QString("PointsLost")
#define RA_RANK  QString("Rank")
#define RA_CAT_REF  QString("CategoryRef")
#define RA_GRP_NUM  QString("MatchGroupNumber")
//#define RA_  QString("")
//#define RA_  QString("")
//#define RA_  QString("")

//----------------------------------------------------------------------------

#define TAB_BRACKET_VIS QString("BracketVisualization")
#define BV_MATCH_REF QString("MatchRefId")
#define BV_CAT_REF QString("CategoryRefId")
#define BV_PAGE QString("PageNumber")
#define BV_GRID_X0 QString("GridX0")
#define BV_GRID_Y0 QString("GridY0")
#define BV_SPAN_Y QString("SpanY")
#define BV_ORIENTATION QString("Orientation")
#define BV_TERMINATOR QString("Terminator")
#define BV_INITIAL_RANK1 QString("InitialRank1")
#define BV_INITIAL_RANK2 QString("InitialRank2")
#define BV_PAIR1_REF QString("PlayerPair1RefId")
#define BV_PAIR2_REF QString("PlayerPair2RefId")
#define BV_Y_PAGEBREAK_SPAN QString("PagebreakOffsetY")
#define BV_NEXT_PAGE_NUM QString("NextPageNum")
#define BV_TERMINATOR_OFFSET_Y QString("TerminatorOffset")
#define BV_ELEMENT_ID QString("BracketElementId")
#define BV_NEXT_WINNER_MATCH QString("NextWinnerMatch")
#define BV_NEXT_LOSER_MATCH QString("NextLoserMatch")
#define BV_NEXT_MATCH_POS_FOR_WINNER QString("NextMatchPosForWinner")
#define BV_NEXT_MATCH_POS_FOR_LOSER QString("NextMatchPosForLoser")
//#define BV_ QString("")
//#define BV_ QString("")

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

  enum CAT_PARAMETER {
    ALLOW_DRAW,
    WIN_SCORE,
    DRAW_SCORE,
    GROUP_CONFIG
    
  };
  
//----------------------------------------------------------------------------

  enum CAT_ADD_STATE {
    WRONG_SEX,         // Player may not join because the sex doesn't fit
    CAN_JOIN,          // Player can join the category now and participate in matches
    CAT_CLOSED,        // Player WOULD fit, but the category doesn't accept any more players
    ALREADY_MEMBER     // Player is already assigned to this category
  };
  
//----------------------------------------------------------------------------

  enum OBJ_STATE {
    STAT_PL_IDLE,
    STAT_PL_PLAYING,
    STAT_PL_WAIT_FOR_REGISTRATION,  // player has to report to match control before considered "ready"
    STAT_CAT_CONFIG,
    STAT_CAT_FROZEN,  // intermediate state in which the category can be configured for the first round
    STAT_CAT_IDLE,
    STAT_CAT_PLAYING,  // at least one match in this category is currently being played
    STAT_CAT_FINALIZED, // no more rounds or matches to come
    STAT_CAT_WAIT_FOR_INTERMEDIATE_SEEDING,  // we need user-controlled seeding / match generation before we can continue
    STAT_MG_CONFIG,   // Match group has been created, matches can still be added or removed
    STAT_MG_FROZEN,   // No more adding or removing of matches; match group CANNOT be staged because earlier rounds/groups have to be staged / scheduled first
    STAT_MG_IDLE,     // No more adding or removing of matches; match group can be staged and waits for being staged
    STAT_MG_STAGED,     // No more adding or removing of matches; match group is selected to be scheduled and waits for being scheduled
    STAT_MG_SCHEDULED, // Match numbers have been assigned
    STAT_MG_FINISHED,  // All matches in this group are finished
    STAT_MA_INCOMPLETE,   // Match is not yet fully defined (e.g., player names or match number are missing)
    STAT_MA_FUZZY,        // Player names are defined by symbolic values (e.g., winner of match XYZ); match number is assigned; match cannot be called
    STAT_MA_WAITING,      // Player names and match number are assigned but match cannot be called because earlier rounds have to be played first
    STAT_MA_READY,        // Opponents and match number are fully defined and all players are idle; match can be called
    STAT_MA_BUSY,         // Opponents and match number are fully defined but some players are busy; match cannot be called
    STAT_MA_RUNNING,      // The match is currently being played
    STAT_MA_FINISHED,     // The match is finished and the result has been entered
    STAT_MA_POSTPONED,    // The match is postponed and cannot be called
    STAT_CO_AVAIL,        // The court is empty and can be assigned to a match
    STAT_CO_BUSY,         // The court is being used by a match
    STAT_CO_DISABLED,     // The court cannot be used (temporarily)
  };
  
//----------------------------------------------------------------------------
  
  enum MATCH_TYPE {
    SINGLES,
    DOUBLES,
    MIXED
  };  
  
//----------------------------------------------------------------------------
  enum SEX
  {
    M,
    F,
    DONT_CARE
  };  
  
//----------------------------------------------------------------------------
    
  enum MATCH_SYSTEM {
    SWISS_LADDER,
    GROUPS_WITH_KO,
    RANDOMIZE,
    RANKING,
    SINGLE_ELIM,
    ROUND_ROBIN,
  };
  
//----------------------------------------------------------------------------

  class TournamentSettings
  {
  public:
    QString tournamentName;
    QString organizingClub;
    bool useTeams;
  } ;

//----------------------------------------------------------------------------

  enum KO_START {
    FINAL,
    SEMI,
    QUARTER,
    L16
  };  

//----------------------------------------------------------------------------

// Special group numbers for MatchGroup objects
// Positive numbers indicate the number of the round robin group within a category,
// Negative numbers have a special meaning for MatchGroups of non-round-robin matches
#define GROUP_NUM__FINAL -1
#define GROUP_NUM__SEMIFINAL -2
#define GROUP_NUM__QUARTERFINAL -4
#define GROUP_NUM__L16 -8
#define GROUP_NUM__ITERATION -100  // just a normal round in Swiss Ladder, Random Matches, ...
  
// A special group number indicating that the match is
// not in any group at all
#define GRP_NUM__NOT_ASSIGNED -1

// A special value that matches any players group
// Is only used for return values of functions
#define ANY_PLAYERS_GROUP_NUMBER -200
    
//----------------------------------------------------------------------------

// A special match number, indicating that a match number
// has not yet been assigned
#define MATCH_NUM_NOT_ASSIGNED -1

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
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    
}

#endif	/* TOURNAMENTDATADEFS_H */

