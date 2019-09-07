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

#ifndef SVGBRACKETSHEET_H
#define SVGBRACKETSHEET_H

#include <functional>
#include <unordered_map>
#include <string>

#include <QObject>

#include "reports/AbstractReport.h"
#include "TournamentDB.h"
#include "TournamentDataDefs.h"
#include "SvgBracket.h"

namespace QTournament
{
  enum class BracketReportType
  {
    Seeding,   ///< show the initial seeding
    AfterRound,  ///< show the status after a given round
    Current   ///< show the status with all available matches filled in
  };

  class SvgBracketSheet : public QObject, public AbstractReport
  {
    Q_OBJECT

  public:
    /** \brief Ctor for a new bracket report
     */
    SvgBracketSheet(
        const QTournament::TournamentDB& _db,   ///< the database we're working on
        const QString& _name,   ///< the internal report name that uniquely identifies the report
        BracketReportType reportType,   ///< the type of bracket report
        const Category& _cat,   ///< the category for which we create the bracket
        Round _round   ///< the applicable round, if necessary
        );
    virtual ~SvgBracketSheet() override;

    virtual upSimpleReport regenerateReport() override;
    virtual QStringList getReportLocators() const override;

  protected:
    /** \returns a set of common bracket tags for this report
     */
    SvgBracket::CommonBracketTags commonTags() const;

    /** \brief prepares the bracket for the report mode "Seeding"
     */
    std::vector<SvgPageDescr> prepReport_Seeding(SimpleReportLib::SimpleReportGenerator* rep,   ///< pointer to the report we should be writing to
        const PlayerPairList& seeding   ///< the seeding for the category; is needed by all reports
        ) const;

    /** \brief prepares the bracket for the report mode "After round XXX"
     */
    std::vector<SvgPageDescr> prepReport_AfterRound(
        SimpleReportLib::SimpleReportGenerator* rep,   ///< pointer to the report we should be writing to
        const PlayerPairList& seeding   ///< the seeding for the category; is needed by all reports
        ) const;

    /** \brief prepares the bracket for the report mode "current status / everything"
     */
    std::vector<SvgPageDescr> prepReport_Current(
        SimpleReportLib::SimpleReportGenerator* rep,   ///< pointer to the report we should be writing to
        const PlayerPairList& seeding   ///< the seeding for the category; is needed by all reports
        ) const;

  private:
    const BracketReportType repType;
    const Category cat;
    const SvgBracketMatchSys msys;
    Round round;
    Round firstBracketRound{1};
  };

}
#endif // PARTICIPANTSLIST_H
