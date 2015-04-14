#ifndef BRACKETGENERATOR_H
#define BRACKETGENERATOR_H

#include <memory>
#include <functional>
#include <vector>

#include <QList>

#include "Tournament.h"

namespace QTournament
{

  class BracketMatchData
  {
  public:
    static constexpr int NO_INITIAL_RANK = 0;
    static constexpr int NO_NEXT_MATCH = 0;
    static constexpr int UNUSED_PLAYER = 999999;

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
    void setNextMatchForWinner(BracketMatchData& nextBracketMatch, int posInNextMatch);
    void setNextMatchForLoser(BracketMatchData& nextBracketMatch, int posInNextMatch);

    void dumpOnScreen();

  private:
    static int lastBracketMatchId;
    int bracketMatchId;
  };

  typedef unique_ptr<BracketMatchData> upBracketMatchData;
  typedef QList<BracketMatchData> BracketMatchDataList;
  typedef std::vector<upBracketMatchData> upBracketMatchDataVector;

  class BracketGenerator
  {
  public:
    static constexpr int BRACKET_SINGLE_ELIM = 1;
    static constexpr int BRACKET_DOUBLE_ELIM = 2;

    BracketGenerator();
    BracketGenerator(int type);

    BracketMatchDataList getBracketMatches(int numPlayers) const;
    static std::function<bool (BracketMatchData&, BracketMatchData&)> getBracketMatchSortFunction_earlyRoundsFirst();
    static std::function<bool (upBracketMatchData&, upBracketMatchData&)> getBracketMatchSortFunction_up_earlyRoundsFirst();
    int getNumRounds(int numPlayers) const;

  private:
    int bracketType;
    upBracketMatchDataVector genBracket__SingleElim(int numPlayers) const;
    void removeUnusedMatches(upBracketMatchDataVector& bracketMatches, int numPlayers) const;  // modifies the list IN PLACE!!
  };

}
#endif // BRACKETGENERATOR_H
