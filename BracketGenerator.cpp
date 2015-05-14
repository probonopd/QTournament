#include "BracketGenerator.h"
#include <memory>
#include <vector>

namespace QTournament
{

  BracketGenerator::BracketGenerator()
    : bracketType(BRACKET_SINGLE_ELIM)
  {
  }

//----------------------------------------------------------------------------

  BracketGenerator::BracketGenerator(int type)
  {
    if ((type != BRACKET_SINGLE_ELIM) &&
        (type != BRACKET_DOUBLE_ELIM) &&
        (type != BRACKET_RANKING1))
    {
      throw std::runtime_error("Request for an invalid bracket type");
    }

    bracketType = type;
  }

//----------------------------------------------------------------------------

  upBracketMatchDataVector BracketGenerator::genBracket__SingleElim(int numPlayers) const
  {
    upBracketMatchDataVector result;

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
    upBracketMatchData bmData = upBracketMatchData(new BracketMatchData);
    bmData->setInitialRanks(1, 2);
    bmData->nextMatchForLoser = -2;
    bmData->nextMatchForWinner = -1;
    bmData->depthInBracket = 0;
    result.push_back(std::move(bmData));

    // prepare a match for third place but don't store it yet
    // in the result list (otherwise it would be part of the
    // "split-each-match-into-two-new-ones"-algorithm (see below)
    upBracketMatchData thirdPlaceMatch = upBracketMatchData(new BracketMatchData);
    thirdPlaceMatch->setInitialRanks(3, 4);
    thirdPlaceMatch->nextMatchForLoser = -4;
    thirdPlaceMatch->nextMatchForWinner = -3;
    thirdPlaceMatch->depthInBracket = 0;

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

      int matchCountBeforeWhile = result.size();
      int cnt=0;
      while (cnt < matchCountBeforeWhile)   // loop over all matches generated so far
      {
        upBracketMatchData& prevMatch = result.at(cnt);
        if (prevMatch->depthInBracket != (curDepth-1))
        {
          ++cnt;
          continue;  // skip all but the last round
        }

        int rank1 = prevMatch->initialRank_Player1;
        int rank2 = prevMatch->initialRank_Player2;

        upBracketMatchData newBracketMatch1 = upBracketMatchData(new BracketMatchData);
        newBracketMatch1->setInitialRanks(rank1, (nActual+1)-rank1);
        newBracketMatch1->setNextMatchForWinner(*prevMatch, 1);
        newBracketMatch1->nextMatchForLoser = BracketMatchData::NO_NEXT_MATCH;
        newBracketMatch1->depthInBracket = curDepth;

        upBracketMatchData newBracketMatch2 = upBracketMatchData(new BracketMatchData);
        newBracketMatch2->setInitialRanks(rank2, (nActual+1)-rank2);
        newBracketMatch2->setNextMatchForWinner(*prevMatch, 2);
        newBracketMatch2->nextMatchForLoser = BracketMatchData::NO_NEXT_MATCH;
        newBracketMatch2->depthInBracket = curDepth;

        // a special treatment for semifinals: losers get a match for third place
        if ((curDepth == 1) && (numPlayers > 3))
        {
          newBracketMatch1->setNextMatchForLoser(*thirdPlaceMatch, 1);
          newBracketMatch2->setNextMatchForLoser(*thirdPlaceMatch, 2);
          result.push_back(std::move(thirdPlaceMatch));
        }

        result.push_back(std::move(newBracketMatch1));
        result.push_back(std::move(newBracketMatch2));
        ++cnt;
      }
    }

    removeUnusedMatches(result, numPlayers);

    return result;
  }

