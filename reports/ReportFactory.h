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

    static constexpr char REP__PARTLIST_BY_NAME[] = "ParticipantsListByName";
    static constexpr char REP__PARTLIST_BY_TEAM[] = "ParticipantsListByTeam";
    static constexpr char REP__PARTLIST_BY_CATEGORY[] = "ParticipantsListByCategory";
    static constexpr char REP__RESULTS[] = "Results";
    static constexpr char REP__RESULTS_BY_GROUP[] = "ResultsByGroup";

  private:
    TournamentDB* db;
  };

}
#endif // REPORTFACTORY_H
