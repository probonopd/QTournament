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

#ifndef SCORE_H
#define SCORE_H

#include <memory>
#include <tuple>

#include <QString>
#include <QList>

using namespace std;

namespace QTournament {

class GameScore
{
public:
  static bool isValidScore(int sc1, int sc2);
  static unique_ptr<GameScore> fromScore(int sc1, int sc2);
  static unique_ptr<GameScore> fromString(const QString& s);
  static int getWinnerScoreForLoserScore(int loserScore);
  static unique_ptr<GameScore> genRandomGame(int winner = 0);

  QString toString() const;

  tuple<int, int> getScore() const;
  int getWinner() const;
  int getLoser() const;
  int getWinnerScore() const;
  int getLoserScore() const;

private:
  GameScore(int sc1, int sc2);

  int player1Score;
  int player2Score;

};

typedef QList<GameScore> GameScoreList;

class MatchScore
{
public:
  static unique_ptr<MatchScore> fromString(const QString& s, int numWinGames=2, bool drawAllowed=false);
  static unique_ptr<MatchScore> fromStringWithoutValidation(const QString& s);
  static unique_ptr<MatchScore> fromGameScoreList(const GameScoreList& gsl, int numWinGames=2, bool drawAllowed=false);
  static unique_ptr<MatchScore> fromGameScoreListWithoutValidation(const GameScoreList& gsl);
  static bool isValidScore(const QString& s, int numWinGames=2, bool drawAllowed=false);
  static bool isValidScore(const GameScoreList& gsl, int numWinGames=2, bool drawAllowed=false);
  bool isValidScore(int numWinGames=2, bool drawAllowed=false) const;

  QString toString() const;

  int getWinner() const;
  int getLoser() const;

  int getNumGames() const;
  int getPointsSum() const;

  unique_ptr<GameScore> getGame(int n) const;

  tuple<int, int> getScoreSum() const;

  tuple<int, int> getGameSum() const;
  static tuple<int, int> getGameSum(const GameScoreList& gsl);

  tuple<int, int> getMatchSum() const;

  static unique_ptr<MatchScore> genRandomScore(int numWinGames=2, bool drawAllowed=false);

private:
  MatchScore() {};
  bool addGame(const GameScore& sc);
  bool addGame(const QString& scString);
  static GameScoreList string2GameScoreList(QString s);

  GameScoreList games;

};

typedef QList<MatchScore> MatchScoreList;

}
#endif // SCORE_H
