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

#include <optional>

#include "CatRoundStatus.h"

#include "DlgRoundFinished.h"
#include "ui_DlgRoundFinished.h"

using namespace QTournament;

DlgRoundFinished::DlgRoundFinished(QWidget *parent, const QTournament::Category& _cat, int _round) :
  QDialog(parent),
  ui(new Ui::DlgRoundFinished),
  db{_cat.getDatabaseHandle()}, cat{_cat}, round{_round}
{
  ui->setupUi(this);

  // select the round we're processing
  CatRoundStatus crs = cat.getRoundStatus();
  if (round < 0)
  {
    // auto-select the last finished round
    round = crs.getFinishedRoundsCount();
  } else {
    // check the validity of the user provided round
    if ((round < 1) || (round > crs.getFinishedRoundsCount()))
    {
      throw invalid_argument("DlgRoundFinished: invalid round number provided by caller");
    }
  }

  // set the title
  QString msg = tr("Round %1 of category %2 finished!");
  msg = msg.arg(round).arg(cat.getName());
  ui->laTitle->setText(msg);

  // decide which reports to offer
  /*try
  {
    optBracket.reset();
    optBracket.emplace(db, "dummy", cat);
  }
  catch (...) {}
  ui->btnBracket->setVisible(optBracket.has_value());*/
  ui->btnBracket->setVisible(false);  // FIX ME: temporary default

  try
  {
    optInOut.reset();
    optInOut.emplace(db, "dummy", cat, round);
  }
  catch (...) {}
  ui->btnInOut->setVisible(optInOut.has_value());

  try
  {
    optMatrix.reset();
    optMatrix.emplace(db, "dummy", cat, round);
  }
  catch (...) {}
  ui->btnMatrix->setVisible(optMatrix.has_value());

  try
  {
    optResults.reset();
    optResults.emplace(db, "dummy", cat, round);
  }
  catch (...) {}
  ui->btnResults->setVisible(optResults.has_value());

  try
  {
    optStandings.reset();
    optStandings.emplace(db, "dummy", cat, round);
  }
  catch (...) {}
  ui->btnStandings->setVisible(optStandings.has_value());

  // manually wire the buttons to the slots.
  // that's easier than fiddling with the designer...
  connect(ui->btnBracket, SIGNAL(clicked(bool)), this, SLOT(onBtnBracketClicked()));
  connect(ui->btnInOut, SIGNAL(clicked(bool)), this, SLOT(onBtnInOutClicked()));
  connect(ui->btnMatrix, SIGNAL(clicked(bool)), this, SLOT(onBtnMatrixClicked()));
  connect(ui->btnResults, SIGNAL(clicked(bool)), this, SLOT(onBtnResultsClicked()));
  connect(ui->btnStandings, SIGNAL(clicked(bool)), this, SLOT(onBtnStandingsClicked()));
}

//----------------------------------------------------------------------------

DlgRoundFinished::~DlgRoundFinished()
{
  delete ui;
}

//----------------------------------------------------------------------------

void DlgRoundFinished::printReport(AbstractReport* rep)
{
  if (rep == nullptr) return;

  // let the report object create the actual output
  upSimpleReport sr = rep->regenerateReport();

  // create an invisible report viewer and directly trigger
  // the print reaction
  SimpleReportLib::SimpleReportViewer viewer{this};
  viewer.setReport(sr.get());
  viewer.onBtnPrintClicked();
}

//----------------------------------------------------------------------------

void DlgRoundFinished::onBtnBracketClicked()
{
  // FIX ME
  //printReport(&(*optBracket));
}

//----------------------------------------------------------------------------

void DlgRoundFinished::onBtnInOutClicked()
{
  printReport(&(*optInOut));
}

//----------------------------------------------------------------------------

void DlgRoundFinished::onBtnResultsClicked()
{
  printReport(&(*optResults));
}

//----------------------------------------------------------------------------

void DlgRoundFinished::onBtnMatrixClicked()
{
  printReport(&(*optMatrix));
}

//----------------------------------------------------------------------------

void DlgRoundFinished::onBtnStandingsClicked()
{
  printReport(&(*optStandings));
}

//----------------------------------------------------------------------------


//----------------------------------------------------------------------------

