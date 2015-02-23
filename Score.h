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

  QString toString() const;

  tuple<int, int> getScore() const;
  int getWinner() const;
  int getLoser() const;

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
  unique_ptr<GameScore> getGame(int n) const;

  tuple<int, int> getScoreSum() const;

  tuple<int, int> getGameSum() const;
  static tuple<int, int> getGameSum(const GameScoreList& gsl);

  tuple<int, int> getMatchSum() const;

private:
  MatchScore() {};
  bool addGame(const GameScore& sc);
  bool addGame(const QString& scString);
  static GameScoreList string2GameScoreList(QString s);

  GameScoreList games;

};
}
#endif // SCORE_H
