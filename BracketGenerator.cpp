#include "BracketGenerator.h"

namespace QTournament
{

  BracketGenerator::BracketGenerator()
    : bracketType(BRACKET_SINGLE_ELIM)
  {
  }

//----------------------------------------------------------------------------

  BracketGenerator::BracketGenerator(int type)
  {
    if ((type != BRACKET_SINGLE_ELIM) && (type != BRACKET_DOUBLE_ELIM))
    {
      throw std::runtime_error("Request for an invalid bracket type");
    }

    bracketType = type;
  }

//----------------------------------------------------------------------------

  BracketMatchDataList BracketGenerator::genBracket__SingleElim(int numPlayers) const
  {
    BracketMatchDataList result;

    // return an empty list in case of invalid arguments
    if (numPlayers < 2)
    {
      return result;
    }

    BracketMatchData::resetBracketMatchId();

    //
    // Overall algorithm: we grow the bracket from the right (finals)
    // to the left (initial matches).
    //
    // the initial configuration is easy:
    // we start with finals, which is simply "first vs. second"
    BracketMatchData bmData;
    bmData.setInitialRanks(1, 2);
    bmData.nextMatchForLoser = -2;
    bmData.nextMatchForWinner = -1;
    bmData.depthInBracket = 0;
    result.append(bmData);

    int nActual = 2;
    int curDepth = 0;

    while (nActual < numPlayers)
    {
      // break the match list of the previous round down
      // and split each match into two new ones.
      //
      // Basic rule: the sum of the player ranks that
      // make up a match must be (nAtual + 1).
      //
      // Example:
      //  * Finals: nActual = 2, (nActual+1) = 3,
      //    1. plays vs. 2. ==> sum = 3
      //
      //  * Quarter finals: nActual = 8, (nActual+1) = 9
      //    1. vs. 8, 2. vs. 7., 3. vs. 6., 4. vs. 5.
      //
      // By breaking each match into a two new ones, we keep the tree's
      // consistency (e.g., 1vs8 and 4vs5 need to go into the same "sub-bracket")

      nActual *= 2;  // the number of players doubles in each round
      ++curDepth;
      BracketMatchDataList newMatches;

      for (auto prevMatch : result)   // loop over all matches generated so far
      {
        if (prevMatch.depthInBracket != (curDepth-1)) continue;  // skip all but the last round

        int rank1 = prevMatch.initialRank_Player1;
        int rank2 = prevMatch.initialRank_Player1;

        BracketMatchData newBracketMatch1;
        newBracketMatch1.setInitialRanks(rank1, (nActual+1)-rank1);
        newBracketMatch1.setNextMatchForWinner(prevMatch, 1);
        newBracketMatch1.nextMatchForLoser = BracketMatchData::NO_NEXT_MATCH;
        newBracketMatch1.depthInBracket = curDepth;

        BracketMatchData newBracketMatch2;
        newBracketMatch2.setInitialRanks(rank2, (nActual+1)-rank2);
        newBracketMatch2.setNextMatchForWinner(prevMatch, 2);
        newBracketMatch1.nextMatchForLoser = BracketMatchData::NO_NEXT_MATCH;
        newBracketMatch1.depthInBracket = curDepth;

        newMatches.append(newBracketMatch1);
        newMatches.append(newBracketMatch2);
      }
      result.append(newMatches);
    }

    removeUnusedMatches(result, numPlayers);

    return result;
  }

//----------------------------------------------------------------------------

  BracketMatchDataList BracketGenerator::getBracketMatches(int numPlayers) const
  {
    if (numPlayers < 2) return BracketMatchDataList();

    if (bracketType == BRACKET_SINGLE_ELIM) return genBracket__SingleElim(numPlayers);

    throw std::runtime_error("TODO: Unimplemented bracket type!");
  }

//----------------------------------------------------------------------------

