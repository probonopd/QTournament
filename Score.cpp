/*
 *    This is QTournament, a badminton tournament management program.
 *    Copyright (C) 2014 - 2016  Volker Knollmann
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

#include <memory>
#include <cmath>

#include <exception>
#include <QStringList>
#include <QtGlobal>

#include "Score.h"


namespace QTournament {

bool GameScore::isValidScore(int sc1, int sc2)
{
  // no negative scores
  if ((sc1 < 0) || (sc2 < 0)) return false;

  // no score above 30
  if ((sc1 > 30) || (sc2 > 30)) return false;

  // no draw
  if (sc1 == sc2) return false;

  int maxScore = (sc1 > sc2) ? sc1 : sc2;
  int minScore = (maxScore == sc1) ? sc2 : sc1;

  // special case: victory by 30:29
  if ((maxScore == 30) && (minScore == 29)) return true;

  // any other case:
  // maxScore must be at least 21 and minScore two below
  if (maxScore < 21) return false;
  if ((maxScore - minScore) < 2) return false;
  if ((maxScore > 21) && ((maxScore - minScore) != 2)) return false;

  return true;
}

//----------------------------------------------------------------------------

unique_ptr<GameScore> GameScore::fromScore(int sc1, int sc2)
{
  // allow only valid scores to be instanciated as a new GameScore object
  if (!(isValidScore(sc1, sc2))) return nullptr;

  // create and return new object
  return unique_ptr<GameScore>(new GameScore(sc1, sc2));
}

//----------------------------------------------------------------------------

GameScore::GameScore(int sc1, int sc2)
{
  if (!(isValidScore(sc1, sc2)))
  {
    // should never happen since the ctor is private and the
    // factory methods fromString and fromScore make all
    // necessary checks
    throw std::runtime_error("Attempt to create GameScore object with invalid score");
  }

  player1Score = sc1;
  player2Score = sc2;
}

//----------------------------------------------------------------------------

unique_ptr<GameScore> GameScore::fromString(const QString &s)
{
  // we need exactly one delimiter
  QStringList scores = s.split(":");
  if (scores.size() != 2) return nullptr;

  // both scores must be valid numbers
  int sc1 = -1;
  int sc2 = -1;
  try
  {
    bool isOk = false;
    sc1 = scores[0].trimmed().toInt(&isOk);
    if (!isOk) return nullptr;

    isOk = false;
    sc2 = scores[1].trimmed().toInt(&isOk);
    if (!isOk) return nullptr;
  }
  catch (exception ex) {
    return nullptr;
  }

  return fromScore(sc1, sc2);
}

//----------------------------------------------------------------------------

QString GameScore::toString() const
{
  return QString::number(player1Score) + ":" + QString::number(player2Score);
}

//----------------------------------------------------------------------------

tuple<int, int> GameScore::getScore() const
{
  return make_tuple(player1Score, player2Score);
}

//----------------------------------------------------------------------------

int GameScore::getWinner() const
{
  return (player1Score > player2Score) ? 1 : 2;
}

//----------------------------------------------------------------------------

int GameScore::getLoser() const
{
  return (player1Score < player2Score) ? 1 : 2;
}

//----------------------------------------------------------------------------

int GameScore::getWinnerScore() const
{
  return (player1Score > player2Score) ? player1Score : player2Score;
}

//----------------------------------------------------------------------------

int GameScore::getLoserScore() const
{
  return (player1Score < player2Score) ? player1Score : player2Score;
}

//----------------------------------------------------------------------------

int GameScore::getWinnerScoreForLoserScore(int loserScore)
{
  if (loserScore < 0) return -1; // invalid value
  if (loserScore < 20) return 21;
  if (loserScore < 29) return loserScore + 2;
  if (loserScore == 29) return 30;
  return -1;  // invalid
}

//----------------------------------------------------------------------------

unique_ptr<GameScore> GameScore::genRandomGame(int winner)
{
  if ((winner < 0) || (winner > 2)) return nullptr;

  // a nice little lambda expression for returning a random number
  // in the range 0 <= r < 1.0
  auto myRand = []() -> double { return qrand() / (RAND_MAX * 1.0); };
  // or in the range 0 <= r <= maxInt
  auto myRandInt = [myRand](int maxInt) -> int { return static_cast<int>(round(myRand() * maxInt)); };

  // some arbitrary values:
  // probability for a games to last beyond 21 points
  constexpr double beyond21PointsThreshold = 0.3;

  // define the winner, if necessary
  if (winner == 0)
  {
    winner = (myRand() > 0.5) ? 2 : 1;
  }

  int loserScore = myRandInt(19);
  if (myRand() < beyond21PointsThreshold)
  {
    loserScore = 20 + myRandInt(9);
  }

  if (winner == 1)
  {
    return fromScore(getWinnerScoreForLoserScore(loserScore), loserScore);
  }
  return fromScore(loserScore, getWinnerScoreForLoserScore(loserScore));
}

//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
//----------------MatchScore class--------------------------------------------
//----------------------------------------------------------------------------

GameScoreList MatchScore::string2GameScoreList(QString s)
{
  GameScoreList result;

  // games are comma-seüarated
  for (QString gameString : s.split(","))
  {
    auto gameSore = GameScore::fromString(gameString);

    // on error return an empty list
    if (gameSore == nullptr)
    {
      return GameScoreList();
    }

    result.append(*gameSore);
  }

  return result;
}

//----------------------------------------------------------------------------

bool MatchScore::isValidScore(const QString& s, int numWinGames, bool drawAllowed)
{
  GameScoreList gsl = MatchScore::string2GameScoreList(s);
  return isValidScore(gsl, numWinGames, drawAllowed);
}

//----------------------------------------------------------------------------

bool MatchScore::isValidScore(const GameScoreList& gsl, int numWinGames, bool drawAllowed)
{
  // empty list (e.g., from invalid string)
  if (gsl.isEmpty()) return false;

  // get statistics about wins and losses of the two players
  auto gameSum = getGameSum(gsl);
  int p1Wins = get<0>(gameSum);
  int p2Wins = get<1>(gameSum);

  // check for draw
  if (p1Wins == p2Wins)
  {
    if (!drawAllowed) return false;

    // if we have a valid draw, each player must
    // have won one game less than necessary for
    // winning the match
    //
    // Since both players have won the same number
    // of games (that's checked in the if-clause above)
    // we only need to check player1 here
    if (p1Wins == (numWinGames - 1)) return true;

    // in any other case, this draw result is invalid
    return false;
  }

  // in a regular match, the sum of all games may not
  // exceed the number of games we need for a win
  // times 2 and minus 1
  if (gsl.count() > (numWinGames * 2 - 1)) return false;

  // player one must have either won the necessary number
  // of games or lost the necessary number of games (which
  // implicitly means that player 2 won the games)
  if ((p1Wins == numWinGames) || (p2Wins == numWinGames)) return true;

  // catch any other error
  return false;
}

//----------------------------------------------------------------------------

unique_ptr<MatchScore> MatchScore::fromString(const QString& s, int numWinGames, bool drawAllowed)
{
  GameScoreList gsl = string2GameScoreList(s);

  return fromGameScoreList(gsl, numWinGames, drawAllowed);
}

//----------------------------------------------------------------------------

unique_ptr<MatchScore> MatchScore::fromStringWithoutValidation(const QString& s)
{
  GameScoreList gsl = string2GameScoreList(s);

  return fromGameScoreListWithoutValidation(gsl);
}

//----------------------------------------------------------------------------

unique_ptr<MatchScore> MatchScore::fromGameScoreList(const GameScoreList& gsl, int numWinGames, bool drawAllowed)
{
  if (!(isValidScore(gsl, numWinGames, drawAllowed)))
  {
    return nullptr;
  }

  return fromGameScoreListWithoutValidation(gsl);
}

//----------------------------------------------------------------------------

unique_ptr<MatchScore> MatchScore::fromGameScoreListWithoutValidation(const GameScoreList& gsl)
{
  MatchScore* result = new MatchScore();
  for (GameScore game : gsl)
  {
    result->addGame(game);
  }

  return unique_ptr<MatchScore>(result);
}

//----------------------------------------------------------------------------

bool MatchScore::addGame(const GameScore& sc)
{
  games.append(sc);
  return true;
}

//----------------------------------------------------------------------------

bool MatchScore::addGame(const QString& scString)
{
  auto sc = GameScore::fromString(scString);
  if (sc == nullptr) return false;

  games.append(*sc);
  return true;
}

//----------------------------------------------------------------------------

QString MatchScore::toString() const
{
  if (games.isEmpty()) return "";

  QString result;
  for (GameScore g : games)
  {
    result += g.toString() + ",";
  }

  return result.left(result.length() - 1);
}

//----------------------------------------------------------------------------

int MatchScore::getWinner() const
{
  auto gamesSum = getGameSum();
  int p1Games = get<0>(gamesSum);
  int p2Games = get<1>(gamesSum);
  if (p1Games > p2Games) return 1;
  if (p2Games > p1Games) return 2;
  return 0; // draw
}

//----------------------------------------------------------------------------

int MatchScore::getLoser() const
{
  int winner = getWinner();
  if (winner == 1) return 2;
  if (winner == 2) return 1;
  return 0;  // draw
}

//----------------------------------------------------------------------------

tuple<int, int> MatchScore::getGameSum() const
{
  return getGameSum(games);
}

//----------------------------------------------------------------------------

tuple<int, int> MatchScore::getGameSum(const GameScoreList& gsl)
{
  // get statistics about wins and losses of the two players
  int p1Wins = 0;
  for (GameScore game : gsl)
  {
    if (game.getWinner() == 1) ++p1Wins;
  }
  int p2Wins = gsl.count() - p1Wins;

  return make_tuple(p1Wins, p2Wins);
}

//----------------------------------------------------------------------------

tuple<int, int> MatchScore::getMatchSum() const
{
  if (getWinner() == 1) return make_tuple(1, 0);
  if (getWinner() == 2) return make_tuple(0, 1);
  return make_tuple(0,0); //draw
}

//----------------------------------------------------------------------------

tuple<int, int> MatchScore::getScoreSum() const
{
  int p1Score = 0;
  int p2Score = 0;
  for (GameScore game : games)
  {
    auto sc = game.getScore();
    p1Score += get<0>(sc);
    p2Score += get<1>(sc);
  }

  return make_tuple(p1Score, p2Score);
}

//----------------------------------------------------------------------------

bool MatchScore::isValidScore(int numWinGames, bool drawAllowed) const
{
  return isValidScore(games, numWinGames, drawAllowed);
}

//----------------------------------------------------------------------------

unique_ptr<MatchScore> MatchScore::genRandomScore(int numWinGames, bool drawAllowed)
{
  // a nice little lambda expression for returning a random number
  // in the range 0 <= r < 1.0
  auto myRand = []() -> double { return qrand() / (RAND_MAX * 1.0); };

  // some arbitrary values:
  // probability for a draw
  constexpr double drawThreshold = 0.3;
  // probability the winner of the first game to win the following games as well
  constexpr double firstWinnerIsStrongerThreshold = 0.7;

  // container for the match result
  GameScoreList gsl;

  // we have an actual draw
  if (drawAllowed && (myRand() < drawThreshold))
  {
    int wonGamesForEachPlayer = numWinGames -1 ;

    // there is no point in shuffling the sequence
    // of won / lost games in a match, since the sequence
    // hasn't any influence on the final ranking.
    // Thus, we create alternating wins for the two players
    for (int i=0; i< wonGamesForEachPlayer; ++i)
    {
      gsl.append(*(GameScore::genRandomGame(1)));
      gsl.append(*(GameScore::genRandomGame(2)));
    }

    return fromGameScoreListWithoutValidation(gsl);
  }

  // if we make it to this point, we have a real winner / loser
  int firstWinner = -1;
  int p1WinGameCount = 0;
  int p2WinGameCount = 0;
  while (true)
  {
    int thisGameWinner = -1;

    // decide who wins the first game
    if (firstWinner < 0)
    {
      firstWinner = (myRand() < 0.5) ? 1 : 2;
      thisGameWinner = firstWinner;
    }

    // decide who wins subsequent games
    if (firstWinner > 0)
    {
      if (myRand() < firstWinnerIsStrongerThreshold)
      {
        thisGameWinner = firstWinner;
      } else {
        thisGameWinner = (firstWinner == 1) ? 2 : 1;
      }
    }

    gsl.append(*(GameScore::genRandomGame(thisGameWinner)));

    if (thisGameWinner == 1) ++p1WinGameCount;
    else ++p2WinGameCount;

    if ((p1WinGameCount == numWinGames) || (p2WinGameCount == numWinGames)) break;
  }

  return fromGameScoreListWithoutValidation(gsl);
}

//----------------------------------------------------------------------------

unique_ptr<GameScore> MatchScore::getGame(int n) const
{
  if ((n < 0) || (n > (games.count() - 1)))
  {
    return nullptr;
  }

  return unique_ptr<GameScore>(new GameScore(games.at(n)));
}

//----------------------------------------------------------------------------

int MatchScore::getNumGames() const
{
  return games.count();
}

//----------------------------------------------------------------------------

int MatchScore::getPointsSum() const
{
  auto scoreSum = getScoreSum();
  return (get<0>(scoreSum) + get<1>(scoreSum));
}

//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------

}
