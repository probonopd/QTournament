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

#include "SwissLadderGenerator.h"

using namespace std;

namespace QTournament
{

  SwissLadderGenerator::SwissLadderGenerator(const vector<int>& _ranking, const vector<tuple<int, int> >& _pastMatches)
    :ranking{_ranking}, pastMatches{_pastMatches}, nPairs{_ranking.size()}
  {
    // no consistency checks here (e.g., do the PlayerPairIDs
    // in _ranking match those in _pastMatches). Just make sure that
    // ranking is not empty
    if (ranking.empty())
    {
      throw std::invalid_argument("Empty ranking in SwissLadderGenerator!");
    }

    // calculate the number of matches per round
    matchesPerRound = ((nPairs % 2) == 0) ? nPairs / 2 : (nPairs - 1) / 2;

    // determine the number of rounds that have been played
    roundsPlayed = pastMatches.size() / matchesPerRound;

    // cross-check to ensure that the size of pastMatches is consistent
    if ((roundsPlayed * matchesPerRound) != pastMatches.size())
    {
      throw std::invalid_argument("SwissLadderGenerator: inconsistent size of list of past matches");
    }

    // count the number of matches that each player already has played
    for (const tuple<int, int>& m : pastMatches)
    {
      int pp1Id = get<0>(m);
      int pp2Id = get<1>(m);

      int& ref1 = matchCount[pp1Id];
      ++ref1;

      int& ref2 = matchCount[pp2Id];
      ++ref2;
    }

  }

  //----------------------------------------------------------------------------

  int SwissLadderGenerator::getNextMatches(vector<tuple<int, int>>& resultVector)
  {
    //
    // a few status variables for the generator
    //

    // is there another round at all?
    int maxRounds = ((nPairs % 2) == 0) ? nPairs - 1 : nPairs;
    if (maxRounds == roundsPlayed) return NO_MORE_ROUNDS; // no more rounds

    // do we need a deadlock prevention check?
    // This check is necessary when generating the matches
    // for the round "nPairs - 3" (or "maxRounds - 2") in order to prevent
    // deadlock AFTER playing that round
    int nextRound = roundsPlayed + 1;
    bool needsDeadlockPrevention = (nextRound == (maxRounds - 2));

    // the rank of the player that has a bye
    // in the next round. Is initialized to
    // "one behind the last rank"
    int curByeRank = nPairs;

    //
    // The generator itself
    //
    while (true)
    {
      // in case we start all over again because we couldn't
      // find a solution, clear all previous solution fragments
      resultVector.clear();

      // get the list of effective players for
      // the next round. remove one player (bye)
      // if necessary
      vector<int> ppList;
      tie(curByeRank, ppList) = getEffectivePlayerList(curByeRank);
      if (ppList.empty())
      {
        return DEADLOCK;
      }
      size_t effPairCount = ppList.size();

      // prepare a list of already "used" ranks for next matches
      vector<int> usedRanks;

      // keep a list of "isRankUsed"-flags. This is redundant to
      // the usedRanks-list, but makes some operations easier
      vector<bool> isRankUsed;
      for (size_t i=0; i < effPairCount; ++i) isRankUsed.push_back(false);

      // start building new player combinations. Follow the approach
      // "first against second", "third against fourth", etc. but avoid
      // playing the same match twice.
      //
      // the algorithm uses two indices: one for the first player, the
      // other one for the second player.
      int pair1Rank = 0;
      int minPair2Rank = 1;
      bool foundSolution = false;
      while(usedRanks.size() != effPairCount)
      {
        // determine the smallest unused player rank
        if (pair1Rank < 0)
        {
          pair1Rank = getNextUnusedRank(isRankUsed, 0);
          minPair2Rank = pair1Rank + 1;
        }

        // is there a possible other pair that results
        // in a unique match?
        int pair2Rank = findOpponentRank(pair1Rank, minPair2Rank, isRankUsed, ppList);
        bool foundMatch = (pair2Rank > 0);

        if (foundMatch)
        {
          // we found a unique match
          //
          // ==> store it
          int pair1Id = ppList[pair1Rank];
          int pair2Id = ppList[pair2Rank];

          resultVector.push_back(make_tuple(pair1Id, pair2Id));
          usedRanks.push_back(pair1Rank);
          usedRanks.push_back(pair2Rank);
          isRankUsed[pair1Rank] = true;
          isRankUsed[pair2Rank] = true;

          // set pair1Rank to -1 as an indication to start
          // over with a fresh pair1Rank in the next iteration
          pair1Rank = -1;
        }

        // Check for deadlocks. The conditions are:
        //    * curByeRank < 1   (-1 = no byes, even number of players; 0 = already used the top player for a bye)
        //    * no match found for player at rank 0
        //
        if ((curByeRank < 1) && (!foundMatch) && (pair1Rank == 0))
        {
          resultVector.clear();
          return DEADLOCK;
        }

        // if we couldn't find a match for the player
        // at rank 0 and have not yet exhausted all bye
        // options, continue with another bye selection
        if ((curByeRank > 0) && (!foundMatch) && (pair1Rank == 0))
        {
          break;
        }

        // perform the deadlock prevention check
        // if we just completed the set of matches and
        // if the deadlock prevention check is necessary
        bool deadlockCheckTriggered = false;
        if (needsDeadlockPrevention && (usedRanks.size() == ppList.size()))
        {
          deadlockCheckTriggered = matchSelectionCausesDeadlock(resultVector);
        }

        // if we couldn't find a match for a player
        // at a rank > 0, undo the last match selection
        // and continue with the match search for the
        // previous pair1Rank
        //
        // we also undo the previous match selection if
        // the deadlock check resulted in a "yes"
        if (deadlockCheckTriggered || ((!foundMatch && (pair1Rank > 0))))
        {
          resultVector.pop_back();
          pair2Rank = usedRanks.back();
          usedRanks.pop_back();
          pair1Rank = usedRanks.back();
          usedRanks.pop_back();
          isRankUsed[pair1Rank] = false;
          isRankUsed[pair2Rank] = false;

          minPair2Rank = pair2Rank + 1;
        }

        // check if we were able to find a full set of matches
        foundSolution = (foundMatch && (usedRanks.size() == effPairCount));
      }

      // if we returned from the inner while loops with a full solution,
      // we're done. Otherwise, we need to continue with the outmost while loop
      // and a new bye selection
      if (foundSolution) break;
    }

    return SOLUTION_FOUND;
  }

