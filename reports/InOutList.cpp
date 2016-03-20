/*
 *    This is QTournament, a badminton tournament management program.
 *    Copyright (C) 2014 - 2015  Volker Knollmann
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

#include "InOutList.h"
#include "SimpleReportGenerator.h"
#include "TableWriter.h"

#include "CatRoundStatus.h"
#include "ui/GuiHelpers.h"
#include "RankingMngr.h"
#include "RankingEntry.h"
#include "KO_Config.h"

namespace QTournament
{


InOutList::InOutList(TournamentDB* _db, const QString& _name, const Category& _cat, int _round)
  :AbstractReport(_db, _name), cat(_cat), round(_round)
{
  if (!isValidCatRoundCombination(_cat, _round))
  {
    throw std::runtime_error("Requested in-out-list for invalid category and/or round");
  }
}

//----------------------------------------------------------------------------

upSimpleReport InOutList::regenerateReport()
{
  QString repName = cat.getName() + " -- " + tr("Knocked-out players after round ") + QString::number(round);
  upSimpleReport result = createEmptyReport_Portrait();

  setHeaderAndHeadline(result.get(), repName);

  // retrieve the "in and out" lists
  ERR err;
  auto specialCat = cat.convertToSpecializedObject();
  PlayerPairList inList = specialCat->getRemainingPlayersAfterRound(round, &err);
  if (err != OK)
  {
    result->writeLine(tr("An error occurred during report generation."));
    return result;
  }
  PlayerPairList outList = specialCat->getEliminatedPlayersAfterRound(round, &err);
  if (err != OK)
  {
    result->writeLine(tr("An error occurred during report generation."));
    return result;
  }

  if (outList.empty())
  {
    result->writeLine(tr("All players are still in the game, no knock-outs yet."));
  } else {
    // sort the outList be the number of rounds that each player survived
    auto tnmt = Tournament::getActiveTournament();
    RankingMngr* rm = tnmt->getRankingMngr();
    QHash<int, int> ppId2Rounds;
    for (PlayerPair pp : outList)
    {
      int roundCount = rm->getHighestRoundWithRankingEntryForPlayerPair(cat, pp);
      ppId2Rounds.insert(pp.getPairId(), roundCount);
    }
    std::sort(outList.begin(), outList.end(), [&](PlayerPair& pp1, PlayerPair& pp2) {
      int roundsPP1 = ppId2Rounds.value(pp1.getPairId());
      int roundsPP2 = ppId2Rounds.value(pp2.getPairId());
      return roundsPP1 < roundsPP2;
    });

    // prepare a table of the knocked-out players
    QStringList header;
    header.append(tr("Name"));
    header.append(tr("Team"));
    header.append(tr("Rounds played"));
    SimpleReportLib::TabSet ts;
    ts.addTab(90, SimpleReportLib::TAB_JUSTIFICATION::TAB_LEFT);
    ts.addTab(145, SimpleReportLib::TAB_JUSTIFICATION::TAB_LEFT);
    SimpleReportLib::TableWriter tw(ts);
    tw.setHeader(header);

    // fill the table
    for (PlayerPair pp : outList)
    {
      QStringList rowContent;
      rowContent << pp.getDisplayName();
      rowContent << pp.getDisplayName_Team();
      rowContent << QString::number(ppId2Rounds.value(pp.getPairId()));
      tw.appendRow(rowContent);
    }

    // put the table below an extra header
    printIntermediateHeader(result, "Knocked-out Players");
    tw.setNextPageContinuationCaption(tr("List of knocked-out players (cont.)"));
    tw.write(result.get());

    // prepare a tab for the remaining players
    header.removeLast();
    tw = SimpleReportLib::TableWriter(ts);
    tw.setHeader(header);

    // fill the table
    for (PlayerPair pp : inList)
    {
      QStringList rowContent;
      rowContent << pp.getDisplayName();
      rowContent << pp.getDisplayName_Team();
      tw.appendRow(rowContent);
    }

    // put the table below an extra header
    printIntermediateHeader(result, "Remaining Players");
    tw.setNextPageContinuationCaption(tr("List of remaining players (cont.)"));
    tw.write(result.get());
  }

  // set header and footer
  setHeaderAndFooter(result, repName);

  return result;
}

//----------------------------------------------------------------------------

QStringList InOutList::getReportLocators() const
{
  QStringList result;

  QString loc = tr("In-Out-List::");
  loc += cat.getName() + "::";
  loc += tr("after round ") + QString::number(round);

  result.append(loc);

  return result;
}

//----------------------------------------------------------------------------

bool InOutList::isValidCatRoundCombination(const Category& _cat, int _round)
{
  // we must be beyond CONFIG for this report to make any sense at all
  OBJ_STATE catState = _cat.getState();
  if ((catState == STAT_CAT_CONFIG) || (catState == STAT_CAT_FROZEN))
  {
    return false;
  }

  // make sure that the requested round is already finished
  CatRoundStatus crs = _cat.getRoundStatus();
  if (_round > crs.getFinishedRoundsCount())
  {
    return false;
  }

  // if we're in match system "round robin with KO rounds", this report
  // makes only sense after the last round robin round
  MATCH_SYSTEM mSys = _cat.getMatchSystem();
  if (mSys == GROUPS_WITH_KO)
  {
    KO_Config cfg = KO_Config(_cat.getParameter_string(GROUP_CONFIG));
    if (_round < cfg.getNumRounds())
    {
      return false;
    }

    // we're good to go
    return true;
  }

  // for elimination contests, we can always generate in-out-lists
  if (mSys == SINGLE_ELIM)
  {
    return true;
  }

  // a "catch all":
  // if we couldn't validate the request up to here, it's probably invalid
  return false;
}

//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


}
