
#include "TableWriter.h"

#include "MatchMatrix.h"


MatchMatrix::MatchMatrix(SimpleReportGenerator* _rep, const QString& tabName, const Category& _cat, int _grpNum)
  :AbstractReportElement(_rep), tableName(tabName), cat(_cat), grpNum(_grpNum)
{

}

//----------------------------------------------------------------------------

QRectF MatchMatrix::plot(const QPointF& topLeft)
{
  QPointF origin = topLeft;

  if ((topLeft.x() < 0) && (topLeft.y() < 0))
  {
    origin = rep->getAbsCursorPos();
  }

  MatrixGrid grid{origin, rep->getUsablePageWidth(), 6, 10};
  for (int r=0; r < 6; ++r)
  {
    for (int c=0; c < 6; ++c)
    {
      auto cell = grid.getCell(r, c);
      QString txt = "%1, %2";
      txt = txt.arg(r).arg(c);
      rep->drawText(cell, RECT_CORNER::CENTER, RECT_CORNER::CENTER, txt, nullptr);
      rep->drawRect(cell);
    }
  }

  return grid.getGridRect(6);
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

MatrixGrid::MatrixGrid(double _width, int _nCols, double _rowHeight)
  :MatrixGrid(QPointF(0,0), _width, _nCols, _rowHeight)
{

}

//----------------------------------------------------------------------------

MatrixGrid::MatrixGrid(const QPointF& _topLeft, double _width, int _nCols, double _rowHeight)
  :topLeft(_topLeft), width(_width), nCols(_nCols), rowHeight(_rowHeight)
{
  if (width <= 0)
  {
    throw invalid_argument("Can't have negative or zero grid width");
  }

  if (nCols <= 0)
  {
    throw invalid_argument("Can't have negative or zero number of grid columns");
  }

  if (rowHeight < 0)
  {
    throw invalid_argument("Can't have negative or zero grid row height.");
  }

  colWidth = width / nCols;
  cellSize = QSizeF(colWidth, rowHeight);
}

//----------------------------------------------------------------------------

QRectF MatrixGrid::getCell(int row, int col) const
{
  if ((row < 0) || (col < 0)) return QRectF();

  QPointF cellTopLeft = topLeft + QPointF(colWidth * col, rowHeight * row);

  return QRectF(cellTopLeft, cellSize);
}

//----------------------------------------------------------------------------

QSizeF MatrixGrid::getGridSize(int nRows) const
{
  if (nRows < 1) return QSizeF();

  return QSizeF(colWidth * nCols, nRows * rowHeight);
}

//----------------------------------------------------------------------------

QRectF MatrixGrid::getGridRect(int nRows) const
{
  if (nRows < 1) return QRectF();

  return QRectF(topLeft, getGridSize(nRows));
}

//----------------------------------------------------------------------------

QRectF MatrixGrid::getGridRegion(int startRow, int startCol, int rowSpan, int colSpan) const
{
  if ((startRow < 0) || (startCol < 0) || (rowSpan < 1) || (colSpan < 1))
  {
    return QRectF();
  }

  auto startCell = getCell(startRow, startCol);

  return QRectF(startCell.topLeft(), QSize(colSpan * colWidth, rowSpan * rowHeight));
}
