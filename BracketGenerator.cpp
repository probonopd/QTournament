#include "BracketGenerator.h"

BracketGenerator::BracketGenerator()
{
}

BracketMatchDataList BracketGenerator::genBracket__SingleElim(int numPlayers)
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

  return result;
}

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

bool BracketMatchData::setNextMatchForWinner(const BracketMatchData &nextBracketMatch, int posInNextMatch)
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

bool BracketMatchData::setNextMatchForLoser(const BracketMatchData &nextBracketMatch, int posInNextMatch)
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

