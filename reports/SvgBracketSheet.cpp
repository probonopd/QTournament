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

#include <QList>

#include <SimpleReportGeneratorLib/SimpleReportGenerator.h>
#include <SimpleReportGeneratorLib/TableWriter.h>

#include "SvgBracketSheet.h"
#include "SvgBracket.h"
#include "MatchMngr.h"
#include "CatMngr.h"
#include "../HelperFunc.h"

namespace QTournament
{


SvgBracketSheet::SvgBracketSheet(const TournamentDB& _db, const QString& _name, const Category& _cat, int _round)
  :AbstractReport(_db, _name), cat{_cat}, round{_round}
{
}

//----------------------------------------------------------------------------

SvgBracketSheet::~SvgBracketSheet()
{

}

//----------------------------------------------------------------------------

upSimpleReport SvgBracketSheet::regenerateReport()
{
  upSimpleReport result = createEmptyReport_Landscape();

  // get the bracket configuration
  auto brMatchSys = static_cast<SvgBracketMatchSys>(cat.getParameter_int(CatParameter::BracketMatchSystem));
  CatMngr cm{db};
  const auto seeding = cm.getSeeding(cat);

  // generate the resulting SVG for several cases
  std::vector<SvgPageDescr> pages;
  if (round == 0)
  {
    pages = SvgBracket::substSvgBracketTags(brMatchSys, seeding, commonTags(), true);
  }

  // append all pages to the report
  bool needsNewPage{false};
  for (const auto& pg : pages)
  {
    if (needsNewPage) result->startNextPage();
    result->addSVG_byData_setW(QPointF{0,0}, SimpleReportLib::RECT_CORNER::TOP_LEFT, pg.content, pg.width_mm);
    needsNewPage = true;
  }

  return result;
}

//----------------------------------------------------------------------------

QStringList SvgBracketSheet::getReportLocators() const
{
  QStringList result;

  QString loc = tr("Brackets::");
  loc += cat.getName() + "::";

  if (round == 0)
  {
    loc += tr("Initial seeding");
  }
  if (round < 0)
  {
    loc += tr("Current");
  }
  if (round > 0)
  {
    loc += tr("after round ") + QString::number(round);
  }

  result.append(loc);
  return result;
}

//----------------------------------------------------------------------------

SvgBracket::CommonBracketTags SvgBracketSheet::commonTags() const
{
  SvgBracket::CommonBracketTags result;

  auto cfg = SqliteOverlay::KeyValueTab{db, TabCfg};
  result.tnmtName = cfg[CfgKey_TnmtName];
  result.club = cfg[CfgKey_TnmtOrga];
  result.catName = QString2StdString(cat.getName());

  if (round == 0)
  {
    result.subtitle = QString2StdString(tr("Initial seeding"));
  }
  if (round < 0)
  {
    result.subtitle = QString2StdString(tr("Current status"));
  }
  if (round > 0)
  {
    QString tmp = tr("After round %1");
    tmp = tmp.arg(round);
    result.subtitle = QString2StdString(tmp);
  }

  // leave date and time empty

  return result;
}

//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


}
