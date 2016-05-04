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

#include "Standings.h"
#include "SimpleReportGenerator.h"
#include "TableWriter.h"

#include "CatRoundStatus.h"
#include "ui/GuiHelpers.h"
#include "RankingMngr.h"
#include "RankingEntry.h"
#include "reports/commonReportElements/plotStandings.h"
#include "MatchMngr.h"
#include "MatchGroup.h"

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

upSimpleReport Standings::regenerateReport()
{
  // retrieve the ranking(s) for this round
  RankingMngr rm{db};
  RankingEntryListList rll = rm.getSortedRanking(cat, round);

  QString repName = cat.getName() + " -- " + tr("Standings after round ") + QString::number(round);
  upSimpleReport result = createEmptyReport_Portrait();

  // return an empty report if we have no standings yet
  if (rll.empty() || ((rll.size() == 1) && (rll.at(0).size() == 0)))
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
    MatchMngr mm{db};
    MatchGroupList mgl = mm.getMatchGroupsForCat(cat, round);
    int matchGroupNumber = mgl.at(0).getGroupNumber();
    MATCH_SYSTEM mSys = cat.getMatchSystem();
    if ((matchGroupNumber < 0) && (matchGroupNumber != GROUP_NUM__ITERATION) &&
        ((mSys == GROUPS_WITH_KO) || (mSys == SINGLE_ELIM)))
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

    // plot the actual standings table
    plotStandings table{result.get(), rl, tableName};
    table.plot();

    result->skip(3.0);
  }

  // if we are in ranking matches, print a list of all best-case reachable places
  if ((cat.getMatchSystem() == RANKING) && (round != cat.getRoundStatus().getTotalRoundsCount()))
  {
    printIntermediateHeader(result, tr("Best case reachable places after this round"));
    printBestCaseList(result);
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
  loc += cat.getName() + "::";
  loc += tr("after round ") + QString::number(round);

  result.append(loc);

  return result;
}

//----------------------------------------------------------------------------

