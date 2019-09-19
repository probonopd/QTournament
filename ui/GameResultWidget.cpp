/*
 *    This is QTournament, a badminton tournament management program.
 *    Copyright (C) 2014 - 2019  Volker Knollmann
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

#include <assert.h>

#include "GameResultWidget.h"
#include "ui_GameResultWidget.h"
#include "Score.h"

using namespace QTournament;

GameResultWidget::GameResultWidget(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::GameResultWidget)
{
  ui->setupUi(this);
}

//----------------------------------------------------------------------------

GameResultWidget::~GameResultWidget()
{
  delete ui;
}

//----------------------------------------------------------------------------

void GameResultWidget::setGameNumber(int n)
{
  if (n < 1) return;
  ui->laGame->setText(QString::number(n) + ". " + tr("Game:"));
}

//----------------------------------------------------------------------------

bool GameResultWidget::hasValidScore() const
{
  auto [sc1, sc2] = getSelectedScore();

  return GameScore::isValidScore(sc1, sc2);
}

//----------------------------------------------------------------------------

std::optional<QTournament::GameScore> GameResultWidget::getScore() const
{
  if (!hasValidScore())
  {
    return {};
  }

  auto [sc1, sc2] = getSelectedScore();

  return QTournament::GameScore::fromScore(sc1, sc2);
}

//----------------------------------------------------------------------------

void GameResultWidget::setScore(const GameScore& sc)
{
  auto [sc1, sc2] = sc.getScore();

  int newIndex = ui->cbPlayer1Score->findText(QString::number(sc1));
  ui->cbPlayer1Score->setCurrentIndex(newIndex);
  newIndex = ui->cbPlayer1Score->findText(QString::number(sc2));
  ui->cbPlayer2Score->setCurrentIndex(newIndex);
}

//----------------------------------------------------------------------------

void GameResultWidget::onScoreSelectionChanged()
{
  QComboBox* sender = dynamic_cast<QComboBox*>(QObject::sender());
  assert ((sender == ui->cbPlayer1Score) || (sender == ui->cbPlayer2Score));

  // if the other combobox is still on "auto", we
  // auto-set the other score and assume that the
  // "auto"-box is the winner of the game
  QString newVal;
  QString otherVal;
  if (sender == ui->cbPlayer1Score)
  {
    newVal = ui->cbPlayer1Score->currentText();
    otherVal = ui->cbPlayer2Score->currentText();
  } else {
    newVal = ui->cbPlayer2Score->currentText();
    otherVal = ui->cbPlayer1Score->currentText();
  }

  newVal = newVal.trimmed();
  otherVal = otherVal.trimmed();
  if ((otherVal.toLower() == "auto") && (!(newVal.isEmpty())) && (!(newVal.toLower() == "auto")))
  {
    bool isOk;
    int loserScore = newVal.toInt(&isOk);
    assert(isOk);

    int winnerScore = GameScore::getWinnerScoreForLoserScore(loserScore);

    if (winnerScore > 0)
    {
      if (sender == ui->cbPlayer1Score)
      {
        int newIndex = ui->cbPlayer2Score->findText(QString::number(winnerScore));
        assert(newIndex >= 0);
        ui->cbPlayer2Score->setCurrentIndex(newIndex);
      } else {
        int newIndex = ui->cbPlayer1Score->findText(QString::number(winnerScore));
        assert(newIndex >= 0);
        ui->cbPlayer1Score->setCurrentIndex(newIndex);
      }
    }

  }

  emit scoreSelectionChanged();
}

//----------------------------------------------------------------------------

std::tuple<int, int> GameResultWidget::getSelectedScore() const
{
  // a little helper function to convert the combobox-value
  // to an integer
  auto valueConverter = [](const QString& val) {
    if (val.toLower() == "auto")
    {
      return AUTO_SCORE;
    }
    if (val.trimmed().isEmpty())
    {
      return EMPTY_SCORE;
    }
    bool isOk;
    int result = val.toInt(&isOk);

    if (!isOk)
    {
      // shouldn't happen, but anyway...
      return EMPTY_SCORE;
    }

    return result;
  };

  int sc1 = valueConverter(ui->cbPlayer1Score->currentText());
  int sc2 = valueConverter(ui->cbPlayer2Score->currentText());

  return std::tuple{sc1, sc2};
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

