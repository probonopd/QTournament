/* 
 * File:   tstGenericDBObject.cpp
 * Author: volker
 * 
 * Created on March 2, 2014, 3:46 PM
 */

#include <QString>
#include "tstScore.h"
#include "Score.h"


using namespace QTournament;

//----------------------------------------------------------------------------

void tstScore::testGameScore_IsValidScore()
{
  printStartMsg("tstScore::testGameScore_IsValidScore");

  int tstScore[][3] = {
      {-1, 10, 0},
      {21, -6, 0},
      {29, 31, 0},
      {32, 10, 0},
      {21, 21, 0},
      {12, 12, 0},
      {30, 29, 1},
      {29, 30, 1},
      {20, 21, 0},
      {21, 20, 0},
      {13, 21, 1},
      {21, 0, 1},
      {23, 21, 1},
      {28, 26, 1},
      {28, 3, 0},
  };

  for (auto score : tstScore)
  {
    bool expectedResult = (score[2] == 1);

    // test the static function isValidScore()
    CPPUNIT_ASSERT(GameScore::isValidScore(score[0], score[1]) == expectedResult);

    // test the factory function fromScore()
    auto g = GameScore::fromScore(score[0], score[1]);
    CPPUNIT_ASSERT((g == nullptr) == !expectedResult);
    if (expectedResult)
    {
      auto sc = g->getScore();
      CPPUNIT_ASSERT(get<0>(sc) == score[0]);
      CPPUNIT_ASSERT(get<1>(sc) == score[1]);
    }

    // test the factory function fromString()
    QString s = QString::number(score[0]) + ":" + QString::number(score[1]);
    g = GameScore::fromString(s);
    CPPUNIT_ASSERT((g == nullptr) == !expectedResult);
    if (expectedResult)
    {
      auto sc = g->getScore();
      CPPUNIT_ASSERT(get<0>(sc) == score[0]);
      CPPUNIT_ASSERT(get<1>(sc) == score[1]);
    }
  }
  
  printEndMsg();
}

//----------------------------------------------------------------------------

void tstScore::testGameScore_ToString()
{
  printStartMsg("tstScore::testGameScore_ToString");

  auto gs = GameScore::fromScore(12, 21);
  CPPUNIT_ASSERT(gs->toString() == "12:21");

  gs = GameScore::fromScore(21, 0);
  CPPUNIT_ASSERT(gs->toString() == "21:0");

  printEndMsg();
}

//----------------------------------------------------------------------------
    
void tstScore::testGameScore_GetWinner()
{
  printStartMsg("tstScore::testGameScore_GetWinner");

  int tstScore[][3] = {
      {30, 29, 1},
      {29, 30, 2},
      {13, 21, 2},
      {21, 0, 1},
      {23, 21, 1},
      {28, 26, 1},
  };

  for (auto score : tstScore)
  {
    auto gs = GameScore::fromScore(score[0], score[1]);
    CPPUNIT_ASSERT(gs->getWinner() == score[2]);

    int loser = (score[2] == 1) ? 2 : 1;
    CPPUNIT_ASSERT(gs->getLoser() == loser);
  }

  printEndMsg();
}

//----------------------------------------------------------------------------
//------------------MatchScore------------------------------------------------
//----------------------------------------------------------------------------

