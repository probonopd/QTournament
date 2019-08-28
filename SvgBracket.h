#ifndef SVGBRACKET_H
#define SVGBRACKET_H

#include <string>
#include <vector>
#include <variant>
#include <string_view>
#include <optional>

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
    int bracketMatchNum{-1};   ///< the number of the match within the bracket (1-based)
    int roundNum{-1};    ///< the round within the bracket when this match is played (1-based)
    int winnerRank{-1};   ///< the rank for the match winner (-1 = no rank)
    int loserRank{-1};   ///< the rank for the match loser (-1 = no rank)
  };

  //----------------------------------------------------------------------------

  /** \brief The data contained in a player tag
   */
  struct PlayerTag
  {
    int bracketMatchNum{-1};   ///< the number of the match within the bracket (1-based)
    int playerPos{-1};   ///< "1" or "2" for player 1 / player 2
    LabelPos pos{LabelPos::First};
    int srcMatch{0};   ///< "-xxx": this is the loser of match xxx, "xxx": this is the winner of match xxx; only set for the primary label
    int initialRank{-1};   ///< initial rank for the seeding, if any
  };

  //----------------------------------------------------------------------------

  /** \brief The data contained in a rank tag
   */
  struct RankTag
  {
    int rank{-1};
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

  //----------------------------------------------------------------------------

  /** \brief Checks whether the bracket data is consistent
   *
   * \returns a string with an error message for the first encountered error; empty string if everything is fine
   */
  std::optional<std::string> consistencyCheck(
      const std::vector<ParsedTag> allTags   ///< list of tags to be checked
      );

  /** \brief Consistency rule 1: there must be at least one match
   */
  std::optional<std::string> checkRule01(const std::vector<ParsedTag>& allMatches, const std::vector<ParsedTag>& allPlayers, const std::vector<ParsedTag>& allRanks);

  /** \brief Consistency rule 2: all matches must have unique, consecutive numbers starting with "1"
   * and the round number must be equal or higher than the round number of the previous match
   */
  std::optional<std::string> checkRule02(const std::vector<ParsedTag>& allMatches, const std::vector<ParsedTag>& allPlayers, const std::vector<ParsedTag>& allRanks);

  /** \brief Consistency rule 3: each match must have four player labels
   */
  std::optional<std::string> checkRule03(const std::vector<ParsedTag>& allMatches, const std::vector<ParsedTag>& allPlayers, const std::vector<ParsedTag>& allRanks);

  /** \brief Each match in the first round must have an initial rank; the
   * sum of two initial ranks in a match must always be "numberOfPlayers + 1"
   */
  std::optional<std::string> checkRule04(const std::vector<ParsedTag>& allMatches, const std::vector<ParsedTag>& allPlayers, const std::vector<ParsedTag>& allRanks);

  /** \brief Rerefential integrity: each match may occur only zero or once as the "winner from X" or "loser from X" in a later round;
   * each match without an initial rank must have references to the previous match
   */
  std::optional<std::string> checkRule05(const std::vector<ParsedTag>& allMatches, const std::vector<ParsedTag>& allPlayers, const std::vector<ParsedTag>& allRanks);
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
  //std::optional<std::string> checkRule01(const std::vector<ParsedTag>& allMatches, const std::vector<ParsedTag>& allPlayers, const std::vector<ParsedTag>& allRanks);

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
}

#endif // SVGBRACKET_H
