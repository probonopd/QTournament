#ifndef RESULTSANDNEXTMATCHES_H
#define RESULTSANDNEXTMATCHES_H

#include <functional>

#include <QObject>

#include "reports/AbstractReport.h"
#include "Tournament.h"
#include "TournamentDB.h"
#include "TournamentDataDefs.h"

using namespace dbOverlay;

namespace QTournament
{
  class ResultsAndNextMatches : public QObject, public AbstractReport
  {
    Q_OBJECT

  public:
    ResultsAndNextMatches(TournamentDB* _db, const QString& _name, const Category& _cat, int _round);

    virtual upSimpleReport regenerateReport() const override;
    virtual QStringList getReportLocators() const override;

  private:
    Category cat;
    int round;
  };

}
#endif // RESULTSANDNEXTMATCHES_H
