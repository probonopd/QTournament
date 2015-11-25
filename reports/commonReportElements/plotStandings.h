#ifndef PLOTSTANDINGS_H
#define PLOTSTANDINGS_H

#include <QObject>

#include "AbstractReportElement.h"
#include "RankingMngr.h"
#include "RankingEntry.h"

using namespace QTournament;

class plotStandings : public QObject, AbstractReportElement
{
  Q_OBJECT

public:
  plotStandings(SimpleReportGenerator* _rep, const RankingEntryList& _rel, const QString& tabName);
  virtual QRectF plot(const QPointF& topLeft = QPointF(-1, -1));
  virtual ~plotStandings() {}

protected:
  RankingEntryList rel;
  QString tableName;
};

#endif // PLOTSTANDINGS_H
