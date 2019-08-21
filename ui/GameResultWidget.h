/*
 *    This is QTournament, a badminton tournament management program.
 *    Copyright (C) 2014 - 2017  Volker Knollmann
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

#ifndef GAMERESULTWIDGET_H
#define GAMERESULTWIDGET_H

#include <tuple>
#include <memory>

#include <QWidget>

#include "Score.h"


namespace Ui {
  class GameResultWidget;
}

class GameResultWidget : public QWidget
{
  Q_OBJECT

public:
  static constexpr int AUTO_SCORE = -1;
  static constexpr int EMPTY_SCORE = -2;
  explicit GameResultWidget(QWidget *parent = nullptr);
  ~GameResultWidget();
  void setGameNumber(int n);
  bool hasValidScore() const;
  std::optional<QTournament::GameScore> getScore() const;
  void setScore(const QTournament::GameScore& sc);

private slots:
  void onScoreSelectionChanged();

signals:
  void scoreSelectionChanged();

private:
  std::tuple<int, int> getSelectedScore() const;
  Ui::GameResultWidget *ui;
};

#endif // GAMERESULTWIDGET_H
