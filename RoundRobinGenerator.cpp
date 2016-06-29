/*
 *    This is QTournament, a badminton tournament management program.
 *    Copyright (C) 2014 - 2016  Volker Knollmann
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

/*
 * Generates match pairs for round robin matches
 *
 * Basic Algorithm:
 * Construct match as "blocks" of player IDs, rotating around
 * a pivot point in the top left corner:
 *
 * Round 0:   Round 1:   Round 2:   Round 3:    etc.
 * 0 1 2 3    0 2 3 4    0 3 4 5    0 4 5 6
 * 7 6 5 4    1 7 6 5    2 1 7 6    3 2 1 7
 *
 * Matches in round 1:  0 vs 7, 1 vs 6, 2 vs 5, 3 vs 4
 * Matches in round 2:  0 vs 1, 2 vs 7, 3 vs 6, 4 vs 5
 * etc.
 *
 *
 * Now we assign symbolic names to the numbers in
 * the block:
 *
 *    n0  n1  n2  n3
 *    n7  n6  n5  n4
 *
 * This yields:
 *
 *   Round 0: n0 = 0, n1 = 1, n2 = 2, ..., n7 = 7
 *   Round 1: n0 = 0, n1 = 2, n2 = 3, ..., n6 = 7, n7 = 1
 *   Round 2: n0 = 0, n1 = 3, n3 = 4, ..., n5 = 7, n6 = 1, n7 = 2
 *   etc.
 *
 * Arranging the numbers and the rounds in a matrix shows the
 * pattern behind the numbers. The columns indicate the
 * round number:
 *
 *      | R0| R1| R2| R3| R4| R5| R6|
 *   ---+---+---+---+---+---+---+---+
 *   n0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
 *   n1 | 1 | 2 | 3 | 4 | 5 | 6 | 7 |
 *   n2 | 2 | 3 | 4 | 5 | 6 | 7 | 1 |
 *   n3 | 3 | 4 | 5 | 6 | 7 | 1 | 2 |
 *   n4 | 4 | 5 | 6 | 7 | 1 | 2 | 3 |
 *   n5 | 5 | 6 | 7 | 1 | 2 | 3 | 4 |
 *   n6 | 6 | 7 | 1 | 2 | 3 | 4 | 5 |
 *   n7 | 7 | 1 | 2 | 3 | 4 | 5 | 6 |
 *
 * This pattern can be expressed as follows:
 *
 *   N : number of players, MUST BE EVEN!!
 *
 *   Number of rounds = N - 1
 *   Matches per round = N / 2
 *
 *   r = round number (index); r = 0, ..., N-2
 *   p = position of the number n or rather: "index to n": p = 0, ..., N-1
 *   n_p = number at index p in the "number block" above: n0, n1, n2, ..., n_(N-1)
 *
 *   Using these symbols, we get:
 *
 *   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 *   !!!   n_p = (r + p -1) mod (N -1) + 1    for 0 < p < N   !!!
 *   !!!   n_p = 0                            for p = 0       !!!
 *   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 *
 *
 * In each round, we have the following matches:
 *
 *   n0 : n_(N-1) ,  n1 : n_(N-2), ..., n_((N-1) / 2) : n_((N+1) / 2)
 *
 *
 * In case we have an odd number of players, we simply add a dummy
 * player. The "real player" playing against the "dummy player" has
 * a bye in that particular round.
 *
 *
 * This class implements a functor that returns the matches
 * for a specific number of players and a specific round
 *
 */

#include "RoundRobinGenerator.h"

namespace QTournament {

  RoundRobinGenerator::RoundRobinGenerator()
  {
  }

//----------------------------------------------------------------------------

  vector<tuple<int, int>> RoundRobinGenerator::operator() (int numPlayers, int round)
  {
    // fake a dummy player, if necessary
    bool isOdd = (numPlayers % 2) != 0;
    if (isOdd) ++numPlayers;

    // prepare the result container
    vector<tuple<int, int>> result;

    // check parameter validity
    if (numPlayers < 2) return result;
    if (round < 0) return result;
    int maxNumRounds = numPlayers-1;
    if (round >= maxNumRounds) return result;

    // define a lambda for calculating n_p
    auto n = [numPlayers] (const int r, const int p) {
      return (p == 0) ? 0 : (r + p - 1) % (numPlayers - 1) + 1;
    };

    // loop over the matches to be created
    int pMax = numPlayers - 1;
    for(int matchNum = 0; matchNum < (numPlayers/2); ++matchNum)
    {
      int player1 = n(round, matchNum);
      int player2 = n(round, pMax - matchNum);

      // skip matches involving the dummy player
      //
      // the dummy player has the index "numPlayers - 1" which
      // happens to be "pMax". So we abuse pMax here for
      // better readibility
      if ((isOdd) && ((player1 == pMax) || (player2 == pMax))) continue;

      result.push_back(make_tuple(player1, player2));
    }

    return result;
  }

//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


}

