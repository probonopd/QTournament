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

  }

  namespace API
  {
  }
}
#endif	/* COURT_H */

