#ifndef GAMERESULTWIDGET_H
#define GAMERESULTWIDGET_H

#include <tuple>
#include <memory>

#include <QWidget>

#include "Score.h"

using namespace std;
using namespace QTournament;

namespace Ui {
  class GameResultWidget;
}

class GameResultWidget : public QWidget
{
  Q_OBJECT

public:
  static constexpr int AUTO_SCORE = -1;
  static constexpr int EMPTY_SCORE = -2;
  explicit GameResultWidget(QWidget *parent = 0);
  ~GameResultWidget();
  void setGameNumber(int n);
  bool hasValidScore() const;
  unique_ptr<GameScore> getScore() const;

private slots:
  void onScoreSelectionChanged();

signals:
  void scoreSelectionChanged();

private:
  tuple<int, int> getSelectedScore() const;
  Ui::GameResultWidget *ui;
};

#endif // GAMERESULTWIDGET_H
