#ifndef BRACKETSHEET_H
#define BRACKETSHEET_H

#include <functional>
#include <tuple>

#include <QObject>

#include "reports/AbstractReport.h"
#include "Tournament.h"
#include "TournamentDB.h"
#include "TournamentDataDefs.h"

using namespace dbOverlay;

namespace QTournament
{
  class BracketSheet : public QObject, public AbstractReport
  {
    Q_OBJECT

  public:
    BracketSheet(TournamentDB* _db, const QString& _name, const Category& _cat);

    virtual upSimpleReport regenerateReport() const override;
    virtual QStringList getReportLocators() const override;

  private:
    Category cat;
    DbTab tabVis;

    tuple<double, double> determineGridSize(const upSimpleReport& rep) const;

  };

}
#endif // BRACKETSHEET_H
