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

#include "SvgBracketSheet.h"
#include <SimpleReportGeneratorLib/SimpleReportGenerator.h>
#include <SimpleReportGeneratorLib/TableWriter.h>

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

  result->writeLine("Hello " + cat.getName() + ", Round " + QString::number(round));

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


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


}
