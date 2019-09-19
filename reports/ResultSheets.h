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

#ifndef RESULTSHEETS_H
#define RESULTSHEETS_H

#include <functional>

#include <QObject>

#include "reports/AbstractReport.h"
#include "TournamentDB.h"
#include "TournamentDataDefs.h"

namespace QTournament
{
  class ResultSheets : public QObject, public AbstractReport
  {
    Q_OBJECT

  public:
    ResultSheets(const QTournament::TournamentDB& _db, const QString& _name, int _numMatches);
    ResultSheets(const QTournament::TournamentDB& _db, const Match& firstMatchForPrinting, int _numMatches=1);

    virtual upSimpleReport regenerateReport() override;
    virtual QStringList getReportLocators() const override;

  public slots:
    void onMatchSelectionChanged(int newlySelectedMatchId);

  private:
    static constexpr int SheetsPerPage = 4;
    static constexpr int GamesPerSheet = 3;
    static constexpr double SheetHeight_mm = HeightA4_mm / SheetsPerPage;
    static constexpr double SheetTopMargin_mm = 10.0;

    int numMatches;
    int firstMatchNum{-1};
    void printMatchData(upSimpleReport& rep, const Match& ma);
  };

}
#endif // RESULTSHEETS_H
