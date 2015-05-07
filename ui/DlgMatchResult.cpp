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
  ui->laMatchNum->setText(tr("Match Number:") + QString::number(ma.getMatchNumber()));
  ui->laCatName->setText(ma.getCategory().getName());

  // set the colors of the winner, loser and draw labels
  ui->laWinnerName->setStyleSheet("QLabel { color : green; }");
  ui->laLoserName->setStyleSheet("QLabel { color : red; }");
  ui->laDraw->setStyleSheet("QLabel { color : blue; }");

  updateControls();
}

//----------------------------------------------------------------------------

DlgMatchResult::~DlgMatchResult()
{
  delete ui;
}

//----------------------------------------------------------------------------

void DlgMatchResult::onGameScoreSelectionChanged()
{
  updateControls();
}

//----------------------------------------------------------------------------

void DlgMatchResult::updateControls()
{
  // is a 3rd game possible?
  //
  // TODO: the number of two won games for a victory is hardcoded here!
  int round = ma.getMatchGroup().getRound();
  bool game3Possible = (ma.getCategory().getMaxNumGamesInRound(round) > 2);

  // is a third game necessary?
  bool g1Valid = ui->game1Widget->hasValidScore();
  bool g2Valid = ui->game2Widget->hasValidScore();
  bool g3Valid = ui->game3Widget->hasValidScore();
  bool game3Necessary = false;
  if (g1Valid && g2Valid && game3Possible)
  {
    auto sc1 = ui->game1Widget->getScore();
    auto sc2 = ui->game2Widget->getScore();
    assert(sc1 != nullptr);
    assert(sc2 != nullptr);

    game3Necessary = (sc1->getWinner() != sc2->getWinner());
  }

  // enable or disable the widget for the 3rd game
  ui->game3Widget->setEnabled(game3Necessary);

  // is the currently selected result valid?
  bool validResult = true;
  if (!g1Valid) validResult = false;
  if (!g2Valid) validResult = false;
  if (game3Necessary && !g3Valid) validResult = false;

  // enable or disable the okay-button
  ui->btnOkay->setEnabled(validResult);

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

