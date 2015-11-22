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

    virtual upSimpleReport regenerateReport() override;
    virtual QStringList getReportLocators() const override;

  private:
    Category cat;
    int round;

    void printResultPart(upSimpleReport& rep) const;
    void printNextMatchPart(upSimpleReport& rep) const;

    std::function<bool (Match& ma1, Match& ma2)> getSortFunction_MatchByGroupAndNumber() const;
  };

}
#endif // RESULTSANDNEXTMATCHES_H
