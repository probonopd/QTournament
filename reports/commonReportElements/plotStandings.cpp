
#include "TableWriter.h"

#include "plotStandings.h"


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
  // a pair of three tabs for each matches, games and points
  for (int i=0; i< 3; ++i)
  {
    double colonPos = 120 + i*30.0;
    ts.addTab(colonPos - 1.0,  SimpleReportLib::TAB_RIGHT);  // first number
    ts.addTab(colonPos,  SimpleReportLib::TAB_CENTER);  // colon
    ts.addTab(colonPos + 1.0,  SimpleReportLib::TAB_LEFT);  // second number
  }
  SimpleReportLib::TableWriter tw(ts);
  tw.setHeader(1, tr("Rank"));
  tw.setHeader(2, tr("Player"));
  tw.setHeader(4, tr("Matches"));
  tw.setHeader(7, tr("Games"));
  tw.setHeader(10, tr("Points"));

  bool hasAtLeastOneEntry = false;
  for (RankingEntry re : rel)
  {
    QStringList rowContent;
    rowContent << "";   // first column is unused

    // skip entries without a valid rank
    int curRank = re.getRank();
    if (curRank == RankingEntry::NO_RANK_ASSIGNED) continue;
    hasAtLeastOneEntry = true;

    rowContent << QString::number(curRank);

    auto pp = re.getPlayerPair();
    rowContent << ((pp != nullptr) ? (*pp).getDisplayName() : "??");

    if (pp != nullptr)
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