int Standings::determineBestPossibleRankForPlayerAfterRound(const PlayerPair& pp, int round) const
{
  // we can only determine the best possible final rank if we
  // are in the RANKING match system
  if (cat.getMatchSystem() != RANKING)
  {
    return -1;
  }

  // make sure the playerpair is from this category
  if (*(pp.getCategory(db)) != cat)
  {
    return -1;
  }

  // make sure the round is valid
  if (round < 1)
  {
    return -1;
  }

  // make sure the round has already been played
  CatRoundStatus crs = cat.getRoundStatus();
  if (crs.getFinishedRoundsCount() < round)
  {
    return -1;
  }

  // determine the last FINISHED match that this player has played.
  // Note: this must not be in this round because the player
  // could have had a bye
  unique_ptr<Match> lastMatch = nullptr;
  int _r = round;
  MatchMngr mm{db};
  while ((lastMatch == nullptr) && (_r > 0))
  {
    lastMatch = mm.getMatchForPlayerPairAndRound(pp, _r);
    if ((lastMatch != nullptr) && (lastMatch->getState() != STAT_MA_FINISHED))
    {
      lastMatch = nullptr;   // skip all matches that are not finished
    }
    --_r;
  }

  if (lastMatch == nullptr)
  {
    // the player had no match yet (or none finished), so the first place is still
    // possible
    return 1;
  }

  // okay, we've found a valid match that is in state FINISHED

  // check if the player already achieved a final rank with this match
  auto winner = lastMatch->getWinner();
  int winnerRank = lastMatch->getWinnerRank();
  int loserRank = lastMatch->getLoserRank();
  bool isWinner = ((*winner) == pp);
  if (isWinner && (winnerRank > 0))
  {
    return winnerRank;
  }
  if (!isWinner && (loserRank > 0))
  {
    return loserRank;
  }

  // no final rank yet. So we have to follow the match tree, assuming
  // that the player wins all subsequent matches


  // a little helper function to get the next match for a match winner
  int finalRound = crs.getTotalRoundsCount();
  auto getNextWinnerMatch = [this, &finalRound, &mm](const Match& ma) -> unique_ptr<Match> {
    if (ma.getWinnerRank() > 0)
    {
      return nullptr;  // no next match
    }

    // maybe we already HAVE winner. then we must search for real
    // pair IDs
    if (ma.getState() == STAT_MA_FINISHED)
    {
      auto w = ma.getWinner();
      assert(w != nullptr);
      int r = ma.getMatchGroup().getRound();

      ++r;
      unique_ptr<Match> nextMatch = nullptr;
      while ((nextMatch == nullptr) && (r <= finalRound))
      {
        nextMatch = mm.getMatchForPlayerPairAndRound(*w, r);
        ++r;
      }

      return nextMatch;   // it's either the match or nullptr to indicate an error
    }

    // the match is not yet finished, so we need to search for symbolic
    // player values. In this case, it is the positive ID of our match
    QString where = QString("%1 = %2 OR %3 = %4").arg(MA_PAIR1_SYMBOLIC_VAL).arg(ma.getId());
    where = where.arg(MA_PAIR2_SYMBOLIC_VAL).arg(ma.getId());

    TabRow winnerMatchRow = db->getTab(TAB_MATCH)->getSingleRowByWhereClause(where.toUtf8().constData());  // this query must always yield exactly one match

    return mm.getMatch(winnerMatchRow.getId());
  };

  //
  // back to the follow-the-chain-of-matches-algorithm
  //

  // since the match ma is already finished (see checks above) and there is no
  // final rank for the player (see check above) there must be a future match for this player
  // and the match must be identifiable by the pair ID, not by a symbolic name (the symbolic
  // name should be resolved by now).
  _r = round + 1;
  unique_ptr<Match> nextMatch = nullptr;
  while ((nextMatch == nullptr) && (_r <= finalRound))
  {
    nextMatch = mm.getMatchForPlayerPairAndRound(pp, _r);
    ++_r;
  }

  if (nextMatch == nullptr)
  {
    // this shouldn't happen. There should always be a next match.
    // I give up.
    return -1;
  }

  // no we follow the bracket tree, assuming that the player wins all
  // subsequent matches. Let's see where it takes us...
  Match oldNextMatch = *nextMatch;
  while (nextMatch != nullptr)
  {
    oldNextMatch = *nextMatch;
    nextMatch = getNextWinnerMatch(*nextMatch);
  }

  // now oldNextMatch holds the final match in the chain of matches
  winnerRank = oldNextMatch.getWinnerRank();

  // this is either the wanted result or -1 which can then as well
  // serve as an error indicator
  return winnerRank;
}

void Standings::printBestCaseList(upSimpleReport& rep) const
{
  if (cat.getMatchSystem() != RANKING)
  {
    return;
  }

  // we print the list for all participating player pairs, sorted
  // alphabetically
  PlayerPairList ppList = cat.getPlayerPairs();
  std::sort(ppList.begin(), ppList.end(), [](PlayerPair& pp1, PlayerPair& pp2)
  {
    return pp1.getDisplayName() < pp2.getDisplayName();
  });

  // prepare a table for the standings
  SimpleReportLib::TabSet ts;
  ts.addTab(120, SimpleReportLib::TAB_JUSTIFICATION::TAB_CENTER);  // the best case rank

  SimpleReportLib::TableWriter tw(ts);
  tw.setHeader(0, tr("Player"));
  tw.setHeader(1, tr("Best case final place"));

  for (PlayerPair pp : ppList)
  {
    QStringList rowContent;
    rowContent << pp.getDisplayName();
    int bestCaseRank = determineBestPossibleRankForPlayerAfterRound(pp, round);
    if (bestCaseRank > 0)
    {
      rowContent << QString::number(bestCaseRank);
    } else {
      rowContent << "??";   // shouldn't happen
    }
    tw.appendRow(rowContent);
  }

  tw.write(rep.get());
}

//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


}
