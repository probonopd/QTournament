#ifndef ABSTRACTREPORT_H
#define ABSTRACTREPORT_H

#include <memory>

#include <QString>

#include "TournamentDB.h"
#include "SimpleReportGenerator.h"


namespace QTournament
{

  typedef unique_ptr<SimpleReportLib::SimpleReportGenerator> upSimpleReport;
  typedef shared_ptr<SimpleReportLib::SimpleReportGenerator> spSimpleReport;

  class AbstractReport
  {
  public:
    static constexpr double A4_WIDTH__MM = 210.0;
    static constexpr double A4_HEIGHT__MM = 297.0;
    static constexpr double DEFAULT_MARGIN__MM = 10.0;

    AbstractReport(TournamentDB* _db, const QString& _name);
    virtual ~AbstractReport();

    virtual upSimpleReport regenerateReport() { throw std::runtime_error("Unimplemented Method: regenerateReport"); };
    virtual QStringList getReportLocators() { throw std::runtime_error("Unimplemented Method: getReportLocators"); };

    static upSimpleReport createEmptyReport_Portrait();

    QString getName() const;

  private:
    TournamentDB* db;
    QString name;
  };

  typedef unique_ptr<AbstractReport> upAbstractReport;
  typedef shared_ptr<AbstractReport> spAbstractReport;

}

#endif // ABSTRACTREPORT_H
