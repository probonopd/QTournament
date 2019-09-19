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

#ifndef BACKENDAPI_H
#define	BACKENDAPI_H

#include <memory>
#include <optional>

//#include "TournamentDB.h"
#include "TournamentErrorCodes.h"
#include "SvgBracket.h"

namespace QTournament
{
  class TournamentDB;
  class Match;
  class Category;

  namespace SvgBracket
  {
    class BracketMatchDataList;
  }

  /** \brief Just a player pair and its rank
   */
  struct SimplifiedRankingEntry
  {
    PlayerPairRefId ppId{-1};
    Rank rank{-1};
  };

  /** \brief Simplified ranking information: just round the round and
   * a list of ranked player pairs.
   *
   * \warning The ranks are not guaranteed to be continuous! If only
   * two of twenty players achieved a final rank in the given round,
   * only these two players are included.
   */
  struct SimplifiedRanking
  {
    Round round;   ///< the round for which the contained ranking is valid
    std::vector<SimplifiedRankingEntry> ranks;   ///< the pairs and their ranks, sorted best to worst player pair
  };

  /** \brief Only queries that are guaranteed to not modify the database
   */
  namespace API::Qry
  {
    /** \returns the next callable match with the lowest match number
     */
    std::optional<Match> nextCallableMatch(
        const TournamentDB& db
        );

    /** \returns `true` if the given round for the given category is a valid
     * bracket round (==> we can draw a bracket for that round).
     */
    bool isBracketRound(
        const TournamentDB& db,
        const Category& cat,
        Round r
        );

    /** \brief Determines which player pairs achieved a final rank in a bracket
     * after we applied the seeding and a list of matches
     *
     * You can determine player that achieve a final rank without playing
     * by just assigning the seeding and set the round number to zero.
     *
     * Essentially, the final will be determined from combining the data
     * contained the finished matches (winner rank, loser rank, winner, loser)
     * with the data in the tree (assigned branches with dead sibling branches).
     *
     * For "groups with KO" categories: set `firstRound` to the number of the
     * last group round in order to get the ranks after intermediate seeding. This
     * is equivalent to "Round zero" in pure bracket categories.
     *
     * The value of 'lastRound' is ignored if it less or equal to 'firstRound'.
     * If the value of 'lastRound' is greater than the number of finished rounds,
     * we only return rankings up to and including the last finished round.
     *
     * \returns a vector with one SimplifiedRanking entry per round in ascending round order, empty on error
     */
    std::vector<SimplifiedRanking> getBracketRanks(
        const Category& cat,
        const Round& firstRound,   ///< the first round for which to include ranks (0 = after seeding)
        const Round& lastRound = Round{-1}   ///< the last round for which to include ranks (ignored if less or equal to firstRound)
        );

    /** \brief Takes a list of matches and extracts all winner / loser ranks from it
     *
     * \returns a list of PairID / ranks tuples, sorting in ascending rank order
     */
    std::vector<SimplifiedRankingEntry> extractSortedRanksFromMatchList(
        const MatchList& maList   ///< the list of matches from which to extract the ranking information
        );

    /** \brief Finds all "matchless ranks" (ranks resulting from the cmobination of
     * a dead and an assigned branch) in a bracket match list
     *
     * \returns a list of PairID / ranks tuples, sorting in ascending rank order
     */
    std::vector<SimplifiedRankingEntry> extractSortedRanksFromBracket(
        const SvgBracket::BracketMatchDataList& bmdl   ///< the list of bracket matches from which to extract the ranking information
        );

    /** \brief Takes a category, looks up its bracket match system and returns
     * a list of all bracket matches with the seeding already applied.
     *
     * \pre The category is valid for bracket matches and has a valid seeding
     *
     * \returns a list of bracket matches with applies seeding, the matches sorted
     * in ascending match number
     */
    SvgBracket::BracketMatchDataList getSeededBracketMatches(
        const Category& cat  ///< the category for which to retrieve the bracket matches
        );
  }

  /** \brief Helper function for the API functions itself; what's inside "API::Internal"
   * should not be called by the GUI directly
   */
  namespace API::Internal
  {
    /** \brief Convenience function that generates a set of bracket matches for
     * a list of PlayerPairs.
     *
     * This function does not do any error checking
     * whether the PlayerPairs or other arguments are valid. It assumes
     * that those checks have been performed before and that it's generally
     * safe to generate the matches here and now.
     *
     * \return error code
      */
    Error generateBracketMatches(
        const Category& cat,   ///< the category to operate on
        SvgBracketMatchSys brMatchSys,   ///< the type of bracket to use (e.g., single elimination)
        const PlayerPairList& seeding,   ///< the initial list of player pairs, sorted from best player (index 0) to weakest player
        int firstRoundNum   ///< the number of the first round of bracket matches
        );

    /** \brief Takes a SimplifiedRanking object and sortes the ranks
     * in place
     */
    void sortSimplifiedRanking(
        std::vector<SimplifiedRankingEntry>& sreList   ///< the ranking that shall be sorted in place
        );
  }

  namespace API
  {
  }
}
#endif	/* COURT_H */

