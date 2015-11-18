#include <tuple>

#include <QList>

#include "MatrixAndStandings.h"
#include "SimpleReportGenerator.h"
#include "TableWriter.h"

#include "CatRoundStatus.h"
#include "ui/GuiHelpers.h"
#include "RankingMngr.h"
#include "RankingEntry.h"
#include "reports/commonReportElements/plotStandings.h"
#include "reports/commonReportElements/MatchMatrix.h"

namespace QTournament
{


MartixAndStandings::MartixAndStandings(TournamentDB* _db, const QString& _name, const Category& _cat, int _round)
  :AbstractReport(_db, _name), cat(_cat), round(_round)
{
  // make sure that the requested round is already finished
  CatRoundStatus crs = cat.getRoundStatus();
  if (round <= crs.getFinishedRoundsCount())
  {
    return; // okay, we're in one of the finished rounds
  } else {
    throw std::runtime_error("Requested matrix and standings report for unfinished round.");
  }

  // make sure this category is eligible for a matrix view
  MATCH_SYSTEM msys = cat.getMatchSystem();
  if ((msys != GROUPS_WITH_KO) && (msys != ROUND_ROBIN))
  {
    throw std::runtime_error("Requested matrix and standings report for invalid match type.");
  }

  // if we are in GROUPS_WITH_KO make sure that "round" is still in
  // round-robin-phase
  if (msys == GROUPS_WITH_KO)
  {
    KO_Config cfg = KO_Config(cat.getParameter_string(GROUP_CONFIG));
    int numGroupRounds = cfg.getNumRounds();
    if (crs.getFinishedRoundsCount() > numGroupRounds)
    {
      throw std::runtime_error("Requested matrix and standings report for elimination phase of category.");
    }

  }
}

//----------------------------------------------------------------------------

upSimpleReport MartixAndStandings::regenerateReport()
{
  // retrieve the ranking(s) for this round
  RankingMngr* rm = Tournament::getRankingMngr();
  RankingEntryListList rll = rm->getSortedRanking(cat, round);

  QString repName = cat.getName() + " -- " + tr("Matrix and standings after round ") + QString::number(round);
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

  setHeaderAndHeadline(result.get(), repName);

  // dump all rankings to the report
  int cnt=0;
  for (RankingEntryList rl : rll)
  {
    QString tableName = cat.getName();
    int grpNum = -1;
    if (isRoundRobin)
    {
      // determine the group number
      // and print an intermediate header
      RankingEntry re = rl.at(0);
      grpNum = re.getGroupNumber();
      QString tableName = tr("Group ") + QString::number(grpNum);
    }

    MatchMatrix matrix{result.get(), tableName, cat, grpNum};
    auto plotRect = matrix.plot();
    result->skip(plotRect.size().height() + 3.0);

    plotStandings elem{result.get(), rl, tableName};
    elem.plot();

    if ((cnt % 2) == 0)
    {
      result->skip(10);
    } else {
      result->startNextPage();
    }

    ++cnt;
  }

  // set header and footer
  setHeaderAndFooter(result, repName);

  return result;
}

//----------------------------------------------------------------------------

QStringList MartixAndStandings::getReportLocators() const
{
  QStringList result;

  QString loc = tr("Matrix and Standings::");
  loc += cat.getName() + "::";
  loc += tr("after round ") + QString::number(round);

  result.append(loc);

  return result;
}

//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


}
