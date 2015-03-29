#ifndef ABSTRACTREPORT_H
#define ABSTRACTREPORT_H

#include <memory>

#include <QString>

#include "TournamentDB.h"
#include "SimpleReportGenerator.h"
#include "Match.h"


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
    static constexpr double AFTER_HEADLINE_SKIP__MM = 5.0;
    static constexpr double HEAD_SUBHEAD_SKIP__MM = 0.5;
    static constexpr double SKIP_BEFORE_INTERMEDIATE_HEADER__MM = 4.0;
    static constexpr double SKIP_AFTER_INTERMEDIATE_HEADER__MM = 1.0;

    static constexpr char HEADLINE_STYLE[] = "Headline";
    static constexpr char SUBHEADLINE_STYLE[] = "SubHeadline";
    static constexpr char INTERMEDIATEHEADLINE_STYLE[] = "IntermediateHeadline";

    AbstractReport(TournamentDB* _db, const QString& _name);
    virtual ~AbstractReport();

    virtual upSimpleReport regenerateReport() const { throw std::runtime_error("Unimplemented Method: regenerateReport"); };
    virtual QStringList getReportLocators() const { throw std::runtime_error("Unimplemented Method: getReportLocators"); };

    upSimpleReport createEmptyReport_Portrait() const;

    QString getName() const;
    void setHeaderAndHeadline(SimpleReportLib::SimpleReportGenerator* rep, const QString& headline, const QString& subHead=QString()) const;

  protected:
    TournamentDB* db;
    QString name;

    void prepStyles(upSimpleReport& rep) const;
    void printIntermediateHeader(upSimpleReport& rep, const QString& txt, double skipBefore__MM=SKIP_BEFORE_INTERMEDIATE_HEADER__MM) const;
    void prepTabsForMatchResults(upSimpleReport& rep) const;
    void printMatchResult(upSimpleReport& rep, const Match& ma, const QString& continuationString) const;
  };

  typedef unique_ptr<AbstractReport> upAbstractReport;
  typedef shared_ptr<AbstractReport> spAbstractReport;

}

#endif // ABSTRACTREPORT_H
