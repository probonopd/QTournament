#ifndef MATCHMATRIX_H
#define MATCHMATRIX_H

#include <QObject>

#include "AbstractReportElement.h"
#include "Category.h"

using namespace QTournament;

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
