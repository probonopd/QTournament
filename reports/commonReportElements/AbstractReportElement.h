#ifndef ABSTRACT_REPORT_ELEMENT__H
#define ABSTRACT_REPORT_ELEMENT__H

#include <QPointF>

#include "SimpleReportGenerator.h"

using namespace SimpleReportLib;

class AbstractReportElement
{

public:
  AbstractReportElement(SimpleReportGenerator* _rep);
  virtual void plot(const QPointF& topLeft = QPointF(-1, -1)) = 0;
  virtual ~AbstractReportElement() {}

protected:
  SimpleReportGenerator* rep;
};

#endif