  //----------------------------------------------------------------------------

  bool SwissLadderGenerator::hasMatchBeenPlayed(int pair1Id, int pair2Id) const
  {
    tuple<int, int> regular{pair1Id, pair2Id};
    tuple<int, int> swapped{pair2Id, pair1Id};

    auto it = find_if(pastMatches.cbegin(), pastMatches.cend(), [&](const tuple<int, int>& m) {
      if (m == regular) return true;
      return (m == swapped);
    });

    return (it != pastMatches.cend());
  }

  //----------------------------------------------------------------------------

  pair<int, vector<int>> SwissLadderGenerator::getEffectivePlayerList(int curByeRank)
  {
    // if the number of pairs is even, all pairs participate in the next round
    if ((nPairs % 2) == 0)
    {
      return make_pair(-1, ranking);
    }

    // we have an odd number of players.
    //
    // find the next player that has a bye, starting with
    // curByeRank - 1
    int nextByeRank = curByeRank -1;
    while (nextByeRank >= 0)
    {
      int ppId = ranking[nextByeRank];
      int nRounds = matchCount[ppId];

      // if the player has participated in all rounds
      // so far, this player will get the next bye
      if (nRounds == roundsPlayed)
      {
        // create a copy of the ranking
        vector<int> ppList = ranking;

        // remove the player pair at nextByeRank
        ppList.erase(ppList.begin() + nextByeRank);

        // return the list and the erased rank
        return make_pair(nextByeRank, ppList);
      }

      // the player already had a bye, check
      // the next one
      --nextByeRank;
    }

    // we couldn't find a bye player, that's weird and
    // should not happen
    return make_pair(-1, vector<int>{});
  }

  //----------------------------------------------------------------------------

  int SwissLadderGenerator::getNextUnusedRank(const vector<bool>& isRankUsed, int minRank) const
  {
    size_t rank = minRank;
    while (rank < isRankUsed.size())
    {
      if (isRankUsed[rank] == false) return rank;
      ++rank;
    }

    return -1;
  }

  //----------------------------------------------------------------------------

