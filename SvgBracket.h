#ifndef SVGBRACKET_H
#define SVGBRACKET_H

#include <string>
#include <vector>
#include <variant>
#include <string_view>
#include <optional>

#include <Sloppy/DateTime/DateAndTime.h>

#include "TournamentDataDefs.h"
#include "PlayerPair.h"

namespace QTournament
{
  /** \brief Basic data about a SVG page
   */
  struct SvgPageDescr
  {
    double width_mm{0};   ///< the page width in mm
    double height_mm{0};   ///< the page height in mm
    std::string resName;   ///< the name of the resource containing the SVG data
    std::string content;   ///< the content of the SVG file; mostly empty and only used to transport readily substituted SVG content
  };
}

namespace QTournament::SvgBracket
{
  /** \brief Information about the position, length and name of
   * a tag in a SVG template
   */
  struct TagData
  {
    int idxStart{-1};  ///< index of the first opening bracket
    int idxEnd{-1};   ///< index of the last closing bracket
    int len{-1};   ///< length of the tag including brackets
    std::string name;   ///< tag name without brackets
  };

  //----------------------------------------------------------------------------

  /** \brief The type of a tag in the SVG file
   */
  enum class TagType
  {
    Match,   ///< the tag identifies a match
    Player,   ///< the tag identifies a player label
    Rank,   ///< the tag identifies a rank label
    Other   ///< the tag represents supporting label like date, category name, tournament name, ...
  };

  /** \brief Identifies the position of a two-row label
   */
  enum class LabelPos
  {
    First,   ///< the first row (single player / first partner in a match or first player for a rank)
    Second   ///< the second row (second partner in a match or for a rank)
  };

  //----------------------------------------------------------------------------

  /** \brief The data contained in a match tag
   */
  struct MatchTag
  {
    BracketMatchNumber bracketMatchNum{-1};   ///< the number of the match within the bracket (1-based)
    Round roundNum{-1};    ///< the round within the bracket when this match is played (1-based)
    Rank winnerRank{-1};   ///< the rank for the match winner (-1 = no rank)
    Rank loserRank{-1};   ///< the rank for the match loser (-1 = no rank)
  };

  //----------------------------------------------------------------------------

  /** \brief The data contained in a player tag
   */
  struct PlayerTag
  {
    BracketMatchNumber bracketMatchNum{-1};   ///< the number of the match within the bracket (1-based)
    int playerPos{-1};   ///< "1" or "2" for player 1 / player 2
    LabelPos pos{LabelPos::First};
    BracketMatchNumber srcMatch{0};   ///< "-xxx": this is the loser of match xxx, "xxx": this is the winner of match xxx; only set for the primary label
    Rank initialRank{-1};   ///< initial rank for the seeding, if any
  };

  //----------------------------------------------------------------------------

  /** \brief The data contained in a rank tag
   */
  struct RankTag
  {
    Rank rank{-1};
    LabelPos pos{LabelPos::First};   ///< "1" for the primary label position, "2" for the secondary label
  };

  //----------------------------------------------------------------------------

  /** \brief A completely parsed tag
   */
  struct ParsedTag
  {
    TagType type{TagType::Other};   ///< the type of the parsed tag
    TagData src;   ///< the tag's original position in the file

    std::variant<MatchTag, PlayerTag, RankTag, std::string> content;   ///< the type dependent tag content, parsed
    //MatchTag matchTagData;
    //PlayerTag playerTagData;
    //RankTag rankTagData;
  };

  //----------------------------------------------------------------------------

  /** \brief Reads binary content from a resource and searches it for tags
   *
   * \throws std::invalid_argument if a resource with the provided name could not be found
   */
  std::vector<TagData> findRawTagsInResource(
      const std::string& resName,   ///< the SVG data to parse
      const std::string& openingBracket = "{{",   ///< the opening tag character
      const std::string& closingBracket = "}}"   ///< the closing tag character
      );

  //----------------------------------------------------------------------------

  /** \brief Finds all tags in the raw SVG data
   */
  std::vector<TagData> findRawTags(
      const std::string_view& svgData,   ///< the SVG data to parse
      const std::string& openingBracket = "{{",   ///< the opening tag character
      const std::string& closingBracket = "}}"   ///< the closing tag character
      );

  //----------------------------------------------------------------------------

  /** \brief Determines the tag type based on the whole tag name
   */
  TagType determineTagTypeFromName(
      const std::string& tagName   ///< the full tag name (everything between the tag's brackets)
      );

  //----------------------------------------------------------------------------

  /** \brief Parses a match tag
   */
  MatchTag parseMatchTag(
      const std::string& tagName   ///< the full tag name (everything between the tag's brackets)
      );

