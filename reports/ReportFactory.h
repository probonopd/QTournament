#ifndef REPORTFACTORY_H
#define REPORTFACTORY_H

#include <memory>
#include <vector>

#include <QList>

#include "TournamentDB.h"
#include "AbstractReport.h"

namespace QTournament
{

  class ReportFactory
  {
  public:
    ReportFactory(TournamentDB* _db);
    ~ReportFactory();

    QStringList getReportCatalogue() const;
    upAbstractReport getReportByName(const QString& repName) const;

    // this works only with std::vector; QList<> won't compile... weird...
    std::vector<upAbstractReport> getMissingReports(const QStringList& existingReportNames) const;

    static const QString REP__PARTLIST_BY_NAME;

  private:
    TournamentDB* db;
  };

}
#endif // REPORTFACTORY_H