  int SwissLadderGenerator::findOpponentRank(int pair1Rank, int minPair2Rank, const vector<bool>& isRankUsed, const vector<int> effPairList) const
  {
    if ((pair1Rank < 0) || (pair1Rank > (effPairList.size() - 2))) return -1;
    if ((minPair2Rank <= pair1Rank) || (minPair2Rank > (effPairList.size() - 1))) return -1;

    int pair1Id = effPairList[pair1Rank];

    // keep the first pair fix while searching for
    // a second pair that results in a unique match
    int pair2Rank = minPair2Rank;
    while (pair2Rank < effPairList.size())
    {
      // find the next unused rank
      if (isRankUsed[pair2Rank])
      {
        ++pair2Rank;
        continue;
      }

      int pair2Id = effPairList[pair2Rank];

      if (hasMatchBeenPlayed(pair1Id, pair2Id))
      {
        ++pair2Rank;
        continue;
      }

      return pair2Rank;
    }

    return -1;
  }

  //----------------------------------------------------------------------------

  bool SwissLadderGenerator::matchSelectionCausesDeadlock(const vector<tuple<int, int>>& nextMatches)
  {
    // check whether the selection of next matches causes
    // a deadlock after playing those played matches in the next
    // round

    // if we have more than 20 players, we do not perform
    // this check because the computation might take too long.
    //
    // in this case we simply assume that the match selection
    // is valid and we rely on the deadlock detection in the
    // match generator.
    if (ranking.size() > 20) return false;


    // Algorithm:
    //
    // Step 1: determine all matches for this category (means: all player pair combinations)
    // Step 2: subtract what has been played in the previous rounds (pastMatches)
    // Step 3: subtract what is to be played in the next round (nextMatches)
    // Step 4: check if the remaining matches allow for at least one more round
    //
    // Do steps 1 and 2 only once to avoid too many computations

    if (remainingMatches.empty())
    {
      //
      // Step 1: determine all matches and store them in "remainingMatches"
      //
      for (size_t idxFirst = 0; idxFirst < (ranking.size() - 1); ++idxFirst)
      {
        int idFirst = ranking[idxFirst];

        for (size_t idxSecond = idxFirst + 1; idxSecond < ranking.size(); ++idxSecond)
        {
          int idSecond = ranking[idxSecond];
          remainingMatches.push_back(make_tuple(idFirst, idSecond));
        }
      }

      //
      // Step 2: subtract already played matches
      //
      for (tuple<int, int>& m : pastMatches)
      {
        tuple<int, int> swapped = make_tuple(get<1>(m), get<0>(m));
        auto it = find(remainingMatches.begin(), remainingMatches.end(), m);
        if (it == remainingMatches.end())
        {
          it = find(remainingMatches.begin(), remainingMatches.end(), swapped);
        }
        if (it != remainingMatches.end())
        {
          remainingMatches.erase(it);
        }
      }
    }

    //
    // Step 3: subtract next matches
    //
    vector<tuple<int, int>> remain = remainingMatches;
    for (const tuple<int, int>& m : nextMatches)
    {
      tuple<int, int> swapped = make_tuple(get<1>(m), get<0>(m));
      auto it = find(remain.begin(), remain.end(), m);
      if (it == remain.end())
      {
        it = find(remain.begin(), remain.end(), swapped);
      }
      if (it != remain.end())
      {
        remain.erase(it);
      }
    }

    //
    // Step 4: check if we can create at least one more round from the
    // remaining matches
    //
    return !(canBuildAnotherRound(remain, nextMatches));
  }

  //----------------------------------------------------------------------------