  //----------------------------------------------------------------------------

  /** \brief Parses a player tag
   */
  PlayerTag parsePlayerTag (
      const std::string& tagName   ///< the full tag name (everything between the tag's brackets)
      );
  //----------------------------------------------------------------------------

  /** \brief Parses a rank tag
   */
  RankTag parseRankTag(
      const std::string& tagName   ///< the full tag name (everything between the tag's brackets)
      );

  //----------------------------------------------------------------------------

  std::vector<ParsedTag> parseContent(
      const std::string_view& svgData,   ///< the SVG data to parse
      const std::string& openingBracket = "{{",   ///< the opening tag character
      const std::string& closingBracket = "}}"   ///< the closing tag character
      );

  /** \brief Takes a list of raw tags (e.g., as returned by findRawTags) and
   * converts it into a list of parsed tags.
   *
   * \returns the list of parsed tags
   */
  std::vector<ParsedTag> parseRawTagList(
      const std::vector<TagData>& tagList   ///< the list of raw tags
      );

  //----------------------------------------------------------------------------

  /** \brief A single page of an SVG bracket
   */
  struct SvgBracketPage : public SvgPageDescr
  {
    double maxNameLen_mm;   ///< the maximum length of a page that fits on a line of the bracket
    std::vector<TagData> rawTags;   ///< a list of all tags on that page
  };

  //----------------------------------------------------------------------------

  /** \brief A complete, multi-page SVG bracket definition
   */
  struct SvgBracketDef
  {
    SvgBracketMatchSys sys;    ///< the match system this bracket can be used for
    int maxNumPlayers;   ///< the maximum number of players for this bracket
    std::vector<SvgBracketPage> pages;   ///< all pages that belong to this bracket
    std::vector<int> roundTypes;   ///< for each round whether it is a normal iteration, a quarter final, semi final, ...
  };

  //----------------------------------------------------------------------------

  /** \brief Extracts the players from a list of raw tags
   *
   * \return a player tag list, sorted
   * by match number in ascending order.
   */
  std::vector<PlayerTag> sortedPlayersFromTagList(
      const std::vector<TagData>& tagList   ///< the list of parsed SVG tags from that we'll extract the players
      );

  //----------------------------------------------------------------------------

  /** \brief Extracts the matches from a list of raw tags
   *
   * \return a match tag list, sorted
   * by match number in ascending order.
   */
  std::vector<MatchTag> sortedMatchesFromTagList(
      const std::vector<TagData>& tagList   ///< the list of parsed SVG tags from that we'll extract the players
      );

  //----------------------------------------------------------------------------

  /** \brief The role of a player pair (in particular in a bracket)
   */
  enum class PairRole
  {
    AsWinner,
    AsLoser
  };

  /** \brief Information on where a player came from in a bracket
   */
  struct IncomingBracketLink
  {
    BracketMatchNumber srcMatch;
    PairRole role;
  };

  /** \brief Information about an outgoing link for a player
   * pair in a bracket
   */
  struct OutgoingBracketLink
  {
    BracketMatchNumber dstMatch;  ///< the bracket match number of the next match
    int pos;   ///< "1": the player becomes player 1 of the next match, "2": guess...
  };

  /** \brief Internal, intermediate struct that captures all important aspects
   * of a bracket match
   */
  class BracketMatchData
  {
  public:
    enum class BranchState
    {
      Dead,
      Alive,
      Assigned
    };

    /** \brief Default ctor for an (invalid) bracket match data struct */
    BracketMatchData() = default;

    /** \brief Standard ctor for a bracket match with initial ranks; the round
     * number is implicitly set to "1".
     */
    BracketMatchData(
        const BracketMatchNumber& n,   ///< the bracket-internal, 1-based number of this match
        const Rank& p1InitialRank,   ///< the initial rank of player pair 1
        const Rank& p2InitialRank,   ///< the initial rank of player pair 2
        const std::variant<OutgoingBracketLink, Rank>& dstWinner,   ///< what happens to the winner
        const std::variant<OutgoingBracketLink, Rank>& dstLoser   ///< what happens to the loser
        );

    /** \brief Standard ctor for a bracket match in a round >= 2
     */
    BracketMatchData(
        const BracketMatchNumber& n,   ///< the bracket-internal, 1-based number of this match
        const Round& r,   ///< the bracket-internal, 1-based round number for this match
        const IncomingBracketLink& inLink1,   ///< where player 1 comes from
        const IncomingBracketLink& inLink2,   ///< where player 2 comes from
        const std::variant<OutgoingBracketLink, Rank>& dstWinner,   ///< what happens to the winner
        const std::variant<OutgoingBracketLink, Rank>& dstLoser   ///< what happens to the loser
        );

