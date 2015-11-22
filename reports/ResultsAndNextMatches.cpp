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
  // if "round" is zero, we only print the first matches
  // if round is greater than zero, we print a normal report

  if (round < 0)
  {
    throw std::runtime_error("Requested results / NextMatches for invalid round.");
  }

  if (round > 0)
  {
    // make sure that the requested round is already finished
    CatRoundStatus crs = cat.getRoundStatus();
    if (round <= crs.getFinishedRoundsCount()) return; // okay, we're in one of the finished rounds

    throw std::runtime_error("Requested results / NextMatches for unfinished round.");
  }
}

//----------------------------------------------------------------------------

upSimpleReport ResultsAndNextMatches::regenerateReport()
{
  MatchMngr* mm = Tournament::getMatchMngr();

  // prepare a subheader if we are in KO-rounds
  MatchGroupList mgl = mm->getMatchGroupsForCat(cat, round);
  QString subHeader = QString();
  if (mgl.size() == 1)
  {
    int grpNum = mgl.at(0).getGroupNumber();
    MATCH_SYSTEM mSys = cat.getMatchSystem();
    if ((grpNum < 0) && (grpNum >= GROUP_NUM__L16) &&
        ((mSys == GROUPS_WITH_KO) || (mSys == SINGLE_ELIM)))
    {
      subHeader = GuiHelpers::groupNumToLongString(mgl.at(0).getGroupNumber());
    }
  }

  upSimpleReport result = createEmptyReport_Portrait();
  QString repName = cat.getName() + " -- ";
  if (round > 0)
  {
    repName += tr("Results of Round ") + QString::number(round) + tr(" and Next Matches");
  } else {
    repName += tr("First Matches");
  }
  setHeaderAndHeadline(result.get(), repName, subHeader);

  // print results, but only after a "real" round, not for the initial match list
  if (round > 0)
  {
    printResultPart(result);

    result->skip(5.0);
  }

  printNextMatchPart(result);

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
  if (round > 0)
  {
    loc += tr("after round ") + QString::number(round);
  } else {
    loc += tr("initial matches");
  }

  result.append(loc);

  return result;
}

//----------------------------------------------------------------------------

void ResultsAndNextMatches::printResultPart(upSimpleReport& rep) const
{
  // collect all matches in this round
  MatchMngr* mm = Tournament::getMatchMngr();
  MatchGroupList mgl = mm->getMatchGroupsForCat(cat, round);
  MatchList allMatches;
  for (MatchGroup mg : mgl)
  {
    allMatches.append(mg.getMatches());
  }

  std::sort(allMatches.begin(), allMatches.end(), getSortFunction_MatchByGroupAndNumber());

  printIntermediateHeader(rep, tr("Results of Round ") + QString::number(round));
  printMatchList(rep, allMatches, PlayerPairList(), tr("Results of Round ") + QString::number(round) + tr(" (cont.)"), true, true);
}

//----------------------------------------------------------------------------

void ResultsAndNextMatches::printNextMatchPart(upSimpleReport& rep) const
{
  if (round > 0)
  {
    printIntermediateHeader(rep, tr("Next Matches"));
  }

  // are there any more matches after this round?
  CatRoundStatus crs = cat.getRoundStatus();
  if (round == crs.getTotalRoundsCount())
  {
    rep->writeLine(tr("There are no more matches in this category."));
    return;
  }

  // collect all matches for the next round
  MatchMngr* mm = Tournament::getMatchMngr();
  MatchGroupList mgl = mm->getMatchGroupsForCat(cat, round+1);
  MatchList allMatches;
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
    rep->writeLine(tr("Matches for next round are not yet scheduled."));
    return;
  }

  // sort matches by group and number
  std::sort(allMatches.begin(), allMatches.end(), getSortFunction_MatchByGroupAndNumber());

  // determine a list of all players having a bye
  PlayerPairList byeList;
  PlayerPairList playingList;
  for (Match ma : allMatches)
  {
    playingList.append(ma.getPlayerPair1());
    playingList.append(ma.getPlayerPair2());
  }
  auto specialCat = cat.convertToSpecializedObject();
  ERR e;
  PlayerPairList remainingPlayers = specialCat->getRemainingPlayersAfterRound(round, &e);
  if (e == OK)
  {
    for (PlayerPair pp : remainingPlayers)
    {
      if (playingList.contains(pp)) continue;
      byeList.append(pp);
    }
  }

  printMatchList(rep, allMatches, byeList, tr("Next Matches") + tr(" (cont.)"), false, true);
}

//----------------------------------------------------------------------------

std::function<bool (Match& ma1, Match& ma2)> ResultsAndNextMatches::getSortFunction_MatchByGroupAndNumber() const
{
  return [](Match& ma1, Match& ma2){
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
}

//----------------------------------------------------------------------------

//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


}