//----------------------------------------------------------------------------

  upBracketMatchDataVector BracketGenerator::genBracket__Ranking1(int numPlayers) const
  {
    upBracketMatchDataVector result;

    // return an empty list in case of invalid arguments
    if ((numPlayers < 2) || (numPlayers > 16))
    {
      return result;
    }

    BracketMatchData::resetBracketMatchId();

    // hard-code the bracket matches according to a
    // given tournament bracket
    int rawBracketData[36][7] =
    {
      // initialRank1, initialRank2, nextMatchWinner, nextMatchLoser, posWinner, posLoser, depth
      {1,                   16,            10,              9,            1,        1,       4},   // Match 1
      {8,                    9,            10,              9,            2,        2,       4},   // Match 2
      {4,                   13,            12,             11,            1,        1,       4},   // Match 3
      {5,                   12,            12,             11,            2,        2,       4},   // Match 4
      {11,                   6,            14,             13,            1,        1,       4},   // Match 5
      {14,                   3,            14,             13,            2,        2,       4},   // Match 6
      {10,                   7,            16,             15,            1,        1,       4},   // Match 7
      {15,                   2,            16,             15,            2,        2,       4},   // Match 8

      {-1,                  -2,            17,             21,            1,        1,       3},   // Match 9
      {-1,                  -2,            27,             19,            1,        2,       3},   // Match 10
      {-3,                  -4,            18,             22,            1,        1,       3},   // Match 11
      {-3,                  -4,            27,             20,            2,        2,       3},   // Match 12
      {-5,                  -6,            19,             21,            1,        2,       3},   // Match 13
      {-5,                  -6,            28,             17,            1,        2,       3},   // Match 14
      {-7,                  -8,            20,             22,            1,        2,       3},   // Match 15
      {-7,                  -8,            28,             18,            2,        2,       3},   // Match 16

      {-9,                 -14,            25,             23,            1,        1,       2},   // Match 17
      {-11,                -16,            25,             24,            2,        1,       2},   // Match 18
      {-13,                -10,            26,             23,            1,        2,       2},   // Match 19
      {-15,                -12,            26,             24,            2,        2,       2},   // Match 20

      {-9,                 -13,            30,             29,            1,        1,       1},   // Match 21
      {-11,                -15,            30,             29,            2,        2,       1},   // Match 22
      {-17,                -19,            32,             31,            1,        1,       1},   // Match 23
      {-18,                -20,            32,             31,            2,        2,       1},   // Match 24
      {-17,                -18,            34,             33,            1,        1,       1},   // Match 25
      {-19,                -20,            34,             33,            2,        2,       1},   // Match 26
      {-10,                -12,            36,             35,            1,        1,       1},   // Match 27
      {-14,                -16,            36,             35,            2,        2,       1},   // Match 28

      {-21,                -22,           -15,            -16,            0,        0,       0},   // Match 29
      {-21,                -22,           -13,            -14,            0,        0,       0},   // Match 30
      {-23,                -24,           -11,            -12,            0,        0,       0},   // Match 31
      {-23,                -24,            -9,            -10,            0,        0,       0},   // Match 32
      {-25,                -26,            -7,             -8,            0,        0,       0},   // Match 33
      {-25,                -26,            -5,             -6,            0,        0,       0},   // Match 34
      {-27,                -28,            -3,             -4,            0,        0,       0},   // Match 35
      {-27,                -28,            -1,            - 2,            0,        0,       0},   // Match 36
    };

    // convert the hard-coded data into bracket match data entries
    for (int i=0; i < 36; ++i)
    {
      upBracketMatchData newBracketMatch = upBracketMatchData(new BracketMatchData);

      newBracketMatch->initialRank_Player1 = rawBracketData[i][0];
      newBracketMatch->initialRank_Player2 = rawBracketData[i][1];
      newBracketMatch->nextMatchForWinner = rawBracketData[i][2];
      newBracketMatch->nextMatchForLoser = rawBracketData[i][3];
      newBracketMatch->nextMatchPlayerPosForWinner = rawBracketData[i][4];
      newBracketMatch->nextMatchPlayerPosForLoser = rawBracketData[i][5];
      newBracketMatch->depthInBracket = rawBracketData[i][6];

      result.push_back(std::move(newBracketMatch));
    }

    removeUnusedMatches(result, numPlayers);

    return result;
  }


//----------------------------------------------------------------------------

  BracketMatchDataList BracketGenerator::getBracketMatches(int numPlayers) const
  {
    if (numPlayers < 2) return BracketMatchDataList();

    upBracketMatchDataVector upResult;
    switch (bracketType)
    {
    case BRACKET_SINGLE_ELIM:
      upResult = genBracket__SingleElim(numPlayers);
      break;
    case BRACKET_RANKING1:
      upResult = genBracket__Ranking1(numPlayers);
      break;
    default:
      throw std::runtime_error("TODO: Unimplemented bracket type!");
    }

    // convert unique_ptrs to standard objects that are easier to handle
    BracketMatchDataList result;
    for_each(upResult.begin(), upResult.end(), [&result](upBracketMatchData& b){result.append(*b);});

    return result;
  }