    /** \brief Default copy ctor */
    BracketMatchData(const BracketMatchData& other) = default;

    /** \brief Default move ctor */
    BracketMatchData(BracketMatchData&& other) = default;

    /** \brief Default copy assignment */
    BracketMatchData& operator=(const BracketMatchData& other) = default;

    /** \brief Default copy assignment */
    BracketMatchData& operator=(BracketMatchData&& other) = default;

    BracketMatchNumber matchNum() const { return brMaNum; }
    Round round() const { return _round; }
    PlayerPairRefId assignedPair1() const { return p1Pair; }
    PlayerPairRefId assignedPair2() const { return p2Pair ;}
    BranchState pair1State() const { return p1State; }
    BranchState pair2State() const { return p2State; }

    const OutgoingBracketLink& nextWinnerMatch() const { return std::get<OutgoingBracketLink>(winnerAction); }
    const OutgoingBracketLink& nextLoserMatch() const { return std::get<OutgoingBracketLink>(loserAction); }
    const IncomingBracketLink& inLink1() const { return std::get<IncomingBracketLink>(src1); }
    const IncomingBracketLink& inLink2() const { return std::get<IncomingBracketLink>(src2); }
    std::optional<Rank> winnerRank() const { return (std::holds_alternative<Rank>(winnerAction) ? std::get<Rank>(winnerAction) : std::optional<Rank>{}); }
    std::optional<Rank> loserRank() const { return (std::holds_alternative<Rank>(loserAction) ? std::get<Rank>(loserAction) : std::optional<Rank>{}); }


    /** \returns an empty optional if the requested player doesn't have
     * an initial rank
     */
    std::optional<Rank> initialRank(
        int pos
        ) const;

    /** \brief Assigns a player pair */
    void assignPlayerPair(
        const PlayerPairRefId& ppId,   ///< the pair ID to assign
        int pos   ///< assign to player "1" or "2"
        );

    /** \brief Declares a player pair as unused
     */
    void setPairUnused(
        int pos   ///< pair "1" or "2"
        );

    /** \returns `true` if both players have no pair assigned
     */
    constexpr bool noPairsAssigned() const { return ((p1Pair.get() == -1) && (p2Pair.get() == -1)); }

    /** \returns `true` if there is a follow-up match for the winner
     */
    constexpr bool hasWinnerMatch() const { return std::holds_alternative<OutgoingBracketLink>(winnerAction); }

    /** \returns `true` if there is a follow-up match for the loser
     */
    constexpr bool hasLoserMatch() const { return std::holds_alternative<OutgoingBracketLink>(loserAction); }

    /** \return `true` if a match can be skipped; that's the case if at least one of the branches is dead.
     */
    constexpr bool canSkip() const { return ((p1State == BranchState::Dead) || (p2State == BranchState::Dead)); }

  protected:

  private:
    BracketMatchNumber brMaNum{-1};   ///< the number of the associated match in the bracket
    Round _round{-1};   ///< the 1-based, bracket-internal round this match is played in

    // static structure information
    std::variant<OutgoingBracketLink, Rank> winnerAction{Rank{-1}};   ///< what happens to the winner
    std::variant<OutgoingBracketLink, Rank> loserAction{Rank{-1}};   ///< what happens to the loser
    std::variant<IncomingBracketLink, Rank> src1{Rank{-1}};   ///< where player 1 comes from
    std::variant<IncomingBracketLink, Rank> src2{Rank{-1}};   ///< where player 2 comes from

    // dynamic allocation information
    BranchState p1State{BranchState::Alive};
    BranchState p2State{BranchState::Alive};
    PlayerPairRefId p1Pair{-1};
    PlayerPairRefId p2Pair{-1};
  };

  /** \brief A sorted list of bracket matches with additional manipulation functions
   * that ensure that the match data (especially the linking) is kept consistent
   */
  class BracketMatchDataList : public std::vector<BracketMatchData>
  {
  public:
    // inherit all parent ctors
    using std::vector<BracketMatchData>::vector;

    /** \brief Takes a sorted list of player pairs (seed) and
     *   * assignes PlayerPair IDs to the first matches as far as possible; and
     *   * tags unused matches
     *
     * \pre The seeding list is sorted with the strongest player at index zero.
     *
     * \pre The seeding list contains at least two player pairs and not more than twice the
     * number of matches in the first round.
     *
     * \pre The list of bracket matches stems from a SvgDef that has passed all checks in SvgBracket::consistencyCheck()
     *
     */
    void applySeeding(
        const QTournament::PlayerPairList& seed   ///< the sorted list of initial player pairs
        );

