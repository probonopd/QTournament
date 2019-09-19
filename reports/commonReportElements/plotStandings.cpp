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

#include <SimpleReportGeneratorLib/TableWriter.h>

#include "plotStandings.h"

using namespace SimpleReportLib;
using namespace QTournament;

plotStandings::plotStandings(SimpleReportGenerator* _rep, const RankingEntryList& _rel, const QString& tabName)
  :AbstractReportElement(_rep), rel(_rel), tableName(tabName)
{

}

//----------------------------------------------------------------------------

QRectF plotStandings::plot(const QPointF& topLeft)
{
  // prepare a table for the standings
  SimpleReportLib::TabSet ts;
  ts.addTab(6, SimpleReportLib::TAB_JUSTIFICATION::TAB_RIGHT);  // the rank
  ts.addTab(10, SimpleReportLib::TAB_JUSTIFICATION::TAB_LEFT);   // the name
  ts.addTab(75, SimpleReportLib::TAB_JUSTIFICATION::TAB_LEFT);   // the team
  // a pair of three tabs for each matches, games and points
  for (int i=0; i< 3; ++i)
  {
    double colonPos = 150 + i*15.0;
    ts.addTab(colonPos - 1.0,  SimpleReportLib::TAB_RIGHT);  // first number
    ts.addTab(colonPos,  SimpleReportLib::TAB_CENTER);  // colon
    ts.addTab(colonPos + 1.0,  SimpleReportLib::TAB_LEFT);  // second number
  }
  SimpleReportLib::TableWriter tw(ts);
  tw.setHeader(1, tr("Rank"));
  tw.setHeader(2, tr("Player"));
  tw.setHeader(3, tr("Team"));
  tw.setHeader(5, tr("Matches"));
  tw.setHeader(8, tr("Games"));
  tw.setHeader(11, tr("Points"));

  bool hasAtLeastOneEntry = false;
  for (RankingEntry re : rel)
  {
    QStringList rowContent;
    rowContent << "";   // first column is unused

    // skip entries without a valid rank
    int curRank = re.getRank();
    if (curRank == RankingEntry::NoRankAssigned) continue;
    hasAtLeastOneEntry = true;

    rowContent << QString::number(curRank);

    auto pp = re.getPlayerPair();
    rowContent << (pp ? pp->getDisplayName(53) : "??");
    rowContent << (pp ? pp->getDisplayName_Team(53) : "??");

    if (pp)
    {
      // TODO: this doesn't work if draw matches are allowed!
      auto matchStats = re.getMatchStats();
      rowContent << QString::number(get<0>(matchStats));
      rowContent << ":";
      rowContent << QString::number(get<2>(matchStats));

      auto gameStats = re.getGameStats();
      rowContent << QString::number(get<0>(gameStats));
      rowContent << ":";
      rowContent << QString::number(get<1>(gameStats));

      auto pointStats = re.getPointStats();
      rowContent << QString::number(get<0>(pointStats));
      rowContent << ":";
      rowContent << QString::number(get<1>(pointStats));
    }

    tw.appendRow(rowContent);
  }

  tw.setNextPageContinuationCaption(tableName + tr(" (cont.)"));
  if (hasAtLeastOneEntry)
  {
    tw.write(rep);
  } else {
    rep->writeLine(tr("There are no standings available in this round."));
  }

  return QRectF();
}
