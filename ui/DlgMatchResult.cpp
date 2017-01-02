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

#include <QMessageBox>

#include "DlgMatchResult.h"
#include "ui_DlgMatchResult.h"
#include "Match.h"
#include "MatchGroup.h"

DlgMatchResult::DlgMatchResult(QWidget *parent, const Match& _ma) :
  QDialog(parent),
  ui(new Ui::DlgMatchResult), ma(_ma)
{
  ui->setupUi(this);

  // connect the game widget score selection changed signals
  // to our handler
  connect(ui->game1Widget, SIGNAL(scoreSelectionChanged()), this, SLOT(onGameScoreSelectionChanged()));
  connect(ui->game2Widget, SIGNAL(scoreSelectionChanged()), this, SLOT(onGameScoreSelectionChanged()));
  connect(ui->game3Widget, SIGNAL(scoreSelectionChanged()), this, SLOT(onGameScoreSelectionChanged()));

  // initialize the text labels
  ui->game1Widget->setGameNumber(1);
  ui->game2Widget->setGameNumber(2);
  ui->game3Widget->setGameNumber(3);
  ui->laMatchNum->setText(tr("Match Number: ") + QString::number(ma.getMatchNumber()));
  ui->laCatName->setText(ma.getCategory().getName());

  QString p1 = ma.getPlayerPair1().getDisplayName();
  p1.replace(" / ", " /\n");
  QString p2 = ma.getPlayerPair2().getDisplayName();
  p2.replace(" / ", " /\n");
  ui->laPlayer1Name->setText(p1);
  ui->laPlayer2Name->setText(p2);

  // set the colors of the winner, loser and draw labels
  ui->laWinnerName->setStyleSheet("QLabel { color : green; }");
  ui->laLoserName->setStyleSheet("QLabel { color : red; }");
  ui->laDraw->setStyleSheet("QLabel { color : blue; }");

  // create a keyboard shortcut for setting a random result
  shortcutRandomResult = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_R), this, SLOT(onRandomResultTriggered()));

  // initialize buttons, label visibility etc.
  updateControls();
}

//----------------------------------------------------------------------------

DlgMatchResult::~DlgMatchResult()
{
  delete ui;
  delete shortcutRandomResult;
}

//----------------------------------------------------------------------------

unique_ptr<MatchScore> DlgMatchResult::getMatchScore() const
{
  if (!hasValidResult())
  {
    return nullptr;
  }

  GameScoreList gsl;
  gsl.append(*(ui->game1Widget->getScore()));
  gsl.append(*(ui->game2Widget->getScore()));
  if (isGame3Necessary())
  {
    gsl.append(*(ui->game3Widget->getScore()));
  }

  return MatchScore::fromGameScoreListWithoutValidation(gsl);
}

//----------------------------------------------------------------------------

void DlgMatchResult::onGameScoreSelectionChanged()
{
  updateControls();
}

//----------------------------------------------------------------------------

void DlgMatchResult::onRandomResultTriggered()
{
  // is a draw possible?
  //
  // TODO: the number of two won games for a victory is hardcoded here!
  int round = ma.getMatchGroup().getRound();
  bool isDrawPossible = (ma.getCategory().getMaxNumGamesInRound(round) == 2);

  // generate a random match score
  //
  // TODO: "2" win games hardcoded again
  auto randomResult = MatchScore::genRandomScore(2, isDrawPossible);
  assert(randomResult != nullptr);

  ui->game1Widget->setScore(*(randomResult->getGame(0)));
  ui->game2Widget->setScore(*(randomResult->getGame(1)));
  if (randomResult->getNumGames() > 2)
  {
    ui->game3Widget->setScore(*(randomResult->getGame(2)));
  }

  updateControls();
}

//----------------------------------------------------------------------------

void DlgMatchResult::updateControls()
{
  // is a third game necessary?
  bool game3Necessary = isGame3Necessary();

  // enable or disable the widget for the 3rd game
  ui->game3Widget->setEnabled(game3Necessary);

  // is the currently selected result valid?
  bool validResult = hasValidResult();

  // enable or disable the okay-button
  ui->btnOkay->setEnabled(validResult);

  // hide the winner / loser / draw labels if we have no valid result
  if (!validResult)
  {
    ui->laWinnerName->setVisible(false);
    ui->laLoserName->setVisible(false);
    ui->laDraw->setVisible(false);
  } else {
    auto sc1 = ui->game1Widget->getScore();
    auto sc2 = ui->game2Widget->getScore();
    assert(sc1 != nullptr);
    assert(sc2 != nullptr);

    // switch the "draw" label on or off
    bool isDraw = (!game3Necessary && (sc1->getWinner() != sc2->getWinner()));
    ui->laDraw->setVisible(isDraw);

    // switch / set the winner label
    auto matchScore = getMatchScore();
    assert(matchScore != nullptr);
    QString winnerLabel;
    QString loserLabel;
    if (matchScore->getWinner() == 1)
    {
      winnerLabel = ma.getPlayerPair1().getDisplayName();
      loserLabel = ma.getPlayerPair2().getDisplayName();
    } else {
      winnerLabel = ma.getPlayerPair2().getDisplayName();
      loserLabel = ma.getPlayerPair1().getDisplayName();
    }
    if (!isDraw)
    {
      ui->laWinnerName->setVisible(true);
      ui->laWinnerName->setText(tr("Winner: ") + winnerLabel);
      ui->laLoserName->setVisible(true);
      ui->laLoserName->setText(tr("Loser: ") + loserLabel);
    } else {
      ui->laWinnerName->setVisible(false);
      ui->laLoserName->setVisible(false);
    }
  }

}

//----------------------------------------------------------------------------

bool DlgMatchResult::isGame3Necessary() const
{
  // is a 3rd game possible?
  //
  // TODO: the number of two won games for a victory is hardcoded here!
  int round = ma.getMatchGroup().getRound();
  bool game3Possible = (ma.getCategory().getMaxNumGamesInRound(round) > 2);
  if (!game3Possible)
  {
    return false;
  }

  // is a third game necessary?
  bool g1Valid = ui->game1Widget->hasValidScore();
  bool g2Valid = ui->game2Widget->hasValidScore();
  if (g1Valid && g2Valid)
  {
    auto sc1 = ui->game1Widget->getScore();
    auto sc2 = ui->game2Widget->getScore();
    assert(sc1 != nullptr);
    assert(sc2 != nullptr);

    return (sc1->getWinner() != sc2->getWinner());
  }

  return false;
}

//----------------------------------------------------------------------------

bool DlgMatchResult::hasValidResult() const
{
  bool g1Valid = ui->game1Widget->hasValidScore();
  bool g2Valid = ui->game2Widget->hasValidScore();
  bool g3Valid = ui->game3Widget->hasValidScore();
  bool validResult = true;
  if (!g1Valid) validResult = false;
  if (!g2Valid) validResult = false;
  if (isGame3Necessary() && !g3Valid) validResult = false;

  return validResult;
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

