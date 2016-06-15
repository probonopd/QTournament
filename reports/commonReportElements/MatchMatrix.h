/*
 *    This is QTournament, a badminton tournament management program.
 *    Copyright (C) 2014 - 2016  Volker Knollmann
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

using namespace QTournament;

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
  enum class CELL_CONTENT_TYPE {
    TITLE,
    HEADER,
    SCORE,
    MATCH_NUMBER,
    EMPTY
  };

  static constexpr double GAP_TEXT_TO_GRID__MM = 1.0;

  MatchMatrix(SimpleReportGenerator* _rep, const QString& tabName, const Category& _cat, int _round, int _grpNum = -1);
  virtual QRectF plot(const QPointF& topLeft = QPointF(-1, -1));
  virtual ~MatchMatrix(){}

protected:
  QString tableName;
  Category cat;
  int round;
  int grpNum;
  bool showMatchNumbersOnly;

  upMatch getMatchForCell(const PlayerPairList& ppList, int row, int col, int minRound, int maxRound) const;
  QStringList getSortedMatchScoreStrings(const Match& ma, const PlayerPair& ppRow, const PlayerPair& ppCol) const;
  tuple<CELL_CONTENT_TYPE, QString> getCellContent(const PlayerPairList& ppList, int row, int col, int minRound, int maxRound) const;
  QString getTruncatedPlayerNames(const PlayerPair& pp, const TextStyle* style, double maxWidth) const;
};

#endif // MATCHMATRIX_H