    /** \brief Follows the bracket in forward direction (==> towards higher rounds)
     * until it finds the next playable match (==> both branches non-dead) starting
     * from a given match as a winner or loser.
     *
     * \returns a BracketMatchNumber if such a match could be found or the resulting final rank
     */
    std::variant<OutgoingBracketLink, Rank> traverseForward(
        const BracketMatchData& ma,   ///< the starting point of the traversal
        PairRole role   ///< start the traversal as a winner or loser
        ) const;

  protected:
    /** \brief Takes the player in the given position and promotes it as to the winner
     * match and declares the loser match as "dead".
     *
     * \pre The given position has an assigned pair.
     *
     * \pre The other position is dead.
     */
    void fastForward(
        BracketMatchData& ma,   ///< the match from which to start
        int pos   ///< the position (1 or 2) that contains the to-be-forwarded pair
        );

    /** \brief Sets the loser branch of a given match to "dead"
     *
     * \pre The given match actually has an outgoing link for the loser
     */
    void declareLoserBranchDead(
        const BracketMatchData& ma   ///< the match of which the loser's branch will be declared "dead"
        );

  };

  //----------------------------------------------------------------------------

  /** \brief Checks whether the bracket data is consistent
   *
   * \returns a string with an error message for the first encountered error; empty string if everything is fine
   */
  std::optional<std::string> consistencyCheck(
      const std::vector<ParsedTag> allTags   ///< list of tags to be checked
      );

  //----------------------------------------------------------------------------

  /** \brief Consistency rule 1: there must be at least one match
   */
  std::optional<std::string> checkRule01(const std::vector<ParsedTag>& allMatches, const std::vector<ParsedTag>& allPlayers, const std::vector<ParsedTag>& allRanks);

  //----------------------------------------------------------------------------

  /** \brief Consistency rule 2: all matches must have unique, consecutive numbers starting with "1"
   * and the round number must be equal or higher than the round number of the previous match
   */
  std::optional<std::string> checkRule02(const std::vector<ParsedTag>& allMatches, const std::vector<ParsedTag>& allPlayers, const std::vector<ParsedTag>& allRanks);

  //----------------------------------------------------------------------------

  /** \brief Consistency rule 3: each match must have four player labels
   */
  std::optional<std::string> checkRule03(const std::vector<ParsedTag>& allMatches, const std::vector<ParsedTag>& allPlayers, const std::vector<ParsedTag>& allRanks);

  //----------------------------------------------------------------------------

  /** \brief Each match in the first round must have an initial rank; the
   * sum of two initial ranks in a match must always be "numberOfPlayers + 1"
   */
  std::optional<std::string> checkRule04(const std::vector<ParsedTag>& allMatches, const std::vector<ParsedTag>& allPlayers, const std::vector<ParsedTag>& allRanks);

  //----------------------------------------------------------------------------

  /** \brief Rerefential integrity: each match may occur only zero or once as the "winner from X" or "loser from X" in a later round;
   * each match without an initial rank must have references to the previous match
   */
  std::optional<std::string> checkRule05(const std::vector<ParsedTag>& allMatches, const std::vector<ParsedTag>& allPlayers, const std::vector<ParsedTag>& allRanks);

  /** \brief Rerefential integrity2: each winner or loser of a match must have either
   * a final rank, drop out of the tournament or has to be used used in a subsequent match;
   * all winner/loser ranks may only be assigned once
   */
  std::optional<std::string> checkRule06(const std::vector<ParsedTag>& allMatches, const std::vector<ParsedTag>& allPlayers, const std::vector<ParsedTag>& allRanks);

