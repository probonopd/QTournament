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

#ifndef MATCHMATRIX_H
#define MATCHMATRIX_H

#include <memory>
#include <tuple>

#include <QObject>

#include "AbstractReportElement.h"
#include "Category.h"
#include "Match.h"

class MatrixGrid
{
public:
  MatrixGrid(double _width, int _nCols, double _rowHeight);
  MatrixGrid(const QPointF& _topLeft, double _width, int _nCols, double _rowHeight);
  QRectF getCell(int row, int col) const;
  QSizeF getGridSize(int nRows) const;
  QRectF getGridRect(int nRows) const;
  QRectF getGridRegion(int startRow, int startCol, int rowSpan, int colSpan) const;

private:
  QPointF topLeft;
  double width;
  int nCols;
  double rowHeight;
  double colWidth;
  QSizeF cellSize;
};

//----------------------------------------------------------------------------

class MatchMatrix : public QObject, AbstractReportElement
{
  Q_OBJECT

public:
  enum class CellContentType {
    Title,
    Header,
    Score,
    MatchScore,
    Empty
  };

  static constexpr double GapTextToGrid_mm = 1.0;

  MatchMatrix(SimpleReportLib::SimpleReportGenerator* _rep, const QString& tabName, const QTournament::Category& _cat, int _round, int _grpNum = -1);
  virtual QRectF plot(const QPointF& topLeft = QPointF(-1, -1));
  virtual ~MatchMatrix(){}

protected:
  QString tableName;
  QTournament::Category cat;
  int round;
  int grpNum;
  bool showMatchNumbersOnly;

  std::optional<QTournament::Match> getMatchForCell(const QTournament::PlayerPairList& ppList, int row, int col, int minRound, int maxRound) const;
  QStringList getSortedMatchScoreStrings(const QTournament::Match& ma, const QTournament::PlayerPair& ppRow, const QTournament::PlayerPair& ppCol) const;
  std::tuple<CellContentType, QString> getCellContent(const QTournament::PlayerPairList& ppList, int row, int col, int minRound, int maxRound) const;
  QString getTruncatedPlayerNames(const QTournament::PlayerPair& pp, const SimpleReportLib::TextStyle* style, double maxWidth) const;
};

#endif // MATCHMATRIX_H
