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

#ifndef BRACKETSHEET_H
#define BRACKETSHEET_H

#include <functional>
#include <tuple>

#include <QObject>

#include "SimpleReportGenerator.h"

#include "reports/AbstractReport.h"
#include "BracketVisData.h"
#include "TournamentDB.h"
#include "TournamentDataDefs.h"

using namespace SqliteOverlay;

namespace QTournament
{
  class BracketSheet : public QObject, public AbstractReport
  {
    Q_OBJECT

    enum class BRACKET_TEXT_ELEMENT {
      PAIR1,
      PAIR2,
      INITIAL_RANK1,
      INITIAL_RANK2,
      SCORE,
      MATCH_NUM,
      WINNER_RANK,
      TERMINATOR_NAME
    };

    static constexpr char BRACKET_STYLE[] = "BracketText";
    static constexpr char BRACKET_STYLE_ITALICS[] = "BracketTextItalics";
    static constexpr char BRACKET_STYLE_BOLD[] = "BracketTextBold";

  public:
    BracketSheet(TournamentDB* _db, const QString& _name, const Category& _cat);

    virtual upSimpleReport regenerateReport() override;
    virtual QStringList getReportLocators() const override;

    static constexpr double GAP_LINE_TXT__MM = 1.0;

  private:
    Category cat;

    SimpleReportLib::SimpleReportGenerator* rawReport;  // raw pointer, only to be used during regenerateReport! (BAAAD style)
    double xFac;
    double yFac;

    void determineGridSize();
    void setupTextStyle();
    tuple<double, double> grid2MM(int gridX, int gridY) const;
    void drawBracketTextItem(int bracketX0, int bracketY0, int ySpan, BRACKET_ORIENTATION orientation, QString txt, BRACKET_TEXT_ELEMENT item, const QString& styleNameOverride="") const;
    QString getTruncatedPlayerName(const Player& p, const QString& postfix, double maxWidth, SimpleReportLib::TextStyle* style) const;
    QString getTruncatedPlayerName(const PlayerPair& pp, double maxWidth, SimpleReportLib::TextStyle* style) const;
    void drawWinnerNameOnTerminator(const QPointF& txtBottomCenter, const PlayerPair& pp, double gridWidth, SimpleReportLib::TextStyle* style) const;

    QString determineSymbolicPlayerPairDisplayText(const BracketVisElement& el, int pos) const;
    int determineEffectivePlayerPairId(const BracketVisElement& el, int pos) const;
    void printHeaderAndFooterOnAllPages() const;
  };

}
#endif // BRACKETSHEET_H
