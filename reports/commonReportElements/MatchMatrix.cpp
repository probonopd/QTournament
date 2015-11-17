
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
    origin.setY(rep->curY);
    origin.setX((rep->getPageWidth() - rep->getUsablePageWidth()) / 2.0);
  }

  rep->drawHorLine(origin.x(), origin.y(), 30);
  rep->drawHorLine(origin.x() + 30, origin.y(), 30);
  rep->drawVertLine(origin.x(), origin.y(), 10);
  rep->drawVertLine(origin.x() + 30, origin.y(), 10);

  return QRectF(origin, QSizeF(30, 10));

  return QRectF();
}
