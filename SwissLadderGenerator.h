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

#ifndef SWISSLADDERGENERATOR_H
#define SWISSLADDERGENERATOR_H

#include <memory>
#include <functional>
#include <vector>
#include <tuple>
#include <unordered_map>

#include <QList>

namespace QTournament
{
  class SwissLadderGenerator
  {
  public:
    static constexpr int SOLUTION_FOUND = 0;
    static constexpr int NO_MORE_ROUNDS = -1;
    static constexpr int DEADLOCK = -2;
    SwissLadderGenerator(const vector<int>& _ranking, const vector<tuple<int, int>>& _pastMatches);
    int getNextMatches(vector<tuple<int, int>>& resultVector);

  protected:
    bool hasMatchBeenPlayed(int pair1Id, int pair2Id) const;
    pair<int, vector<int>> getEffectivePlayerList(int curByeRank);
    int getNextUnusedRank(const vector<bool>& isRankUsed, int minRank) const;
    int findOpponentRank(int pair1Rank, int minPair2Rank, const vector<bool>& isRankUsed, const vector<int> effPairList) const;
    bool matchSelectionCausesDeadlock(const vector<tuple<int, int>>& nextMatches);
    bool canBuildAnotherRound(const vector<tuple<int, int>>& remain, const vector<tuple<int, int>>& nextMatches) const;
    void flagMatchesWithPlayerPairOccurence(const vector<tuple<int, int> >& matchSet, vector<bool>& flagList, const tuple<int, int>& refMatch, bool newState) const;
    vector<int> getPotentialByePairs(const vector<tuple<int, int> >& optionalAdditionalMatches) const;
    int findByePlayerInMatchSet(const vector<tuple<int, int>>& matchSet = vector<tuple<int, int>>()) const;

  private:
    vector<int> ranking;
    vector<tuple<int, int>> pastMatches;
    int roundsPlayed;
    int matchesPerRound;
    size_t nPairs;
    unordered_map<int, int> matchCount;
    vector<tuple<int, int>> remainingMatches;  // will be initialized upon the first call of matchSelectionCausesDeadlock()
  };

}
#endif // SWISSLADDERGENERATOR_H
