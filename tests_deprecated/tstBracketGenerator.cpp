/* 
 * File:   tstGenericDBObject.cpp
 * Author: volker
 * 
 * Created on March 2, 2014, 3:46 PM
 */

#include <QString>
#include "tstBracketGenerator.h"
#include "BracketGenerator.h"


using namespace QTournament;

//----------------------------------------------------------------------------

void tstBracketGenerator::testSingleElim()
{
  constexpr int UPPER_PLAYER_LIMIT = 32;

  printStartMsg("tstBracketGenerator::testSingleElim");
  
  BracketGenerator bg{BracketGenerator::BRACKET_MatchSystem::SingleElim};

  // Test all brackets for 2 to 32 players
  for (int numPlayers=2; numPlayers <= UPPER_PLAYER_LIMIT; ++numPlayers)
  {
    BracketMatchDataList bmdl = bg.getBracketMatches(numPlayers);
    CPPUNIT_ASSERT(bmdl.size() > 0);

    // Test 1: we may only have as many initialRanks as we have players
    bool usedRanks[UPPER_PLAYER_LIMIT];
    for (int i=0; i < UPPER_PLAYER_LIMIT; ++i) usedRanks[i] = false;
    int usedRanksCount = 0;
    for (BracketMatchData bmd : bmdl)
    {
      if (bmd.initialRank_Player1 > 0)
      {
        int r = bmd.initialRank_Player1;
        if (usedRanks[r-1])
        {
            qDebug() << "Initial rank " << r << " used twice!";
            CPPUNIT_ASSERT(false);
        }
        CPPUNIT_ASSERT(!(usedRanks[r-1]));  // the rank may not have been used before
        usedRanks[r-1] = true;
        ++usedRanksCount;
        CPPUNIT_ASSERT((r > 0) && (r <= numPlayers));
      }
      if (bmd.initialRank_Player2 > 0)
      {
        int r = bmd.initialRank_Player2;
        if (usedRanks[r-1])
        {
            qDebug() << "Initial rank " << r << " used twice!";
            CPPUNIT_ASSERT(false);
        }
        CPPUNIT_ASSERT(!(usedRanks[r-1]));  // the rank may not have been used before
        usedRanks[r-1] = true;
        ++usedRanksCount;
        CPPUNIT_ASSERT((r > 0) && (r <= numPlayers));
      }
    }
    CPPUNIT_ASSERT(usedRanksCount == numPlayers);

    // Test 2: all losers must be knocked out
    // and all ranks must be zero, except for the finals
    for (BracketMatchData bmd : bmdl)
    {
      if (bmd.depthInBracket == 0)   // finals
      {
        CPPUNIT_ASSERT(bmd.nextMatchForWinner == -1);
        CPPUNIT_ASSERT(bmd.nextMatchForLoser == -2);
        continue;
      }
      CPPUNIT_ASSERT(bmd.nextMatchForWinner > 0);
      CPPUNIT_ASSERT(bmd.nextMatchForLoser == 0);
    }

    // Test 3: make sure that the internal linking is consistent
    auto getMatchById = [&bmdl](int matchId) {
      BracketMatchDataList::iterator i = bmdl.begin();
      while (i != bmdl.end())
      {
        if ((*i).getBracketMatchId() == matchId) return *i;
        ++i;
      }
    };
    for (BracketMatchData bmd : bmdl)
    {
      if (bmd.initialRank_Player1 < 0)
      {
        int fromMatchId = -(bmd.initialRank_Player1);
        BracketMatchData fromMatch = getMatchById(fromMatchId);
        bool foundLink1 = false;
        foundLink1 = ((fromMatch.nextMatchForWinner == bmd.getBracketMatchId()) && (fromMatch.nextMatchPlayerPosForWinner == 1));
        bool foundLink2 = false;
        foundLink2 = ((fromMatch.nextMatchForLoser == bmd.getBracketMatchId()) && (fromMatch.nextMatchPlayerPosForLoser == 1));

        // only either one can be true, not both
        CPPUNIT_ASSERT(!(foundLink1 && foundLink2));

        // one of both MUST be true
        CPPUNIT_ASSERT(foundLink1 || foundLink2);

        // the target match must always be in a lower depth (higher round) that the source match
        CPPUNIT_ASSERT(fromMatch.depthInBracket > bmd.depthInBracket);
      }
      if (bmd.initialRank_Player2 < 0)
      {
        int fromMatchId = -(bmd.initialRank_Player2);
        BracketMatchData fromMatch = getMatchById(fromMatchId);
        bool foundLink1 = false;
        foundLink1 = ((fromMatch.nextMatchForWinner == bmd.getBracketMatchId()) && (fromMatch.nextMatchPlayerPosForWinner == 2));
        bool foundLink2 = false;
        foundLink2 = ((fromMatch.nextMatchForLoser == bmd.getBracketMatchId()) && (fromMatch.nextMatchPlayerPosForLoser == 2));

        // only either one can be true, not both
        CPPUNIT_ASSERT(!(foundLink1 && foundLink2));

        // one of both MUST be true
        CPPUNIT_ASSERT(foundLink1 || foundLink2);

        // the target match must always be in a lower depth (higher round) that the source match
        CPPUNIT_ASSERT(fromMatch.depthInBracket > bmd.depthInBracket);
      }
    }

    // Test 4: in single elimination brackets, we may not have any orphaned
    // players pairs set to UNUSED_PLAYER
    for (BracketMatchData bmd : bmdl)
    {
      CPPUNIT_ASSERT(bmd.initialRank_Player1 != BracketMatchData::UNUSED_PLAYER);
      CPPUNIT_ASSERT(bmd.initialRank_Player2 != BracketMatchData::UNUSED_PLAYER);
    }

    // Test 5: initial ranks may not occur later than "maxDepth-1" (read: second round)
    int maxDepth = bg.getNumRounds(numPlayers) - 1;
    CPPUNIT_ASSERT(maxDepth >= 0);
    for (BracketMatchData bmd : bmdl)
    {
      if (bmd.initialRank_Player1 > 0)
      {
        CPPUNIT_ASSERT(bmd.depthInBracket >= (maxDepth-1));
      }
      if (bmd.initialRank_Player2 > 0)
      {
        CPPUNIT_ASSERT(bmd.depthInBracket >= (maxDepth-1));
      }

      // check for valid depths, by the way
      CPPUNIT_ASSERT(bmd.depthInBracket >= 0);
    }
  }
  printEndMsg();
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

