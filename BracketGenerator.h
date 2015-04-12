#ifndef BRACKETGENERATOR_H
#define BRACKETGENERATOR_H

#include <QList>

#include "Tournament.h"

class BracketMatchData
{
public:
  static constexpr int NO_INITIAL_RANK = 0;
  static constexpr int NO_NEXT_MATCH = 0;

  BracketMatchData();

  // the next two values are either...
  //   * > 0 if they indicate an initial rank in the lef-most part of the bracket; or
  //   * < 0 the if player comes from a previous match (e. g. "-5" for bracketMatchId 5)
  int initialRank_Player1;
  int initialRank_Player2;

  // the next two values are either...
  //   * > 0 if they refer to the bracketMatchId for the next Match; or
  //   * == 0 if the player drops out without any final rank
  //   * < 0 the if player reaches a final rank (e. g. "-5" for rank 5)
  int nextMatchForWinner;
  int nextMatchForLoser;

  int nextMatchPlayerPosForWinner;  // 1 or 2 for player 1 or player 2
  int nextMatchPlayerPosForLoser;   // 1 or 2 for player 1 or player 2

  int depthInBracket;  // 0 = finals, 1 = semifinals, 2 = quarterfinals, 3 = last 16, ...

  static void resetBracketMatchId();
  int getBracketMatchId();
  void setInitialRanks(int initialRank_P1, int initialRank_P2);
  bool setNextMatchForWinner(const BracketMatchData& nextBracketMatch, int posInNextMatch);
  bool setNextMatchForLoser(const BracketMatchData& nextBracketMatch, int posInNextMatch);

private:
  static int lastBracketMatchId = 0;
  int bracketMatchId;
};

typedef QList<BracketMatchData> BracketMatchDataList;

class BracketGenerator
{
public:
  BracketGenerator();

  static BracketMatchDataList genBracket__SingleElim(int numPlayers);
};

#endif // BRACKETGENERATOR_H