void tstScore::testMatchScore_FactoryFuncs_ToString()
{
  constexpr int MAX_NUM_GAMES = 5;

  printStartMsg("tstScore::testMatchScore_FactoryFuncs_ToString");

  int gameScore[][2] = {
      {30, 29},   // 0
      {29, 30},   // 1
      {13, 21},   // 2
      {21, 0},    // 3
      {21, 18},   // 4
      {23, 21},   // 5
      {28, 26},   // 6
  };

  int gameCombinations[][MAX_NUM_GAMES + 5] = {
      // game1, game2, game3, game4, game5, numWinGames, allowDraw, isValid, winner, loser
      //                                       +0      ,    +1    ,   +2   ,   +3  ,   +4
      {0, 1, -1, -1, -1, 2, 0, 0, -1, -1},   // invalid draw
      {0, 0, -1, -1, -1, 2, 0, 1, 1, 2},     // two games, player 1 wins
      {2, 4, 2, 2, -1, 2, 0, 0, -1, -1},     // four games but only two win games ==> invalid
      {2, 4, 2, 2, -1, 3, 0, 1, 2, 1},     // four games, three win games ==> valid
      {2, 4, 4, 2, 2, 3, 0, 1, 2, 1},     // five games, three win games ==> valid
      {2, 4, 4, -1, -1, 3, 0, 0, -1, -1},     // three games, three win games ==> invalid
      {2, 4, 4, -1, -1, 2, 0, 1, 1, 2},     // three games, two win games ==> valid


      {2, 4, 4, -1, -1, 3, 1, 0, -1, -1},     // three games, three win games, draw allowed ==> invalid
      {2, 4, -1, -1, -1, 2, 1, 1, 0, 0},     // two games, two win games, draw allowed ==> valid
      {2, 4, 2, 4, -1, 3, 1, 1, 0, 0},     // four games, three win games, draw allowed ==> valid
  };

  for (auto combi : gameCombinations)
  {
    GameScoreList gsl;
    QString scoreString;
    for (int gameCount = 0; gameCount < MAX_NUM_GAMES; ++gameCount)
    {
      if (combi[gameCount] != -1)
      {
        auto game = GameScore::fromScore(gameScore[combi[gameCount]][0], gameScore[combi[gameCount]][1]);
        CPPUNIT_ASSERT(game != nullptr);
        gsl.append(*game);
        scoreString += game->toString() + ",";
      }
    }
    scoreString = scoreString.left(scoreString.length() - 1);

    int numWinGames = combi[MAX_NUM_GAMES];
    bool allowDraw = (combi[MAX_NUM_GAMES+1] == 1);
    bool isValidMatch = (combi[MAX_NUM_GAMES+2] == 1);

    // test the factory function fromGameScoreList()
    auto match = MatchScore::fromGameScoreList(gsl, numWinGames, allowDraw);
    if (!isValidMatch)
    {
      CPPUNIT_ASSERT(match == nullptr);
      continue;
    }
    CPPUNIT_ASSERT(match != nullptr);

    // test the factory function fromString()
    auto match1 = MatchScore::fromString(scoreString, numWinGames, allowDraw);
    if (!isValidMatch)
    {
      CPPUNIT_ASSERT(match1 == nullptr);
      continue;
    }
    CPPUNIT_ASSERT(match1 != nullptr);

    // test the static isValidScore() function
    CPPUNIT_ASSERT(MatchScore::isValidScore(gsl, numWinGames, allowDraw) == isValidMatch);

    // test the toString() function
    CPPUNIT_ASSERT(match->toString() == scoreString);
    CPPUNIT_ASSERT(match1->toString() == scoreString);

  }


  printEndMsg();
}

//----------------------------------------------------------------------------

void tstScore::testMatchScore_GetWinner_GetLoser()
{
  constexpr int MAX_NUM_GAMES = 5;

  printStartMsg("tstScore::testMatchScore_GetWinner_GetLoser");

  int gameScore[][2] = {
      {30, 29},   // 0
      {29, 30},   // 1
      {13, 21},   // 2
      {21, 0},    // 3
      {21, 18},   // 4
      {23, 21},   // 5
      {28, 26},   // 6
  };

  int gameCombinations[][MAX_NUM_GAMES + 5] = {
      // game1, game2, game3, game4, game5, numWinGames, allowDraw, isValid, winner, loser
      //                                       +0      ,    +1    ,   +2   ,   +3  ,   +4
      {0, 0, -1, -1, -1, 2, 0, 1, 1, 2},     // two games, player 1 wins
      {2, 4, 2, 2, -1, 3, 0, 1, 2, 1},     // four games, three win games ==> valid
      {2, 4, 4, 2, 2, 3, 0, 1, 2, 1},     // five games, three win games ==> valid
      {2, 4, 4, -1, -1, 2, 0, 1, 1, 2},     // three games, two win games ==> valid

      {2, 4, -1, -1, -1, 2, 1, 1, 0, 0},     // two games, two win games, draw allowed ==> valid
      {2, 4, 2, 4, -1, 3, 1, 1, 0, 0},     // four games, three win games, draw allowed ==> valid
  };

  for (auto combi : gameCombinations)
  {
    GameScoreList gsl;
    for (int gameCount = 0; gameCount < MAX_NUM_GAMES; ++gameCount)
    {
      if (combi[gameCount] != -1)
      {
        auto game = GameScore::fromScore(gameScore[combi[gameCount]][0], gameScore[combi[gameCount]][1]);
        CPPUNIT_ASSERT(game != nullptr);
        gsl.append(*game);
      }
    }

    int numWinGames = combi[MAX_NUM_GAMES];
    bool allowDraw = (combi[MAX_NUM_GAMES+1] == 1);
    auto match = MatchScore::fromGameScoreList(gsl, numWinGames, allowDraw);
    CPPUNIT_ASSERT(match != nullptr);

    // test getWinner() and getLoser()
    CPPUNIT_ASSERT(match->getWinner() == combi[MAX_NUM_GAMES+3]);
    CPPUNIT_ASSERT(match->getLoser() == combi[MAX_NUM_GAMES+4]);
  }


  printEndMsg();
}