  void BracketGenerator::removeUnusedMatches(BracketMatchDataList &bracketMatches, int numPlayers) const
  {
    // sort the bracket matches so that we always traverse the tree "from left to right" (read: from the
    // earlier to the later matches)
    std::sort(bracketMatches.begin(), bracketMatches.end(), getBracketMatchSortFunction_earlyRoundsFirst());

    // a little helper function that returns an iterator to a match with
    // a given ID
    auto getMatchById = [&bracketMatches](int matchId) {
      BracketMatchDataList::iterator i = bracketMatches.begin();
      while (i != bracketMatches.end())
      {
        if ((*i).getBracketMatchId() == matchId) return i;
        ++i;
      }
    };

    // a little helper function that update a player
    // in a bracket match pointed to by a match ID
    auto updatePlayer = [&](int matchId, int playerPos, int newVal) {
      auto iMatch = getMatchById(matchId);
      if (iMatch == bracketMatches.end()) return; // invalid ID

      if (playerPos == 1)
      {
        (*iMatch).initialRank_Player1 = newVal;
      } else {
        (*iMatch).initialRank_Player2 = newVal;
      }
    };


    // traverse the tree again and again, until we find no more changes to make
    bool matchesChanged = true;
    while (matchesChanged)
    {
      matchesChanged = false;

      // first step: flag all matches with initial ranks > numPlayers for BOTH players
      // as "to be deleted"
      BracketMatchDataList::iterator i = bracketMatches.begin();
      while (i != bracketMatches.end())
      {
        BracketMatchData bmd = *i;
        if ((bmd.initialRank_Player1 > numPlayers) && (bmd.initialRank_Player2 > numPlayers))
        {
          if (bmd.nextMatchForWinner > 0)
          {
            updatePlayer(bmd.nextMatchForWinner, bmd.nextMatchPlayerPosForWinner, BracketMatchData::UNUSED_PLAYER);
          }
          if (bmd.nextMatchForLoser > 0)
          {
            updatePlayer(bmd.nextMatchForLoser, bmd.nextMatchPlayerPosForLoser, BracketMatchData::UNUSED_PLAYER);
          }
          // actually delete the element from the match list
          i = bracketMatches.erase(i);
          matchesChanged = true;
        } else {
          ++i;
        }
      }

      // second step: delete / promote all matches in which only ONE player
      // is unavailable
      i = bracketMatches.begin();
      while (i != bracketMatches.end())
      {
        BracketMatchData bmd = *i;
        if (bmd.initialRank_Player1 > numPlayers)
        {
          // player 1 does not exist, this means that player 2 wins automatically;
          // find the match the winner will be promoted to
          if (bmd.nextMatchForWinner > 0)
          {
            updatePlayer(bmd.nextMatchForWinner, bmd.nextMatchPlayerPosForWinner, bmd.initialRank_Player2);

            // if player2 of "bmd" is not a directly seeded, initial player
            // but the winner/loser of a previous match, we need to update
            // that previous match, too
            if (bmd.initialRank_Player2 < 0)
            {
              int prevMatchId = -(bmd.initialRank_Player2);
              auto prevMatch = getMatchById(prevMatchId);
              auto nextMatch = getMatchById(bmd.nextMatchForWinner);
              if ((*prevMatch).nextMatchForWinner == bmd.getBracketMatchId())
              {
                (*prevMatch).setNextMatchForWinner(*nextMatch, bmd.nextMatchPlayerPosForWinner);
              } else {
                (*prevMatch).setNextMatchForLoser(*nextMatch, bmd.nextMatchPlayerPosForWinner);
              }
            }
          }

          // player 1 does not exist and player 2 wins automatically
          // ==> we have no loser! If we promote the (non-existing) loser to a next match
          // we need to update that match, too
          if (bmd.nextMatchForLoser > 0)
          {
            updatePlayer(bmd.nextMatchForLoser, bmd.nextMatchPlayerPosForLoser, BracketMatchData::UNUSED_PLAYER);
          }

          // we may only delete this match if the winner does not achieve a final rank.
          // Otherwise we would lose this ranking information.
          if (bmd.nextMatchForWinner >= 0)
          {
            i = bracketMatches.erase(i);
          } else {
            ++i;
          }

          // it is safe to stop processing here; the case that both
          // players are invalid is already captured by step 1 above
          matchesChanged = true;
          continue;
        }

        if (bmd.initialRank_Player2 > numPlayers)
        {
          // player 2 does not exist, this means that player 1 wins automatically;
          // find the match the winner will be promoted to
          if (bmd.nextMatchForWinner > 0)
          {
            updatePlayer(bmd.nextMatchForWinner, bmd.nextMatchPlayerPosForWinner, bmd.initialRank_Player1);

            // if player1 of "bmd" is not a directly seeded, initial player
            // but the winner/loser of a previous match, we need to update
            // that previous match, too
            if (bmd.initialRank_Player1 < 0)
            {
              int prevMatchId = -(bmd.initialRank_Player1);
              auto prevMatch = getMatchById(prevMatchId);
              auto nextMatch = getMatchById(bmd.nextMatchForWinner);
              if ((*prevMatch).nextMatchForWinner == bmd.getBracketMatchId())
              {
                (*prevMatch).setNextMatchForWinner(*nextMatch, bmd.nextMatchPlayerPosForWinner);
              } else {
                (*prevMatch).setNextMatchForLoser(*nextMatch, bmd.nextMatchPlayerPosForWinner);
              }
            }
          }

          // player 2 does not exist and player 1 wins automatically
          // ==> we have no loser! If we promote the (non-existing) loser to a next match
          // we need to update that match, too
          if (bmd.nextMatchForLoser > 0)
          {
            updatePlayer(bmd.nextMatchForLoser, bmd.nextMatchPlayerPosForLoser, BracketMatchData::UNUSED_PLAYER);
          }

          // we may only delete this match if the winner does not achieve a final rank.
          // Otherwise we would lose this ranking information.
          if (bmd.nextMatchForWinner >= 0)
          {
            i = bracketMatches.erase(i);
          } else {
            ++i;
          }
          continue;
        }

        ++i;
      }

    }
  }

//----------------------------------------------------------------------------

