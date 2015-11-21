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

class MatchMatrix : public AbstractReportElement, public QObject
{
public:
  enum class CELL_CONTENT_TYPE {
    TITLE,
    HEADER,
    SCORE,
    MATCH_NUMBER,
    EMPTY
  };

  static constexpr double MATCH_NUMBER_MARGIN__MM = 1.0;

  MatchMatrix(SimpleReportGenerator* _rep, const QString& tabName, const Category& _cat, int _round, int _grpNum = -1);
  virtual QRectF plot(const QPointF& topLeft = QPointF(-1, -1)) override;

protected:
  QString tableName;
  Category cat;
  int round;
  int grpNum;

  upMatch getMatchForCell(const PlayerPairList& ppList, int row, int col, int maxRound) const;
  QStringList getSortedMatchScoreStrings(const Match& ma, const PlayerPair& ppRow, const PlayerPair& ppCol) const;
  tuple<CELL_CONTENT_TYPE, QString> getCellContent(const PlayerPairList& ppList, int row, int col, int maxRound) const;
};

#endif // MATCHMATRIX_H