//----------------------------------------------------------------------------

void tstScore::testRandomMatchGeneration()
{
  constexpr int MATCH_COUNT = 1000;
  constexpr double PROBABILITY_MARGIN = 0.05;

  printStartMsg("tstScore::testRandomMatchGeneration");

  // a lambda as a match generator
  auto matchGenerator = [](int count, int numWinGames, bool drawAllowed) {
    MatchScoreList result;
    for (int i=0; i < count; ++i)
    {
      result.append(*(MatchScore::genRandomScore(numWinGames, drawAllowed)));
    }
    return result;
  };

  // a lambda to compare an actual count with an expected count
  // including a margin
  auto isCountInRange = [](int expected, int actual, double margin) {
    int absMargin = static_cast<int>(expected * margin);
    int minCount = expected - absMargin;
    int maxCount = expected + absMargin;
    return ((actual <= maxCount) && (actual >= minCount));
  };

  // Generate matches with 2 win games and no draw
  MatchScoreList msl = matchGenerator(MATCH_COUNT, 2, false);
  CPPUNIT_ASSERT(msl.count() == MATCH_COUNT);

  // gather some statistics
  int p1Wins = 0;
  int p2Wins = 0;
  int gamesCount = 0;
  int gamesBeyond21 = 0;
  for (MatchScore ms : msl)
  {
    if (ms.getWinner() == 1) ++p1Wins;
    if (ms.getWinner() == 2) ++p2Wins;

    int cnt = ms.getNumGames();
    for (int i=0; i < cnt; ++i)
    {
      ++gamesCount;
      auto gs = ms.getGame(i);
      if (gs->getWinnerScore() > 21) ++gamesBeyond21;
    }

    CPPUNIT_ASSERT(ms.isValidScore(2, false));
  }

  // make sure that wins are equally distributed among players
  CPPUNIT_ASSERT(isCountInRange(MATCH_COUNT / 2, p1Wins, PROBABILITY_MARGIN));
  CPPUNIT_ASSERT(isCountInRange(MATCH_COUNT / 2, p2Wins, PROBABILITY_MARGIN));
  CPPUNIT_ASSERT(isCountInRange(gamesCount * 0.3, gamesBeyond21, PROBABILITY_MARGIN));


  // Generate matches with 3 win games and draw
  msl.clear();
  msl = matchGenerator(MATCH_COUNT, 3, true);
  CPPUNIT_ASSERT(msl.count() == MATCH_COUNT);

  // gather some statistics
  p1Wins = 0;
  p2Wins = 0;
  int draws = 0;
  gamesCount = 0;
  gamesBeyond21 = 0;
  for (MatchScore ms : msl)
  {
    if (ms.getWinner() == 0) ++draws;
    if (ms.getWinner() == 1) ++p1Wins;
    if (ms.getWinner() == 2) ++p2Wins;

    int cnt = ms.getNumGames();
    for (int i=0; i < cnt; ++i)
    {
      ++gamesCount;
      auto gs = ms.getGame(i);
      if (gs->getWinnerScore() > 21) ++gamesBeyond21;
    }

    if (ms.getWinner() == 0)
    {
      CPPUNIT_ASSERT(ms.getNumGames() == 4);   // 4 = (numWinGames - 1) * 2
    }

    CPPUNIT_ASSERT(ms.isValidScore(3, true));
  }

  // make sure that wins are equally distributed among players
  CPPUNIT_ASSERT(isCountInRange(MATCH_COUNT * 0.3, draws, PROBABILITY_MARGIN));
  CPPUNIT_ASSERT(isCountInRange(MATCH_COUNT * 0.35, p1Wins, PROBABILITY_MARGIN));
  CPPUNIT_ASSERT(isCountInRange(MATCH_COUNT * 0.35, p2Wins, PROBABILITY_MARGIN));
  CPPUNIT_ASSERT(isCountInRange(gamesCount * 0.3, gamesBeyond21, PROBABILITY_MARGIN));

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

