/*
 *    This is QTournament, a badminton tournament management program.
 *    Copyright (C) 2014 - 2019  Volker Knollmann
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

#include <unordered_map>

#include <QString>

#include <Sloppy/NamedType.h>

namespace QTournament
{
  constexpr int DbVersionMajor = 3;
  constexpr int DbVersionMinor = 0;
  constexpr int MinRequiredDbVersion = 3;

//----------------------------------------------------------------------------

#define GenericNameFieldName "Name"
  constexpr int MaxNameLen = 50;
  
#define GenericStateFieldName "ObjState"
  
#define GenericSeqnumFieldName "SequenceNumber"

//----------------------------------------------------------------------------

  constexpr int MaxGroupSize = 50;
  constexpr int MaxGroupCount = 50;
  
//----------------------------------------------------------------------------

  constexpr int MaxRandomizationRounds = 100;    // for group assignments, player pairs, etc.

//----------------------------------------------------------------------------

  // Named types as aliases from simple types like int, etc.
  //
  // Not yet used consistently throughout the app
  using PlayerPairRefId = Sloppy::NamedType<int, struct PlayerPairRefIdTag>;   ///< reference ID of a player pair
  using BracketMatchNumber = Sloppy::NamedType<int, struct BracketMatchNumberTag>;   ///< the 1-based, bracket-internal number of a match
  using Rank  = Sloppy::NamedType<int, struct RankTag>;   ///< the 1-based rank of a player pair in a ranking / in an initial seeding
  using Round  = Sloppy::NamedType<int, struct RoundTag>;   ///< the 1-based round a match is played in

  // for using NamedTypes as keys in unordered maps
  template<typename NT>
  struct NamedTypeHasher
  {
    std::size_t operator()(const NT& x) const noexcept
    {
      return std::hash<typename NT::UnderlyingType>{}(x.get());
    }
  };

  template<typename NT_Key, typename V>
  using unordered_map_NT = std::unordered_map<NT_Key, V, NamedTypeHasher<NT_Key>>;

//----------------------------------------------------------------------------
    
    
#define DbNull QVariant::Int

//----------------------------------------------------------------------------
    
#define TabCfg "Config"
#define CfgKey_DbVersion "DatabaseVersion"
#define CfgKey_TnmtName "TournamentName"
#define CfgKey_TnmtOrga "OrganizingClub"
#define CfgKey_UseTeams "UseTeams"
#define CfgKey_ExtPlayerDb "ExternalPlayerDatabase"
#define CfgKey_DefaultRefereemode "DefaultRefereeMode"
#define CfgKey_RefereeTeamId "RefereeTeamId"
#define CfgKey_Keystore "Keystore"
#define CfgKey_RegistrationTimestamp "RegistrationTimestamp"
//#define CFG_KEY_ ""

//----------------------------------------------------------------------------
    
#define TabPlayer "Player"
#define PL_Fname "FirstName"
#define PL_Lname "LastName"
#define PL_TeamRef "TeamRefId"
#define PL_Sex "Sex"
#define PL_RefereeCount "RefereeCount"
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
    
#define TabTeam "Team"
//#define TE_ ""

//----------------------------------------------------------------------------
    
#define TabCourt "Court"
#define CO_Number "Number"
#define CO_IsManualAssignment "IsManualAssignment"
//#define CO_ ""
//#define CO_ ""
//#define CO_ ""
//#define CO_ ""
//#define CO_ ""

//----------------------------------------------------------------------------
    
#define TabCategory "Category"
#define CAT_MatchType "MatchType"
#define CAT_Sex "Sex"
#define CAT_Sys "System"
#define CAT_AcceptDraw "AcceptDraw"
#define CAT_WinScore "WinScore"
#define CAT_DrawScore "DrawScore"
#define CAT_GroupConfig "GroupConfig"
#define CAT_BracketMatchSys "BracketMatchSys"
#define CAT_RoundRobinIterations "RoundRobinIterations"
#define CAT_RoundOffset "RoundOffset"
//#define CAT_ ""
  
//----------------------------------------------------------------------------

#define TabP2C "Player2Category"
#define P2C_PlayerRef "PlayerRefId"
#define P2C_CatRef "CategoryRefId"
  
//----------------------------------------------------------------------------

#define TabPairs "PlayerPair"
#define Pairs_Player1Ref "Player1RefId"
#define Pairs_Player2Ref "Player2RefId"
#define Pairs_CatRef "CategoryRefId"
#define Pairs_GrpNum "GroupNumber"
#define Pairs_InitialRank "InitialRank"
  
//----------------------------------------------------------------------------

#define TabMatch "Match"
#define MA_GrpRef  "MatchGroupRefId"
#define MA_Num  "Number"
#define MA_Pair1Ref  "PlayerPair1RefId"
#define MA_Pair2Ref  "PlayerPair2RefId"
#define MA_ActualPlayer1aRef  "ActualPlayer1aRefId"
#define MA_ActualPlayer1bRef  "ActualPlayer1bRefId"
#define MA_ActualPlayer2aRef  "ActualPlayer2aRefId"
#define MA_ActualPlayer2bRef  "ActualPlayer2bRefId"
#define MA_Result  "Result"
#define MA_CourtRef  "CourtRefId"
#define MA_StartTime  "StartTime"
#define MA_AdditionalCallTimes  "CallTimes"
#define MA_FinishTime  "FinishTime"
#define MA_Pair1SymbolicVal  "PlayerPair1SymbolicValue"
#define MA_Pair2SymbolicVal  "PlayerPair2SymbolicValue"
#define MA_WinnerRank  "WinnerRank"
#define MA_LoserRank  "LoserRank"
#define MA_RefereeMode  "RefereeMode"
#define MA_RefereeRef  "RefereeRefId"
#define MA_BracketMatchNum  "BracketMatchNum"
//#define MA_  ""
//#define MA_  ""
//#define MA_  ""
  
//----------------------------------------------------------------------------

#define TabMatchGroup "MatchGroup"
#define MG_CatRef  "CategoryRefId"
#define MG_Round  "Round"
#define MG_GrpNum  "RoundRobinGroupNumber"
#define MG_StageSeqNum  "StageSequenceNumber"
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

#define TabMatchSystem "Ranking"
#define RA_Round  "Round"
#define RA_PairRef  "PlayerPairRef"
#define RA_GamesWon  "GamesWon"
#define RA_GamesLost  "GamesLost"
#define RA_MatchesWon  "MatchesWon"
#define RA_MatchesLost  "MatchesLost"
#define RA_MatchesDraw  "MatchesDraw"
#define RA_PointsWon  "PointsWon"
#define RA_PointsLost  "PointsLost"
#define RA_Rank  "Rank"
#define RA_CatRef  "CategoryRef"
#define RA_GrpNum  "MatchGroupNumber"
//#define RA_  ""
//#define RA_  ""
//#define RA_  ""

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

  enum class CatParameter {
    AllowDraw,
    WinScore,
    DrawScore,
    GroupConfig,
    RoundRobinIterations,
    BracketMatchSystem,
    FirstRoundOffset
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
  enum class Sex
  {
    M,
    F,
    DontCare
  };  
  
//----------------------------------------------------------------------------
    
  enum class MatchSystem {
    SwissLadder,
    GroupsWithKO,
    Randomize,
    RoundRobin,
    Bracket
  };

  //----------------------------------------------------------------------------

  /** \brief A list of all match systems that are available as SVG bracket
   */
  enum SvgBracketMatchSys
  {
    SingleElim,   ///< single elimination bracket (KO rounds)
    DoubleElim,   ///< double elimination
    RankSys,   ///< used in official ranking tournaments
    SemiWithRanks,   ///< semifinals plus "one-one-one" matches for additional ranks
    FinalsWithRanks,   ///< "one-on-one" matches for all ranks (directly; just one round)
    FinalAnd3rd,   ///< only a final and the match for 3rd place
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

  // A special initial rank indicating that a player pair
  // does not (yet) have an initial rank assigned
  static constexpr int InitialRankNotAssigned = -1;

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

