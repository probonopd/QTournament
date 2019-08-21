/*
 *    This is QTournament, a badminton tournament management program.
 *    Copyright (C) 2014 - 2017  Volker Knollmann
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ABSTRACTREPORT_H
#define ABSTRACTREPORT_H

#include <memory>

#include <QString>

#include <SqliteOverlay/KeyValueTab.h>

#include "TournamentDB.h"
#include <SimpleReportGeneratorLib/SimpleReportGenerator.h>
#include "Match.h"
#include "MatchGroup.h"


namespace QTournament
{

  using upSimpleReport = std::unique_ptr<SimpleReportLib::SimpleReportGenerator>;
  using spSimpleReport = shared_ptr<SimpleReportLib::SimpleReportGenerator>;

  class AbstractReport
  {
  public:
    static constexpr double WidthA4_mm = 210.0;
    static constexpr double HeightA4_mm = 297.0;
    static constexpr double DefaultMargin_mm = 10.0;
    static constexpr double BeforeHeadlineSkip_mm = 2.0;
    static constexpr double AfterHeadlineSkip_mm = 5.0;
    static constexpr double HeadSubheadSkip_mm = 0.5;
    static constexpr double SkipBeforeIntermediaHeader_mm = 4.0;
    static constexpr double SkipAfterIntermediateHeader_mm = 1.0;

    static constexpr char HeadlineStyle[] = "Headline";
    static constexpr char SubheadStyle[] = "SubHeadline";
    static constexpr char IntermediateHeadlineStyle[] = "IntermediateHeadline";
    static constexpr char ResultsheetNameStyle[] = "ResultSheet_Name";
    static constexpr char ResultsheetTeamStyle[] = "ResultSheet_Team";
    static constexpr char ResultsheetGamelabelStyle[] = "ResultSheet_GameLabel";
    static constexpr char BoldStyle[] = "Bold";

    AbstractReport(const QTournament::TournamentDB& _db, const QString& _name);
    virtual ~AbstractReport();

    virtual upSimpleReport regenerateReport() { throw std::runtime_error("Unimplemented Method: regenerateReport"); }
    virtual QStringList getReportLocators() const { throw std::runtime_error("Unimplemented Method: getReportLocators"); }

    upSimpleReport createEmptyReport_Portrait() const;
    upSimpleReport createEmptyReport_Landscape() const;

    QString getName() const;
    void setHeaderAndHeadline(SimpleReportLib::SimpleReportGenerator* rep, const QString& headline, const QString& subHead=QString()) const;

  protected:
    const QTournament::TournamentDB& db;
    QString name;
    SqliteOverlay::KeyValueTab cfg;

    void prepStyles(upSimpleReport& rep) const;
    void printIntermediateHeader(upSimpleReport& rep, const QString& txt, double skipBefore__MM=SkipBeforeIntermediaHeader_mm) const;
    void printMatchList(upSimpleReport& rep, const MatchList& maList, const PlayerPairList& byeList, const QString& continuationString, bool withResults=false, bool withGroupColumn=false) const;
    void setHeaderAndFooter(upSimpleReport& rep, const QString& reportName);
  };

  using upAbstractReport = std::unique_ptr<AbstractReport>;
  using spAbstractReport = shared_ptr<AbstractReport>;

}

#endif // ABSTRACTREPORT_H
