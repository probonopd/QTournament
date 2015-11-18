#ifndef MATCHMATRIX_H
#define MATCHMATRIX_H

#include <QObject>

#include "AbstractReportElement.h"
#include "Category.h"

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

class MatchMatrix : public AbstractReportElement, public QObject
{
public:
  MatchMatrix(SimpleReportGenerator* _rep, const QString& tabName, const Category& _cat, int _grpNum=-1);
  virtual QRectF plot(const QPointF& topLeft = QPointF(-1, -1)) override;

protected:
  QString tableName;
  Category cat;
  int grpNum;
};

#endif // MATCHMATRIX_H
