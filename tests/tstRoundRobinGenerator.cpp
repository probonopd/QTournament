/* 
 * File:   tstGenericDBObject.cpp
 * Author: volker
 * 
 * Created on March 2, 2014, 3:46 PM
 */

#include "tstRoundRobinGenerator.h"
#include "RoundRobinGenerator.h"


using namespace QTournament;

//----------------------------------------------------------------------------

void tstRoundRobinGenerator::testMatchCreation()
{
  printStartMsg("tstRoundRobinGenerator::testMatchCreation");
  
  constexpr int maxNumPlayers = 10;
  bool matchMatrix[maxNumPlayers][maxNumPlayers];
  bool hasBye[maxNumPlayers];
  bool usedInThisRound[maxNumPlayers];

  // Brute-force test of all group sizes between 2 and 10 players
  RoundRobinGenerator rrg;
  for (int grpSize = 2; grpSize <= maxNumPlayers; ++grpSize)
  {
    // calculate the number of rounds we should expect for this group size
    bool isOdd = (grpSize % 2);
    int numRounds = isOdd ? grpSize : grpSize - 1;

    for (int i=0; i < maxNumPlayers; ++i)
    {
      for (int j=0; j < maxNumPlayers; ++j) matchMatrix[i][j] = false;
      matchMatrix[i][i] = true;
      hasBye[i] = false;
    }

    for (int round=0; round < maxNumPlayers; ++round)
    {
      for (int i=0; i < maxNumPlayers; ++i) usedInThisRound[i] = false;

      // Call the generator, the object under test
      auto matches = rrg(grpSize, round);

      // check the correct number of returned matches
      if (round < numRounds)
      {
        if (isOdd)
        {
          CPPUNIT_ASSERT(matches.size() == (grpSize-1) / 2);
        } else {
          CPPUNIT_ASSERT(matches.size() == grpSize / 2);
        }
      } else {
        CPPUNIT_ASSERT(matches.size() == 0);
        continue;
      }

      for (auto match : matches)
      {
        int p1 = get<0>(match);
        int p2 = get<1>(match);

        // make sure the two players are not identical
        // and within a valid range
        CPPUNIT_ASSERT(p1 != p2);
        CPPUNIT_ASSERT((p1 >= 0) && (p1 < grpSize));
        CPPUNIT_ASSERT((p2 >= 0) && (p2 < grpSize));

        // ensure that the players have not been used before in this round
        CPPUNIT_ASSERT(usedInThisRound[p1] == false);
        CPPUNIT_ASSERT(usedInThisRound[p2] == false);
        usedInThisRound[p1] = true;
        usedInThisRound[p2] = true;

        // ensure that this match has not been generated before
        CPPUNIT_ASSERT(matchMatrix[p1][p2] == false);
        CPPUNIT_ASSERT(matchMatrix[p2][p1] == false);
        matchMatrix[p1][p2] = true;
        matchMatrix[p2][p1] = true;
      }

      // check how many players had a bye in this round
      int numBye = 0;
      for (int i=0; i < grpSize; ++i) if (!usedInThisRound[i]) ++numBye;
      if (isOdd) CPPUNIT_ASSERT(numBye == 1);
      else CPPUNIT_ASSERT(numBye == 0);

      // tag the player who had a bye
      // and make sure that every player has only one bye
      if (isOdd)
      {
        int byePlayer = -1;
        for (int i=0; i < grpSize; ++i) if (!(usedInThisRound[i])) byePlayer = i;
        CPPUNIT_ASSERT(byePlayer >= 0);
        CPPUNIT_ASSERT(hasBye[byePlayer] == false);
        hasBye[byePlayer] = true;
      }
    }

    // now that all rounds have been played, check that every
    // combination of players has been called
    for (int i=0; i < grpSize; ++i)
    {
      for (int j=0; j < grpSize; ++j)
      {
        CPPUNIT_ASSERT(matchMatrix[i][j] == true);
      }
    }

    // for odd group sizes, every player should have had a bye
    if (isOdd)
    {
      for (int i=0; i < grpSize; ++i) CPPUNIT_ASSERT(hasBye[i] == true);
    }
  }

  // test some border cases
  CPPUNIT_ASSERT(rrg(-1, 0).size() == 0);  // invalid number of players
  CPPUNIT_ASSERT(rrg(0, 0).size() == 0);  // invalid number of players
  CPPUNIT_ASSERT(rrg(1, 0).size() == 0);  // invalid number of players
  CPPUNIT_ASSERT(rrg(4, -1).size() == 0);  // negative round number

  printEndMsg();
}

//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    