//----------------------------------------------------------------------------

  void BracketGenerator::removeUnusedMatches(upBracketMatchDataVector &bracketMatches, int numPlayers) const
  {
    // sort the bracket matches so that we always traverse the tree "from left to right" (read: from the
    // earlier to the later matches)
    std::sort(bracketMatches.begin(), bracketMatches.end(), getBracketMatchSortFunction_up_earlyRoundsFirst());

    // since I have some trouble with std::sort() (see below), I put in another safeguard
    // that all bracket matches are sorted properly
    int nMatches = bracketMatches.size();
    for (int i=0; i < (nMatches-1); ++i)
    {
      // the depth of a match must be greater or equal to the depth of the following
      // match. Remember: macthes with higher depth values are played first
      assert(bracketMatches[i]->depthInBracket >= bracketMatches[i+1]->depthInBracket);
    }
    // the last match must be at depth 0
    assert(bracketMatches[nMatches-1]->depthInBracket == 0);

    // a little helper function that returns an iterator to a match with
    // a given ID
    auto getMatchById = [&bracketMatches](int matchId) {
      upBracketMatchDataVector::iterator i = bracketMatches.begin();
      while (i != bracketMatches.end())
      {
        if ((**i).getBracketMatchId() == matchId) return i;
        ++i;
      }
      return i;
    };

    // a little helper function that updates a player
    // in a bracket match pointed to by a match ID
    auto updatePlayer = [&](int matchId, int playerPos, int newVal) {
      auto iMatch = getMatchById(matchId);
      if (iMatch == bracketMatches.end()) return; // invalid ID

      if (playerPos == 1)
      {
        (*iMatch)->initialRank_Player1 = newVal;
      } else {
        (*iMatch)->initialRank_Player2 = newVal;
      }
    };


    // traverse the tree again and again, until we find no more changes to make
    bool matchesChanged = true;
    while (matchesChanged)
    {
      matchesChanged = false;

      // first step: flag all matches with initial ranks > numPlayers for BOTH players
      // as "to be deleted"
      upBracketMatchDataVector::iterator i = bracketMatches.begin();
      while (i != bracketMatches.end())
      {
        upBracketMatchData& bmd = *i;
        if ((bmd->initialRank_Player1 > numPlayers) && (bmd->initialRank_Player2 > numPlayers))
        {
          if (bmd->nextMatchForWinner > 0)
          {
            updatePlayer(bmd->nextMatchForWinner, bmd->nextMatchPlayerPosForWinner, BracketMatchData::UNUSED_PLAYER);
          }
          if (bmd->nextMatchForLoser > 0)
          {
            updatePlayer(bmd->nextMatchForLoser, bmd->nextMatchPlayerPosForLoser, BracketMatchData::UNUSED_PLAYER);
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
        upBracketMatchData& bmd = *i;
        if (bmd->initialRank_Player1 > numPlayers)
        {
          // player 1 does not exist, this means that player 2 wins automatically;
          // find the match the winner will be promoted to
          if (bmd->nextMatchForWinner > 0)
          {
            updatePlayer(bmd->nextMatchForWinner, bmd->nextMatchPlayerPosForWinner, bmd->initialRank_Player2);

            // if player2 of "bmd" is not a directly seeded, initial player
            // but the winner/loser of a previous match, we need to update
            // that previous match, too
            if (bmd->initialRank_Player2 < 0)
            {
              int prevMatchId = -(bmd->initialRank_Player2);
              auto prevMatch = getMatchById(prevMatchId);
              auto nextMatch = getMatchById(bmd->nextMatchForWinner);
              if ((*prevMatch)->nextMatchForWinner == bmd->getBracketMatchId())
              {
                (*prevMatch)->setNextMatchForWinner(**nextMatch, bmd->nextMatchPlayerPosForWinner);
              } else {
                (*prevMatch)->setNextMatchForLoser(**nextMatch, bmd->nextMatchPlayerPosForWinner);
              }
            }
          }

          // player 1 does not exist and player 2 wins automatically
          // ==> we have no loser! If we promote the (non-existing) loser to a next match
          // we need to update that match, too
          if (bmd->nextMatchForLoser > 0)
          {
            updatePlayer(bmd->nextMatchForLoser, bmd->nextMatchPlayerPosForLoser, BracketMatchData::UNUSED_PLAYER);
          }

          // we may only delete this match if the winner does not achieve a final rank.
          // Otherwise we would lose this ranking information.
          if (bmd->nextMatchForWinner >= 0)
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

        if (bmd->initialRank_Player2 > numPlayers)
        {
          // player 2 does not exist, this means that player 1 wins automatically;
          // find the match the winner will be promoted to
          if (bmd->nextMatchForWinner > 0)
          {
            updatePlayer(bmd->nextMatchForWinner, bmd->nextMatchPlayerPosForWinner, bmd->initialRank_Player1);

            // if player1 of "bmd" is not a directly seeded, initial player
            // but the winner/loser of a previous match, we need to update
            // that previous match, too
            if (bmd->initialRank_Player1 < 0)
            {
              int prevMatchId = -(bmd->initialRank_Player1);
              auto prevMatch = getMatchById(prevMatchId);
              auto nextMatch = getMatchById(bmd->nextMatchForWinner);
              if ((*prevMatch)->nextMatchForWinner == bmd->getBracketMatchId())
              {
                (*prevMatch)->setNextMatchForWinner(**nextMatch, bmd->nextMatchPlayerPosForWinner);
              } else {
                (*prevMatch)->setNextMatchForLoser(**nextMatch, bmd->nextMatchPlayerPosForWinner);
              }
            }
          }

          // player 2 does not exist and player 1 wins automatically
          // ==> we have no loser! If we promote the (non-existing) loser to a next match
          // we need to update that match, too
          if (bmd->nextMatchForLoser > 0)
          {
            updatePlayer(bmd->nextMatchForLoser, bmd->nextMatchPlayerPosForLoser, BracketMatchData::UNUSED_PLAYER);
          }

          // we may only delete this match if the winner does not achieve a final rank.
          // Otherwise we would lose this ranking information.
          if (bmd->nextMatchForWinner >= 0)
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
      // initial note:
      // std::sort now seems to provide invalid bmd2 reference that
      // cause a SIGSEGV. Using the debugger I could trace it down to
      // stl_algo.h using an invalid value for the last element of the
      // vector, although std:sort was called with a correct value.
      // std::sort seems to go beyond the last element and that causes
      // a SIGSEGV.
      //
      // weird.
      //
      // changing the compiler or the optimization settings didn't help.
      //
      // as workaround, i test the validity of bmd1 and bmd2 first
      int depth1;
      int depth2;
      try
      {
        // this is madness: being a reference, bmd1 or bmd2 can't be null
        // by definition! however, i encountered bmd1 being a null reference
        // during debugging
        if (&bmd1 == nullptr) return false;
        if (&bmd2 == nullptr) return false;
        depth1 = bmd1.depthInBracket;
        depth2 = bmd2.depthInBracket;
      } catch (std::exception& e)
      {
        return false;    // invalid pointer, return some arbitrary value
      }

      // if matches are at the same depth level,
      // than matches with end in a final rank should be played
      // later.
      //
      // if both matches result in a final rank, the numerically lower
      // rank should be played later


      if (depth1 == depth2)
      {
        int rank1 = bmd1.nextMatchForWinner;
        int rank2 = bmd2.nextMatchForWinner;
        if ((rank1 < 0) && (rank2 > 0))
        {
          // only match 1 results in a final rank,
          // so play match 2 first
          return false;
        }
        if ((rank1 > 0) && (rank2 < 0))
        {
          // only match 2 results in a final rank,
          // so play match 1 first
          return true;
        }
        if ((rank1 < 0) && (rank2 < 0))
        {
          // if rank1 is higher (e.g. -1 = rank 1) then
          // play match 1 later
          return rank1 < rank2;
        }

        // no match ends in a final rank, order doesn't matter
        return true;
      }

      // if we made it to this point, we can be sure
      // that the matches are at different depths, so
      // the depth is the only sorting criteria
      return depth1 > depth2;
    };
  }


//----------------------------------------------------------------------------

    std::function<bool (upBracketMatchData&, upBracketMatchData&)> BracketGenerator::getBracketMatchSortFunction_up_earlyRoundsFirst()
    {
      return [](upBracketMatchData& bmd1, upBracketMatchData& bmd2) {
        // initial note:
        // std::sort now seems to provide invalid bmd2 reference that
        // cause a SIGSEGV. Using the debugger I could trace it down to
        // stl_algo.h using an invalid value for the last element of the
        // vector, although std:sort was called with a correct value.
        // std::sort seems to go beyond the last element and that causes
        // a SIGSEGV.
        //
        // weird.
        //
        // changing the compiler or the optimization settings didn't help.
        //
        // as a workaround, i test the validity of bmd1 and bmd2 first
        int depth1;
        int depth2;
        try
        {
          if (bmd1 == nullptr) return false;
          if (bmd2 == nullptr) return false;
          uint64_t p1 = (uint64_t)(bmd1.get());
          if (p1 < 0x1000) return false;
          uint64_t p2 = (uint64_t)(bmd2.get());
          if (p2 < 0x1000) return false;
          depth1 = bmd1->depthInBracket;
          depth2 = bmd2->depthInBracket;
        } catch (std::exception& e)
        {
          return false;    // invalid pointer, return some arbitrary value
        }

        // if matches are at the same depth level,
        // than matches with end in a final rank should be played
        // later.
        //
        // if both matches result in a final rank, the numerically lower
        // rank should be played later

        if (depth1 == depth2)
        {
          int rank1 = bmd1->nextMatchForWinner;
          int rank2 = bmd2->nextMatchForWinner;
          if ((rank1 < 0) && (rank2 > 0))
          {
            // only match 1 results in a final rank,
            // so play match 2 first
            return false;
          }
          if ((rank1 > 0) && (rank2 < 0))
          {
            // only match 2 results in a final rank,
            // so play match 1 first
            return true;
          }
          if ((rank1 < 0) && (rank2 < 0))
          {
            // if rank1 is higher (e.g. -1 = rank 1) then
            // play match 1 later
            return rank1 < rank2;
          }

          // no match ends in a final rank, order doesn't matter
          return true;
        }

        // if we made it to this point, we can be sure
        // that the matches are at different depths, so
        // the depth is the only sorting criteria
        return depth1 > depth2;
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
    if (bracketType != BracketGenerator::BRACKET_RANKING1)
    {
      int nRounds = 1;
      int n = 2;
      while (n < numPlayers)
      {
        n = n * 2;
        ++nRounds;
      }
      return nRounds;
    }
    if (bracketType == BracketGenerator::BRACKET_RANKING1)
    {
      // hard-coded values RANKING1
      if (numPlayers > 8) return 5;
      if (numPlayers > 4) return 3;
      if (numPlayers > 2) return 2;
      return 1;
    }

    return -1;   // shouldn't happen
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
    return bracketMatchId;
  }

//----------------------------------------------------------------------------

  void BracketMatchData::setInitialRanks(int initialRank_P1, int initialRank_P2)
  {
    initialRank_Player1 = initialRank_P1;
    initialRank_Player2 = initialRank_P2;
  }

//----------------------------------------------------------------------------

  void BracketMatchData::setNextMatchForWinner(BracketMatchData &nextBracketMatch, int posInNextMatch)
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

  void BracketMatchData::setNextMatchForLoser(
      BracketMatchData &nextBracketMatch, int posInNextMatch)
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

  void BracketMatchData::dumpOnScreen()
  {
    qDebug() << "Bracket Match ID = " << bracketMatchId;
    qDebug() << "  Depth        = " << depthInBracket;
    qDebug() << "  I1           = " << initialRank_Player1;
    qDebug() << "  I2           = " << initialRank_Player2;
    qDebug() << "  Winner to    = " << nextMatchForWinner << "." << nextMatchPlayerPosForWinner;
    qDebug() << "  Loser to     = " << nextMatchForLoser << "." << nextMatchPlayerPosForLoser;
    qDebug();
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

}
