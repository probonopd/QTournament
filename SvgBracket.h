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
#include "Match.h"

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
  // forwards
  class BracketMatchData;

  //----------------------------------------------------------------------------
  //-------------------------- Data Types: Tags --------------------------------
  //----------------------------------------------------------------------------

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

  //----------------------------------------------------------------------------

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
  //------------------------ Data Types: SVG Pages -----------------------------
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
  //---------------------- Data Types: Visualization ---------------------------
  //----------------------------------------------------------------------------

  /** \brief A struct that defines how a match should be represented in
   * a bracket visualization
   */
  struct MatchDispInfo
  {
    enum class PairRepresentation
    {
      None,   ///< do not print any pair information
      RealNamesOnly,   ///< only print real names, no symbolic names
      RealOrSymbolic  ///< real or symbolic names
    };

    enum class ResultFieldContent
    {
      None,   ///< leave the result field empty
      MatchNumberOnly,   ///< print the assigned match number, if any
      ResultOnly,   ///< print the match result only, if any
      ResultOrNumber   ///< print the result, if any; if no result, print match number, if any; if no number, do nothing
    };

    Match ma;   ///< the affected match
    PairRepresentation pairRep;   ///< how to display the player pairs
    ResultFieldContent resultRep;   ///< what to print in the bracket center
  };

  //----------------------------------------------------------------------------
  //------------------------------ Classes -------------------------------------
  //----------------------------------------------------------------------------

  //----------------------------------------------------------------------------
  //--------------------------- Free Functions ---------------------------------
  //----------------------------------------------------------------------------

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

  /** \brief Takes an SvgBracketMatchSys along with a given seeding list and
   * match list and returns the ready-to-print SVG sheets for bracket with all tags substituted.
   *
   * All tags without substitution data will be erased (= replaced by an empty string).
   *
   * \returns a vector of SVG pages (empty on error)
   */
  std::vector<SvgPageDescr> substSvgBracketTags(
      const TournamentDB& db,
      SvgBracketMatchSys msys,   ///< the match system for which we need a bracket
      const QTournament::PlayerPairList& seed,   ///< the sorted list of initial player pairs
      const std::vector<MatchDispInfo>& maList,  ///< a list of matches that should be filled into the bracket
      const CommonBracketTags& cbt   ///< common metadata used on all bracket sheets
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

  /** \brief A struct that contains a tag names for a single bracket element
   */
  struct BracketElementTagNames
  {
    std::string matchTagName;
    std::string p1aTagName;
    std::string p1bTagName;
    std::string p2aTagName;
    std::string p2bTagName;
    std::string winnerRankTagName;
    std::string loserRankTagName;
  };

  /** \brief Determines the substituion strings for a single bracket element and
   * adds them to a dictionary
   */
  void defSubstStringsForBracketElement(const TournamentDB& db, std::unordered_map<std::string, std::string>& dict,   ///< dictionary for the substitution strings
      const BracketElementTagNames& betl,   ///< tag names (keys) for this bracket for the dictionary
      const BracketMatchData& bmd,   ///< the BracketMatchData that represents the bracket element
      const std::optional<MatchDispInfo>& mdi   ///< how match and player pairs should be represented
      );

}

#endif // SVGBRACKET_H
