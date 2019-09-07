#ifndef SVGRULECHECKS_H
#define SVGRULECHECKS_H

#include <optional>
#include <string>
#include <vector>

namespace QTournament::SvgBracket
{
  // Forward definition
  class ParsedTag;

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
}
#endif
