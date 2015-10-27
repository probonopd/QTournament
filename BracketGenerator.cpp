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

  tuple<upBracketMatchDataVector, RawBracketVisDataDef> BracketGenerator::genBracket__SingleElim(int numPlayers) const
  {
    upBracketMatchDataVector result;
    RawBracketVisDataDef visResult;

    // return an empty list in case of invalid arguments
    if (numPlayers < 2)
    {
      return make_tuple(upBracketMatchDataVector(), visResult);
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

    return make_tuple(std::move(result), visResult);
  }

//----------------------------------------------------------------------------

  tuple<upBracketMatchDataVector, RawBracketVisDataDef> BracketGenerator::genBracket__Ranking1(int numPlayers) const
  {
    upBracketMatchDataVector result;
    RawBracketVisDataDef visResult;

    // return an empty list in case of invalid arguments
    if ((numPlayers < 2) || (numPlayers > 32))
    {
      return make_tuple(upBracketMatchDataVector(), visResult);
    }

    BracketMatchData::resetBracketMatchId();

    // hard-code the bracket matches according to a
    // given tournament bracket
    int rawBracketData_16[36][7] =
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

    int bracketVisData_16[36][9] =
    {
      // page, grid x0, grid y0, y-span, yPageBreakSpan, nextPage, orientation, terminator, terminatorOffsetY      || orientation: -1= left, 1=right; terminator: 1=outwards, -1=inwards
      {0, 5, 0, 2, 0, 0, 1, 0, 0},    // Match 1
      {0, 5, 4, 2, 0, 0, 1, 0, 0},    // Match 2
      {0, 5, 8, 2, 0, 0, 1, 0, 0},    // Match 3
      {0, 5, 12, 2, 0, 0, 1, 0, 0},   // Match 4
      {0, 5, 16, 2, 0, 0, 1, 0, 0},   // Match 5
      {0, 5, 20, 2, 0, 0, 1, 0, 0},   // Match 6
      {0, 5, 24, 2, 0, 0, 1, 0, 0},   // Match 7
      {0, 5, 28, 2, 0, 0, 1, 0, 0},   // Match 8

      {0, 5, 1, 4, 0, 0, -1, 0, 0},   // Match 9
      {0, 6, 1, 4, 0, 0, 1, 0, 0},    // Match 10
      {0, 5, 9, 4, 0, 0, -1, 0, 0},   // Match 11
      {0, 6, 9, 4, 0, 0, 1, 0, 0},    // Match 12
      {0, 5, 17, 4, 0, 0, -1, 0, 0},  // Match 13
      {0, 6, 17, 4, 0, 0, 1, 0, 0},   // Match 14
      {0, 5, 25, 4, 0, 0, -1, 0, 0},  // Match 15
      {0, 6, 25, 4, 0, 0, 1, 0, 0},   // Match 16

      {0, 4, 3, 4, 0, 0, -1, 0, 0},   // Match 17
      {0, 4, 11, 4, 0, 0, -1, 0, 0},  // Match 18
      {0, 4, 19, 4, 0, 0, -1, 0, 0},  // Match 19
      {0, 4, 27, 4, 0, 0, -1, 0, 0},  // Match 20

      {0, 8, 34, 2, 0, 0, 1, 0, 0},   // Match 21
      {0, 8, 38, 2, 0, 0, 1, 0, 0},   // Match 22
      {0, 2, 34, 2, 0, 0, 1, 0, 0},   // Match 23
      {0, 2, 38, 2, 0, 0, 1, 0, 0},   // Match 24
      {0, 3, 4, 8, 0, 0, -1, 0, 0},   // Match 25
      {0, 3, 20, 8, 0, 0, -1, 0, 0},  // Match 26
      {0, 7, 3, 8, 0, 0, 1, 0, 0},    // Match 27
      {0, 7, 19, 8, 0, 0, 1, 0, 0},   // Match 28

      {0, 8, 35, 4, 0, 0, -1, 1, 0},  // Match 29
      {0, 9, 35, 4, 0, 0, 1, 1, 0},   // Match 30
      {0, 2, 35, 4, 0, 0, -1, 1, 0},  // Match 31
      {0, 3, 35, 4, 0, 0, 1, 1, 0},   // Match 32
      {0, 2, 30, 2, 0, 0, -1, 1, 0},  // Match 33
      {0, 2, 8, 16, 0, 0, -1, 1, 0},  // Match 34
      {0, 9, 29, 2, 0, 0, 1, 1, 0},   // Match 35
      {0, 8, 7, 16, 0, 0, 1, 1, 0},   // Match 36
    };

    int rawBracketData_32[92][7] =
    {
      // initialRank1, initialRank2, nextMatchWinner, nextMatchLoser, posWinner, posLoser, depth
      {1,32,18,17,1,1,6},  // Match 1
      {16,17,18,17,2,2,6},  // Match 2
      {8,25,20,19,1,1,6},  // Match 3
      {9,24,20,19,2,2,6},  // Match 4
      {4,29,22,21,1,1,6},  // Match 5
      {13,20,22,21,2,2,6},  // Match 6
      {5,28,24,23,1,1,6},  // Match 7
      {12,21,24,23,2,2,6},  // Match 8
      {22,11,26,25,1,1,6},  // Match 9
      {27,6,26,25,2,2,6},  // Match 10
      {19,14,28,27,1,1,6},  // Match 11
      {30,3,28,27,2,2,6},  // Match 12
      {23,10,30,29,1,1,6},  // Match 13
      {26,7,30,29,2,2,6},  // Match 14
      {18,15,32,31,1,1,6},  // Match 15
      {31,2,32,31,2,2,6},  // Match 16
      {-1,-2,33,45,1,1,5},  // Match 17
      {-1,-2,34,36,1,2,5},  // Match 18
      {-3,-4,35,45,1,2,5},  // Match 19
      {-3,-4,34,38,2,2,5},  // Match 20
      {-5,-6,36,46,1,1,5},  // Match 21
      {-5,-6,37,33,1,2,5},  // Match 22
      {-7,-8,38,46,1,2,5},  // Match 23
      {-7,-8,37,35,2,2,5},  // Match 24
      {-9,-10,39,47,1,1,5},  // Match 25
      {-9,-10,40,42,1,2,5},  // Match 26
      {-11,-12,41,47,1,2,5},  // Match 27
      {-11,-12,40,44,2,2,5},  // Match 28
      {-13,-14,42,48,1,1,5},  // Match 29
      {-13,-14,43,41,1,2,5},  // Match 30
      {-15,-16,44,48,1,2,5},  // Match 31
      {-15,-16,43,39,2,2,5},  // Match 32
      {-17,-22,53,49,1,1,4},  // Match 33
      {-18,-20,57,65,1,2,4},  // Match 34
      {-19,-24,53,49,2,2,4},  // Match 35
      {-21,-18,54,50,1,1,4},  // Match 36
      {-22,-24,57,66,2,2,4},  // Match 37
      {-23,-20,54,50,2,2,4},  // Match 38
      {-25,-32,55,51,1,1,4},  // Match 39
      {-26,-28,58,63,1,2,4},  // Match 40
      {-27,-30,55,51,2,2,4},  // Match 41
      {-29,-26,56,52,1,1,4},  // Match 42
      {-30,-32,58,64,2,2,4},  // Match 43
      {-31,-28,56,52,2,2,4},  // Match 44
      {-17,-19,60,59,1,1,4},  // Match 45
      {-21,-23,60,59,2,2,4},  // Match 46
      {-25,-27,62,61,1,1,4},  // Match 47
      {-29,-31,62,61,2,2,4},  // Match 48
      {-33,-35,68,67,1,1,3},  // Match 49
      {-36,-38,68,67,2,2,3},  // Match 50
      {-39,-41,70,69,1,1,3},  // Match 51
      {-42,-44,70,69,2,2,3},  // Match 52
      {-33,-35,63,71,1,1,3},  // Match 53
      {-36,-38,64,71,1,2,3},  // Match 54
      {-39,-41,65,72,1,1,3},  // Match 55
      {-42,-44,66,72,1,2,3},  // Match 56
      {-34,-37,92,91,1,1,2},  // Match 57
      {-40,-43,92,91,2,2,2},  // Match 58
      {-45,-46,78,77,1,1,2},  // Match 59
      {-45,-46,80,79,1,1,2},  // Match 60
      {-47,-48,78,77,2,2,2},  // Match 61
      {-47,-48,80,79,2,2,2},  // Match 62
      {-53,-40,75,73,1,1,2},  // Match 63
      {-54,-43,75,73,2,2,2},  // Match 64
      {-55,-34,76,74,1,1,2},  // Match 65
      {-56,-37,76,74,2,2,2},  // Match 66
      {-49,-50,82,81,1,1,1},  // Match 67
      {-49,-50,84,83,1,1,1},  // Match 68
      {-51,-52,82,81,2,2,1},  // Match 69
      {-51,-52,84,83,2,2,1},  // Match 70
      {-53,-54,86,85,1,1,1},  // Match 71
      {-55,-56,86,85,2,2,1},  // Match 72
      {-63,-64,88,87,1,1,1},  // Match 73
      {-65,-66,88,87,2,2,1},  // Match 74
      {-63,-64,90,89,1,1,1},  // Match 75
      {-65,-66,90,89,2,2,1},  // Match 76
      {-59,-61,-31,-32,0,0,0},  // Match 77
      {-59,-61,-29,-30,0,0,0},  // Match 78
      {-60,-62,-27,-28,0,0,0},  // Match 79
      {-60,-62,-25,-26,0,0,0},  // Match 80
      {-67,-69,-23,-24,0,0,0},  // Match 81
      {-67,-69,-21,-22,0,0,0},  // Match 82
      {-68,-70,-19,-20,0,0,0},  // Match 83
      {-68,-70,-17,-18,0,0,0},  // Match 84
      {-71,-72,-15,-16,0,0,0},  // Match 85
      {-71,-72,-13,-14,0,0,0},  // Match 86
      {-73,-74,-11,-12,0,0,0},  // Match 87
      {-73,-74,-9,-10,0,0,0},  // Match 88
      {-75,-76,-7,-8,0,0,0},  // Match 89
      {-75,-76,-5,-6,0,0,0},  // Match 90
      {-57,-58,-3,-4,0,0,0},  // Match 91
      {-57,-58,-1,-2,0,0,0},  // Match 92
    };

    // convert the hard-coded data into bracket match data entries
    if (numPlayers <= 16)
    {
      // the bracket matches as such
      for (int i=0; i < 36; ++i)
      {
        upBracketMatchData newBracketMatch = upBracketMatchData(new BracketMatchData);

        newBracketMatch->initialRank_Player1 = rawBracketData_16[i][0];
        newBracketMatch->initialRank_Player2 = rawBracketData_16[i][1];
        newBracketMatch->nextMatchForWinner = rawBracketData_16[i][2];
        newBracketMatch->nextMatchForLoser = rawBracketData_16[i][3];
        newBracketMatch->nextMatchPlayerPosForWinner = rawBracketData_16[i][4];
        newBracketMatch->nextMatchPlayerPosForLoser = rawBracketData_16[i][5];
        newBracketMatch->depthInBracket = rawBracketData_16[i][6];

        result.push_back(std::move(newBracketMatch));
      }

      // the visualization data
      visResult.addPage(BRACKET_PAGE_ORIENTATION::LANDSCAPE, BRACKET_LABEL_POS::BOTTOM_LEFT);
      for (int i=0; i < 36; ++i)
      {
        RawBracketVisElement el{bracketVisData_16[i]};
        visResult.addElement(el);
      }

    } else {
      for (int i=0; i < 92; ++i)
      {
        upBracketMatchData newBracketMatch = upBracketMatchData(new BracketMatchData);

        newBracketMatch->initialRank_Player1 = rawBracketData_32[i][0];
        newBracketMatch->initialRank_Player2 = rawBracketData_32[i][1];
        newBracketMatch->nextMatchForWinner = rawBracketData_32[i][2];
        newBracketMatch->nextMatchForLoser = rawBracketData_32[i][3];
        newBracketMatch->nextMatchPlayerPosForWinner = rawBracketData_32[i][4];
        newBracketMatch->nextMatchPlayerPosForLoser = rawBracketData_32[i][5];
        newBracketMatch->depthInBracket = rawBracketData_32[i][6];

        result.push_back(std::move(newBracketMatch));
      }
    }

    removeUnusedMatches(result, numPlayers);

    return make_tuple(std::move(result), visResult);
  }


//----------------------------------------------------------------------------

  tuple<BracketMatchDataList, RawBracketVisDataDef> BracketGenerator::getBracketMatches(int numPlayers) const
  {
    if (numPlayers < 2) return make_tuple(BracketMatchDataList(), RawBracketVisDataDef());

    upBracketMatchDataVector upResult;
    RawBracketVisDataDef visResult;
    switch (bracketType)
    {
    case BRACKET_SINGLE_ELIM:
      tie(upResult, visResult) = genBracket__SingleElim(numPlayers);
      break;
    case BRACKET_RANKING1:
      tie(upResult, visResult) = genBracket__Ranking1(numPlayers);
      break;
    default:
      throw std::runtime_error("TODO: Unimplemented bracket type!");
    }

    // convert unique_ptrs to standard objects that are easier to handle
    BracketMatchDataList result;
    for_each(upResult.begin(), upResult.end(), [&result](upBracketMatchData& b){result.push_back(*b);});

    return make_tuple(result, visResult);
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

        // skip deleted matches
        if (bmd->matchDeleted)
        {
          ++i;
          continue;
        }

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
          // tag the match as deleted
          //
          // note: we may not actually delete the element from the match list because otherwise we
          // lose the visualization information
          //i = bracketMatches.erase(i);
          bmd->matchDeleted = true;
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

        // skip deleted matches
        if (bmd->matchDeleted)
        {
          ++i;
          continue;
        }

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
            // tag the match as deleted
            //
            // note: we may not actually delete the element from the match list because otherwise we
            // lose the visualization information
            //i = bracketMatches.erase(i);
            bmd->matchDeleted = true;
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
            matchesChanged = true;
          }

          // player 2 does not exist and player 1 wins automatically
          // ==> we have no loser! If we promote the (non-existing) loser to a next match
          // we need to update that match, too
          if (bmd->nextMatchForLoser > 0)
          {
            updatePlayer(bmd->nextMatchForLoser, bmd->nextMatchPlayerPosForLoser, BracketMatchData::UNUSED_PLAYER);
            matchesChanged = true;
          }

          // we may only delete this match if the winner does not achieve a final rank.
          // Otherwise we would lose this ranking information.
          if (bmd->nextMatchForWinner >= 0)
          {
            // tag the match as deleted
            //
            // note: we may not actually delete the element from the match list because otherwise we
            // lose the visualization information
            //i = bracketMatches.erase(i);
            bmd->matchDeleted = true;
          } else {
            ++i;
          }
          continue;
        }

        ++i;
      }

      // third step:
      // all matches that have one "unused player" and that have a final rank for the winner
      // transfer their final rank to the previous match. Example:
      // in match 42 we have the winner of #21 (player 1) and UNUSED_PLAYER (player 2). The
      // winner rank of match #42 is 9. So we change match #21 from "winner goes to #42" to "winner goes to
      // rank 9" and delete match #42
      i = bracketMatches.begin();
      while (i != bracketMatches.end())
      {
        upBracketMatchData& bmd = *i;

        // skip deleted matches
        if (bmd->matchDeleted)
        {
          ++i;
          continue;
        }

        if ((bmd->initialRank_Player1 == BracketMatchData::UNUSED_PLAYER) && (bmd->initialRank_Player2 < 0))
        {
          int prevMatchId = -(bmd->initialRank_Player2);
          auto prevMatch = getMatchById(prevMatchId);
          int winnerRank = bmd->nextMatchForWinner;
          assert(winnerRank < 0);   // must be true because of step 2 before
          if ((*prevMatch)->nextMatchForWinner == bmd->getBracketMatchId())
          {
            (*prevMatch)->nextMatchForWinner = winnerRank;
          } else {
            (*prevMatch)->nextMatchForLoser = winnerRank;
          }

          // tag the match as deleted
          //
          // note: we may not actually delete the element from the match list because otherwise we
          // lose the visualization information
          //i = bracketMatches.erase(i);
          bmd->matchDeleted = true;
          matchesChanged = true;
          continue;
        }
        if ((bmd->initialRank_Player2 == BracketMatchData::UNUSED_PLAYER) && (bmd->initialRank_Player1 < 0))
        {
          int prevMatchId = -(bmd->initialRank_Player1);
          assert(prevMatchId > 0);    // there should never be a final rank for a non-symbolic player
          auto prevMatch = getMatchById(prevMatchId);
          int winnerRank = bmd->nextMatchForWinner;
          assert(winnerRank < 0);   // must be true because of step 2 before
          if ((*prevMatch)->nextMatchForWinner == bmd->getBracketMatchId())
          {
            (*prevMatch)->nextMatchForWinner = winnerRank;
          } else {
            (*prevMatch)->nextMatchForLoser = winnerRank;
          }

          // tag the match as deleted
          //
          // note: we may not actually delete the element from the match list because otherwise we
          // lose the visualization information
          //i = bracketMatches.erase(i);
          bmd->matchDeleted = true;
          matchesChanged = true;
          continue;
        }
        ++i;
      }
    }

    // before we return we want to check that no match has "UNUSED_PLAYER" anymore
    upBracketMatchDataVector::iterator i = bracketMatches.begin();
    while (i != bracketMatches.end())
    {
      upBracketMatchData& bmd = *i;
      // skip deleted matches
      if (bmd->matchDeleted)
      {
        ++i;
        continue;
      }

      assert(bmd->initialRank_Player1 != BracketMatchData::UNUSED_PLAYER);
      assert(bmd->initialRank_Player2 != BracketMatchData::UNUSED_PLAYER);
      ++i;
    }

    // Done.
  }

//----------------------------------------------------------------------------

  std::function<bool (BracketMatchData&, BracketMatchData&)> BracketGenerator::getBracketMatchSortFunction_earlyRoundsFirst()
  {
    return [](const BracketMatchData& bmd1, const BracketMatchData& bmd2) {
      // if matches are at the same depth level,
      // than matches with end in a final rank should be played
      // later.
      //
      // if both matches result in a final rank, the numerically lower
      // rank should be played later


      if (bmd1.depthInBracket == bmd2.depthInBracket)
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
      return bmd1.depthInBracket > bmd2.depthInBracket;
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
      if (numPlayers > 16) return 7;
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

  int BracketMatchData::getBracketMatchId() const
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