  //std::optional<std::string> checkRule01(const std::vector<ParsedTag>& allMatches, const std::vector<ParsedTag>& allPlayers, const std::vector<ParsedTag>& allRanks);
  //std::optional<std::string> checkRule01(const std::vector<ParsedTag>& allMatches, const std::vector<ParsedTag>& allPlayers, const std::vector<ParsedTag>& allRanks);
  //std::optional<std::string> checkRule01(const std::vector<ParsedTag>& allMatches, const std::vector<ParsedTag>& allPlayers, const std::vector<ParsedTag>& allRanks);
  //std::optional<std::string> checkRule01(const std::vector<ParsedTag>& allMatches, const std::vector<ParsedTag>& allPlayers, const std::vector<ParsedTag>& allRanks);
  //std::optional<std::string> checkRule01(const std::vector<ParsedTag>& allMatches, const std::vector<ParsedTag>& allPlayers, const std::vector<ParsedTag>& allRanks);
  //std::optional<std::string> checkRule01(const std::vector<ParsedTag>& allMatches, const std::vector<ParsedTag>& allPlayers, const std::vector<ParsedTag>& allRanks);
  //std::optional<std::string> checkRule01(const std::vector<ParsedTag>& allMatches, const std::vector<ParsedTag>& allPlayers, const std::vector<ParsedTag>& allRanks);
  //std::optional<std::string> checkRule01(const std::vector<ParsedTag>& allMatches, const std::vector<ParsedTag>& allPlayers, const std::vector<ParsedTag>& allRanks);
  //std::optional<std::string> checkRule01(const std::vector<ParsedTag>& allMatches, const std::vector<ParsedTag>& allPlayers, const std::vector<ParsedTag>& allRanks);
  //std::optional<std::string> checkRule01(const std::vector<ParsedTag>& allMatches, const std::vector<ParsedTag>& allPlayers, const std::vector<ParsedTag>& allRanks);

  //----------------------------------------------------------------------------

  /** \brief Searches for a SVG bracket with a given match system and for a given number
   * of players.
   *
   * \returns the full bracket definition if the search was successful; empty otherwise
   */
  std::optional<SvgBracketDef> findSvgBracket(
      SvgBracketMatchSys msys,   ///< the match system for which we need a bracket
      int nPlayers   ///< the number of players currently in the category
      );

  //----------------------------------------------------------------------------

  /** \brief Converts the tag-based SvgBracketDef into the more
   * application-level based representation BracketMatchData
   *
   * \return all bracket matches, sorted by match number (and thus by round)
   */
  BracketMatchDataList convertToBracketMatches(const SvgBracketDef& def);

  //----------------------------------------------------------------------------

  struct CommonBracketTags
  {
    std::string tnmtName;   ///< the tournament name
    std::string club;   ///< the organizing club
    std::string catName;   ///< the category name
    std::string subtitle;   ///< e.g., "After round 1" or "Initial matches"
    std::optional<Sloppy::DateTime::LocalTimestamp> time;   ///< timestamp; use current time if empty
    std::optional<int> date;   ///< integer date, e.g. 20190916; use current date if empty
  };

  //----------------------------------------------------------------------------

  /** \brief Converts a player pair into a two-line representation for the bracket
   *
   * \returns the "a" and "b" strings of a player pair for a bracket label
   */
  std::pair<std::string, std::string> pair2bracketLabel(const PlayerPair& pp);

  //----------------------------------------------------------------------------

  /** \brief Takes an SvgBracketMatchSys along with a given seeding list and returns
   * the ready-to-print SVG sheets for bracket with all tags substituted.
   *
   * All tags without substitution data will be erased (= replaced by an empty string).
   *
   * \returns a vector of SVG pages (empty on error)
   */
  std::vector<SvgPageDescr> substSvgBracketTags(
      SvgBracketMatchSys msys,   ///< the match system for which we need a bracket
      const QTournament::PlayerPairList& seed,   ///< the sorted list of initial player pairs
      const CommonBracketTags& cbt,   ///< common metadata used on all bracket sheets
      bool includeMatchNumbers    ///< if `true` the match numbers for the first round will be included as well (if available)
      );

  //----------------------------------------------------------------------------

  /** \returns the four player tags (1a, 1b, 2a, 2b) for a given bracket match number
   *
   * \throws std::invalid_argument if the match number was invalid
   */
  std::tuple<ParsedTag, ParsedTag, ParsedTag, ParsedTag> findPlayerTagsforMatchNum(
      const std::vector<ParsedTag>& tagList,
      BracketMatchNumber brNum
      );

  //----------------------------------------------------------------------------

  /** \brief Applies a substitution dictionary to bracket definition and
   * returns the ready-to-print SVG sheets
   */
  std::vector<SvgPageDescr> applySvgSubstitution(
      const std::vector<SvgBracketPage>& pages,   ///< the input bracket
      const std::unordered_map<std::string, std::string>& dict   ///< dictionary with the substitution strings; tag names are the keys
      );

  //----------------------------------------------------------------------------

  /** \brief Add the CommonBracketTags (date, time, title, ...) to an already
   * existing dictionary with substitution strings.
   *
   * The dictionary is modified in place.
   */
  void addCommonTagsToSubstDict(
      std::unordered_map<std::string, std::string>& dict,   ///< dictionary with the substitution strings; tag names are the keys
      const CommonBracketTags& cbt   ///< the common meta tags to add
      );

  //----------------------------------------------------------------------------


}

#endif // SVGBRACKET_H
