/*
 *    This is QTournament, a badminton tournament management program.
 *    Copyright (C) 2014 - 2015  Volker Knollmann
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

