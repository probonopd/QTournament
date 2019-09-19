/*
 *    This is QTournament, a badminton tournament management program.
 *    Copyright (C) 2014 - 2019  Volker Knollmann
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

#include <tuple>

#include <QList>

#include "ResultsAndNextMatches.h"
#include <SimpleReportGeneratorLib/SimpleReportGenerator.h>
#include <SimpleReportGeneratorLib/TableWriter.h>

#include "CatRoundStatus.h"
#include "ui/GuiHelpers.h"
#include "RankingMngr.h"
#include "RankingEntry.h"
#include "MatchMngr.h"
#include "MatchGroup.h"

namespace QTournament
{


ResultsAndNextMatches::ResultsAndNextMatches(const TournamentDB& _db, const QString& _name, const Category& _cat, int _round)
  :AbstractReport(_db, _name), cat(_cat), round(_round)
{
  roundOffset = cat.getParameter_int(CatParameter::FirstRoundOffset);

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
  MatchMngr mm{db};

  // prepare a subheader if we are in KO-rounds
  MatchGroupList mgl = mm.getMatchGroupsForCat(cat, round);
  QString subHeader = QString();
  if (mgl.size() == 1)
  {
    int grpNum = mgl.at(0).getGroupNumber();
    MatchSystem mSys = cat.getMatchSystem();
    if ((grpNum < 0) && (grpNum >= GroupNum_L16) &&
        ((mSys == MatchSystem::GroupsWithKO) || (mSys == MatchSystem::Bracket)))
    {
      subHeader = GuiHelpers::groupNumToLongString(mgl.at(0).getGroupNumber());
    }
  }

  upSimpleReport result = createEmptyReport_Portrait();
  QString repName = cat.getName() + " -- ";
  if (round > 0)
  {
    repName += tr("Results of Round ") + QString::number(round + roundOffset) + tr(" and Next Matches");
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
    loc += tr("after round ") + QString::number(round + roundOffset);
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
  MatchMngr mm{db};
  MatchGroupList mgl = mm.getMatchGroupsForCat(cat, round);
  MatchList allMatches;
  for (MatchGroup mg : mgl)
  {
    for (Match& m : mg.getMatches()) allMatches.push_back(m);
  }

  std::sort(allMatches.begin(), allMatches.end(), getSortFunction_MatchByGroupAndNumber());

  printIntermediateHeader(rep, tr("Results of Round ") + QString::number(round + roundOffset));
  printMatchList(rep, allMatches, PlayerPairList(), tr("Results of Round ") + QString::number(round + roundOffset) + tr(" (cont.)"), true, true);
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
  MatchMngr mm{db};
  MatchGroupList mgl = mm.getMatchGroupsForCat(cat, round+1);
  MatchList allMatches;
  bool isAllScheduled = true;
  for (MatchGroup mg : mgl)
  {
    ObjState mgStat = mg.getState();
    if ((mgStat != ObjState::MG_Scheduled) && (mgStat != ObjState::MG_Finished))
    {
      isAllScheduled = false;
      break;
    }
    for (Match& m : mg.getMatches()) allMatches.push_back(m);
  }

  // if there are unscheduled match groups, print nothing at all
  if ((!isAllScheduled) || (allMatches.empty()))
  {
    rep->writeLine(tr("Matches for next round are not yet scheduled."));
    return;
  }

  // erase all walkovers from the list
  auto it = allMatches.begin();
  while (it != allMatches.end())
  {
    const Match& ma = *it;
    if (ma.isWonByWalkover())
    {
      it = allMatches.erase(it);
    } else {
      ++it;
    }
  }

  // sort matches by group and number
  std::sort(allMatches.begin(), allMatches.end(), getSortFunction_MatchByGroupAndNumber());

  // determine a list of all players having a bye
  PlayerPairList byeList;
  PlayerPairList playingList;
  for (const Match& ma : allMatches)
  {
    playingList.push_back(ma.getPlayerPair1());
    playingList.push_back(ma.getPlayerPair2());
  }
  auto specialCat = cat.convertToSpecializedObject();
  Error e;
  PlayerPairList remainingPlayers = specialCat->getRemainingPlayersAfterRound(round, &e);
  if (e == Error::OK)
  {
    for (PlayerPair pp : remainingPlayers)
    {
      if (std::find(playingList.begin(), playingList.end(), pp) != playingList.end()) continue;
      byeList.push_back(pp);
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