  bool SwissLadderGenerator::canBuildAnotherRound(const vector<tuple<int, int> >& remain, const vector<tuple<int, int> >& nextMatches) const
  {
    // a list of indices to matchSet that make up a
    // match combination for the next round
    vector<int> usedMatchSequence;

    // a list of flags that tags each entry in matchSet as used or not
    vector<bool> isMatchUsed;
    for (size_t i=0; i < remain.size(); ++i) isMatchUsed.push_back(false);

    // calculate the number of matches for a round
    size_t requiredMatchCount = ((ranking.size() % 2) == 0) ? ranking.size() / 2 : (ranking.size() - 1) / 2;

    size_t idxNextMatch = 0;
    while (usedMatchSequence.size() != requiredMatchCount)
    {
      // find the next match that has not already been used
      // and that contains only "unused" player pairs
      while (idxNextMatch < remain.size())
      {
        if (isMatchUsed[idxNextMatch])
        {
          ++idxNextMatch;
          continue;
        }

        break;
      }

      // if we found a match, store it andint flag
      // all other matches with these player pairs as "used"
      if (idxNextMatch < remain.size())
      {
        usedMatchSequence.push_back(idxNextMatch);
        tuple<int, int> m = remain[idxNextMatch];

        // flag all matches with these two pairs
        // as "used".
        flagMatchesWithPlayerPairOccurence(remain, isMatchUsed, m, true);

        // start all over again
        idxNextMatch = 0;
      }

      // if we have an odd number of players and we just
      // found a complete match combination, we need to
      // make sure that the implicitly selected "bye" player
      // is valid. Each player should only have ONE bye
      if ((usedMatchSequence.size() == requiredMatchCount) && ((ranking.size() % 2) != 0))
      {
        vector<tuple<int, int>> matchSubset;
        for (int idx : usedMatchSequence)
        {
          matchSubset.push_back(remain[idx]);
        }

        int byePairId = findByePlayerInMatchSet(matchSubset);
        if (byePairId >= 0)
        {
          vector<int> potentialBye = getPotentialByePairs(nextMatches);
          auto it = find(potentialBye.begin(), potentialBye.end(), byePairId);

          // if the selected bye pair is not in the list
          // of permitted bye players, we fake a "match not found"
          // which causes the deletion of the last match in the
          // following code block.
          //
          // This deletion, in turn, will cause another search iteration
          if (it == potentialBye.end())
          {
            idxNextMatch = remain.size();
          }
        }
      }

      // if we couldn't find a match, then the previous match
      // selection was wrong. We undo the previous match selection
      // and continue with the next index
      if (idxNextMatch == remain.size())
      {
        idxNextMatch = usedMatchSequence.back();
        usedMatchSequence.pop_back();
        tuple<int, int> m = remain[idxNextMatch];

        // flag all matches with these two pairs
        // as "unused".
        flagMatchesWithPlayerPairOccurence(remain, isMatchUsed, m, false);

        // continue with the next match
        ++idxNextMatch;

        // stop criterion: if the list of used matches is empty
        // and we are requiredMatchCount matches away from the
        // end of the match list, there is no solution
        if (usedMatchSequence.empty() && (idxNextMatch == (remain.size() - requiredMatchCount + 1)))
        {
          return false;
        }
      }
    }

    return true;
  }

  //----------------------------------------------------------------------------

  void SwissLadderGenerator::flagMatchesWithPlayerPairOccurence(const vector<tuple<int, int> >& matchSet, vector<bool>& flagList,
                                                                const tuple<int, int>& refMatch, bool newState) const
  {
    int pp1Id = get<0>(refMatch);
    int pp2Id = get<1>(refMatch);

    for (size_t i=0; i < matchSet.size(); ++i)
    {
      // we don't need to check matches that already
      // are in the new state
      if (flagList[i] == newState) continue;

      const tuple<int, int>& m = matchSet[i];
      int other1Id = get<0>(m);
      int other2Id = get<1>(m);

      if ((other1Id == pp1Id) || (other1Id == pp2Id) || (other2Id == pp1Id) || (other2Id == pp2Id))
      {
        flagList[i] = newState;
      }
    }
  }

  //----------------------------------------------------------------------------

  vector<int> SwissLadderGenerator::getPotentialByePairs(const vector<tuple<int, int> >& optionalAdditionalMatches) const
  {
    unordered_map<int, int> matchCountCopy = matchCount;

    for (const tuple<int, int>& m : optionalAdditionalMatches)
    {
      int pp1Id = get<0>(m);
      int pp2Id = get<1>(m);

      int& ref1 = matchCountCopy[pp1Id];
      ++ref1;

      int& ref2 = matchCountCopy[pp2Id];
      ++ref2;
    }

    int nRounds = optionalAdditionalMatches.empty() ? roundsPlayed : (roundsPlayed + 1);

    vector<int> result;
    for (int ppId : ranking)
    {
      if (matchCountCopy.at(ppId) == nRounds) result.push_back(ppId);
    }

    return result;
  }

  //----------------------------------------------------------------------------

  int SwissLadderGenerator::findByePlayerInMatchSet(const vector<tuple<int, int> >& matchSet) const
  {
    vector<int> allPlayers = ranking;

    for (const tuple<int, int>& m : matchSet)
    {
      int ppId = get<0>(m);
      auto it = find(allPlayers.begin(), allPlayers.end(), ppId);

      // the following "if" should always be true, but let's be conservative...
      if (it != allPlayers.end()) allPlayers.erase(it);

      ppId = get<1>(m);
      it = find(allPlayers.begin(), allPlayers.end(), ppId);
      if (it != allPlayers.end()) allPlayers.erase(it);
    }

    if (allPlayers.empty()) return -1;

    return allPlayers[0];
  }


//----------------------------------------------------------------------------

}
