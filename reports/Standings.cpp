#include <tuple>

#include <QList>

#include "Standings.h"
#include "SimpleReportGenerator.h"
#include "TableWriter.h"

#include "CatRoundStatus.h"
#include "ui/GuiHelpers.h"
#include "RankingMngr.h"
#include "RankingEntry.h"

namespace QTournament
{


Standings::Standings(TournamentDB* _db, const QString& _name, const Category& _cat, int _round)
  :AbstractReport(_db, _name), cat(_cat), round(_round)
{
  // make sure that the requested round is already finished
  CatRoundStatus crs = cat.getRoundStatus();
  if (round <= crs.getFinishedRoundsCount()) return; // okay, we're in one of the finished rounds

  throw std::runtime_error("Requested standings report for unfinished round.");
}

//----------------------------------------------------------------------------

upSimpleReport Standings::regenerateReport() const
{
  // retrieve the ranking(s) for this round
  RankingMngr* rm = Tournament::getRankingMngr();
  RankingEntryListList rll = rm->getSortedRanking(cat, round);

  QString repName = cat.getName() + " -- " + tr("Standings after round ") + QString::number(round);
  upSimpleReport result = createEmptyReport_Portrait();

  // return an empty report if we have no standings yet
  if (rll.isEmpty() || ((rll.size() == 1) && (rll.at(0).size() == 0)))
  {
    setHeaderAndHeadline(result.get(), repName);
    result->writeLine(tr("There are no standings for this round yet."));
    return result;
  }

  // an internal marker if we are in round-robin matches or not
  bool isRoundRobin = (rll.size() > 1);

  // if we are in a "special round" like semi-finals, etc.
  // create a suitable sub-headline
  QString subHeader;
  if (!isRoundRobin)
  {
    MatchMngr* mm = Tournament::getMatchMngr();
    MatchGroupList mgl = mm->getMatchGroupsForCat(cat, round);
    int matchGroupNumber = mgl.at(0).getGroupNumber();
    if ((matchGroupNumber < 0) && (matchGroupNumber != GROUP_NUM__ITERATION))
    {
      subHeader = GuiHelpers::groupNumToLongString(mgl.at(0).getGroupNumber());
    }
  }
  setHeaderAndHeadline(result.get(), repName, subHeader);

  // dump all rankings to the report
  for (RankingEntryList rl : rll)
  {
    QString tableName = tr("Standings in category ") + cat.getName() + tr(" after round ") + QString::number(round);
    if (isRoundRobin)
    {
      // determine the group number
      // and print an intermediate header
      RankingEntry re = rl.at(0);
      QString hdr = tr("Group ") + QString::number(re.getGroupNumber());
      printIntermediateHeader(result, hdr);

      tableName += ", " + tr("Group ") + QString::number(re.getGroupNumber());
    }

    // prepare a table for the standings
    SimpleReportLib::TabSet ts;
    ts.addTab(2, SimpleReportLib::TAB_JUSTIFICATION::TAB_RIGHT);  // the rank
    ts.addTab(8, SimpleReportLib::TAB_JUSTIFICATION::TAB_LEFT);   // the name
    // a pair of three tabs for each matches, games and points
    for (int i=0; i< 3; ++i)
    {
      double colonPos = 100 + i*30.0;
      ts.addTab(colonPos - 1.0,  SimpleReportLib::TAB_RIGHT);  // first number
      ts.addTab(colonPos,  SimpleReportLib::TAB_CENTER);  // colon
      ts.addTab(colonPos + 1.0,  SimpleReportLib::TAB_LEFT);  // second number
    }
    SimpleReportLib::TableWriter tw(ts);
    tw.setHeader(0, tr("Player"));
    tw.setHeader(4, tr("Matches"));
    tw.setHeader(7, tr("Games"));
    tw.setHeader(10, tr("Points"));

    int curRank = 1;
    for (RankingEntry re : rl)
    {
      QStringList rowContent;
      rowContent << "";   // first column is unused
      rowContent << QString::number(curRank);
      rowContent << re.getPlayerPair().getDisplayName();

      // TODO: this doesn't work if we draw matches are allowed!
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

      tw.appendRow(rowContent);
      ++curRank;
    }

    tw.setNextPageContinuationCaption(tableName + tr(" (cont.)"));
    tw.write(result.get());
  }

  // set header and footer
  setHeaderAndFooter(result, repName);

  return result;
}

//----------------------------------------------------------------------------

QStringList Standings::getReportLocators() const
{
  QStringList result;

  QString loc = tr("Standings::");
  loc += cat.getName() + "::by round::";
  loc += tr("Round ") + QString::number(round);

  result.append(loc);

  return result;
}

//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


}
