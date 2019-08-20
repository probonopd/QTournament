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

#ifndef DLGROUNDFINISHED_H
#define DLGROUNDFINISHED_H

#include <optional>

#include <QDialog>

#include <SimpleReportGeneratorLib/SimpleReportViewer.h>

#include "TournamentDB.h"
#include "Category.h"

#include "reports/BracketSheet.h"
#include "reports/InOutList.h"
#include "reports/ResultsAndNextMatches.h"
#include "reports/MatrixAndStandings.h"
#include "reports/Standings.h"


namespace Ui {
  class DlgRoundFinished;
}

class DlgRoundFinished : public QDialog
{
  Q_OBJECT

public:
  explicit DlgRoundFinished(QWidget *parent, const QTournament::Category& _cat, int _round = -1);
  ~DlgRoundFinished();

  void printReport(QTournament::AbstractReport* rep);

public slots:
  void onBtnBracketClicked();
  void onBtnInOutClicked();
  void onBtnResultsClicked();
  void onBtnMatrixClicked();
  void onBtnStandingsClicked();

private:
  Ui::DlgRoundFinished *ui;
  const QTournament::TournamentDB& db;
  const QTournament::Category& cat;
  int round;

  std::optional<QTournament::BracketSheet> optBracket;
  std::optional<QTournament::InOutList> optInOut;
  std::optional<QTournament::ResultsAndNextMatches> optResults;
  std::optional<QTournament::MatrixAndStandings> optMatrix;
  std::optional<QTournament::Standings> optStandings;
};

#endif // DLGROUNDFINISHED_H
