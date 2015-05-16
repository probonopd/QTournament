#include <tuple>

#include <QList>

#include "ResultsAndNextMatches.h"
#include "SimpleReportGenerator.h"
#include "TableWriter.h"

#include "CatRoundStatus.h"
#include "ui/GuiHelpers.h"
#include "RankingMngr.h"
#include "RankingEntry.h"

namespace QTournament
{


ResultsAndNextMatches::ResultsAndNextMatches(TournamentDB* _db, const QString& _name, const Category& _cat, int _round)
  :AbstractReport(_db, _name), cat(_cat), round(_round)
{
  // make sure that the requested round is already finished
  CatRoundStatus crs = cat.getRoundStatus();
  if (round <= crs.getFinishedRoundsCount()) return; // okay, we're in one of the finished rounds

  throw std::runtime_error("Requested results / NextMatches for unfinished round.");
}

//----------------------------------------------------------------------------

upSimpleReport ResultsAndNextMatches::regenerateReport() const
{
  // collect all matches in this round
  MatchMngr* mm = Tournament::getMatchMngr();
  MatchGroupList mgl = mm->getMatchGroupsForCat(cat, round);
  MatchList allMatches;
  for (MatchGroup mg : mgl)
  {
    allMatches.append(mg.getMatches());
  }

  // helper function to sort matches by group first, and then by match number
  auto sortMatchByGroupAndNumber = [](Match& ma1, Match& ma2){
    int mg1Num = ma1.getMatchGroup().getGroupNumber();
    int mg2Num = ma2.getMatchGroup().getGroupNumber();
    if (mg1Num < mg2Num)
    {
      return true;
    }
    if (mg1Num > mg2Num)
    {
      return false;
    }

    // group numbers are equal. so we sort by match numbers
    return (ma1.getMatchNumber() < ma2.getMatchNumber());
  };

  std::sort(allMatches.begin(), allMatches.end(), sortMatchByGroupAndNumber);

  // prepare a subheader if we are in KO-rounds
  QString subHeader = QString();
  if ((mgl.size() == 1) && (mgl.at(0).getGroupNumber() < 0))
  {
    subHeader = GuiHelpers::groupNumToLongString(mgl.at(0).getGroupNumber());
  }

  upSimpleReport result = createEmptyReport_Portrait();
  QString repName = cat.getName() + tr(" -- Results of Round ") + QString::number(round) + tr(" and Next Matches");
  setHeaderAndHeadline(result.get(), repName, subHeader);

  printIntermediateHeader(result, tr("Results of Round ") + QString::number(round));
  printMatchList(result, allMatches, tr("Results of Round ") + QString::number(round) + tr(" (cont.)"), true, true);

  result->skip(5.0);

  printIntermediateHeader(result, tr("Next Matches"));

  // are there any more matches after this round?
  CatRoundStatus crs = cat.getRoundStatus();
  if (round == crs.getTotalRoundsCount())
  {
    result->writeLine(tr("There are no more matches in this category."));
    // set header and footer
    setHeaderAndFooter(result, repName);
    return result;
  }

  // collect all matches for the next round
  mgl = mm->getMatchGroupsForCat(cat, round+1);
  allMatches.clear();
  bool isAllScheduled = true;
  for (MatchGroup mg : mgl)
  {
    OBJ_STATE mgStat = mg.getState();
    if ((mgStat != STAT_MG_SCHEDULED) && (mgStat != STAT_MG_FINISHED))
    {
      isAllScheduled = false;
      break;
    }
    allMatches.append(mg.getMatches());
  }

  // if there are unscheduled match groups, print nothing at all
  if ((!isAllScheduled) || (allMatches.isEmpty()))
  {
    result->writeLine(tr("Matches for next round are not yet scheduled."));
    // set header and footer
    setHeaderAndFooter(result, repName);
    return result;
  }

  // sort matches by group and number
  std::sort(allMatches.begin(), allMatches.end(), sortMatchByGroupAndNumber);

  printMatchList(result, allMatches, tr("Next Matches") + tr(" (cont.)"), false, true);

  // set header and footer
  setHeaderAndFooter(result, repName);

  return result;
}

//----------------------------------------------------------------------------

QStringList ResultsAndNextMatches::getReportLocators() const
{
  QStringList result;

  QString loc = tr("Results and next matches::");
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