  std::function<bool (BracketMatchData&, BracketMatchData&)> BracketGenerator::getBracketMatchSortFunction_earlyRoundsFirst()
  {
    return [](BracketMatchData& bmd1, BracketMatchData& bmd2) {
      return bmd1.depthInBracket > bmd2.depthInBracket;
    };
  }

//----------------------------------------------------------------------------

  int BracketGenerator::getNumRounds(int numPlayers) const
  {
    if (numPlayers < 2) return -1;

    // I don't dare to use something like ceil(ln(numPlayers)/ln(2)) here
    // because I don't trust e.g. ln(8)/ln(2) to exactly yield 3 and not
    // three-point-something which is then rounded up to 4
    //
    // Thus I use a stupid loop here to count up the rounds
    int nRounds = 1;
    int n = 2;
    while (n < numPlayers)
    {
      n = n * 2;
      ++nRounds;
    }

    return nRounds;
  }

//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

  int BracketMatchData::lastBracketMatchId = 0;

  BracketMatchData::BracketMatchData()
  {
    bracketMatchId = ++lastBracketMatchId;
  }

  void BracketMatchData::resetBracketMatchId()
  {
    lastBracketMatchId = 0;
  }

//----------------------------------------------------------------------------

  int BracketMatchData::getBracketMatchId()
  {
    return getBracketMatchId();
  }

//----------------------------------------------------------------------------

  void BracketMatchData::setInitialRanks(int initialRank_P1, int initialRank_P2)
  {
    initialRank_Player1 = initialRank_P1;
    initialRank_Player2 = initialRank_P2;
  }

//----------------------------------------------------------------------------

  bool BracketMatchData::setNextMatchForWinner(BracketMatchData &nextBracketMatch, int posInNextMatch)
  {
    nextMatchForWinner = nextBracketMatch.getBracketMatchId();
    nextMatchPlayerPosForWinner = posInNextMatch;

    // keep a back-reference in the next match from which match the player came from
    if (posInNextMatch == 1)
    {
      nextBracketMatch.initialRank_Player1 = -bracketMatchId;
    }
    if (posInNextMatch == 2)
    {
      nextBracketMatch.initialRank_Player2 = -bracketMatchId;
    }
  }

//----------------------------------------------------------------------------

  bool BracketMatchData::setNextMatchForLoser(BracketMatchData &nextBracketMatch, int posInNextMatch)
  {
    nextMatchForLoser = nextBracketMatch.getBracketMatchId();
    nextMatchPlayerPosForLoser = posInNextMatch;

    // keep a back-reference in the next match from which match the player came from
    if (posInNextMatch == 1)
    {
      nextBracketMatch.initialRank_Player1 = -bracketMatchId;
    }
    if (posInNextMatch == 2)
    {
      nextBracketMatch.initialRank_Player2 = -bracketMatchId;
    }
  }

//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------

}
