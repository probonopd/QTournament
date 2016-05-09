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

#include "ResultSheets.h"
#include "SimpleReportGenerator.h"
#include "TableWriter.h"

#include "ui/GuiHelpers.h"
#include "Match.h"
#include "MatchMngr.h"
#include "SignalRelay.h"

namespace QTournament
{


ResultSheets::ResultSheets(TournamentDB* _db, const QString& _name, int _numMatches)
  :AbstractReport(_db, _name), numMatches(_numMatches)
{
  firstMatchNum = -1;
  SignalRelay::getInstance()->registerReceiver(this);
}

//----------------------------------------------------------------------------

ResultSheets::ResultSheets(TournamentDB* _db, const Match& singleMatchForPrinting)
  :AbstractReport(_db, "Dummy"), numMatches(1), firstMatchNum(singleMatchForPrinting.getMatchNumber())
{
  // No connection to the SignalRelay here!
  // We are completely independent.
  // If this constructor is used, the object is only transient for immediate printing.
}

//----------------------------------------------------------------------------

upSimpleReport ResultSheets::regenerateReport()
{
  upSimpleReport result = createEmptyReport_Portrait();

  // return an error message if we have no valid match number range selected
  if ((firstMatchNum < 0) || (numMatches < 1))
  {
    setHeaderAndHeadline(result.get(), "ResultSheets");
    result->writeLine(tr("No match selected."));
    return result;
  }

  // collect the matches to be printed
  MatchMngr mm{db};
  QList<Match> matchList;
  int lastMatch = mm.getMaxMatchNum();
  int i = firstMatchNum;
  while (matchList.size() < numMatches)
  {
    auto ma = mm.getMatchByMatchNum(i);
    if (ma != nullptr)
    {
      // we can only print result sheet for unfinished
      // matches. For now, let's also acceppt FUZZY and POSTPONED matches...
      OBJ_STATE stat = ma->getState();
      if ((stat == STAT_MA_BUSY) || (stat == STAT_MA_FUZZY) || (stat == STAT_MA_READY) ||
          (stat == STAT_MA_WAITING) || (stat == STAT_MA_POSTPONED))
      {
        matchList.append(*ma);
      }
    }

    ++i;
    if (i > lastMatch) break;
  }

  // return an empty report if we have no matches
  if (matchList.isEmpty())
  {
    setHeaderAndHeadline(result.get(), "ResultSheets");
    result->writeLine(tr("There are no printable matches for the selected range."));
    return result;
  }

  // okay, we have at least one sheet to print
  // --> setup the tabs
  result->addTab(95.0,  SimpleReportLib::TAB_CENTER);  // colon
  result->addTab(92.0,  SimpleReportLib::TAB_RIGHT);  // player 1
  result->addTab(98.0,  SimpleReportLib::TAB_LEFT);  // player 2
  result->addTab(190.0,  SimpleReportLib::TAB_RIGHT);  // right-bound stuff

  // we have three result sheets per page. Calculate the number of pages
  int numPages = matchList.size() / SHEETS_PER_PAGE;   // this always rounds down ("truncates to zero")
  if ((matchList.size() % SHEETS_PER_PAGE) != 0)
  {
    ++numPages;   // round up. we'll have at least one page
  }

  // create the report page by page
  for (int p=0; p < numPages; ++p)
  {
    for (int n=0; n < SHEETS_PER_PAGE; ++n)
    {
      // top of page 1: game1; top of page 2: game2, ...
      // mid of page 1: game"numPages"; mid of page 2: game"numPages+1", ...
      int matchIndex = p + n * numPages;

      // we might have empty sheets due to rounding
      if (matchIndex >= matchList.size()) continue;

      result->warpTo(n * SHEET_HEIGHT__MM + SHEET_TOP_MARGIN__MM);
      printMatchData(result, matchList.at(matchIndex));

      if (n < (SHEETS_PER_PAGE - 1))
      {
        result->addHorLine_absPos((n+1) * SHEET_HEIGHT__MM, SimpleReportLib::THIN);
      }
    }

    if (p < (numPages - 1))
    {
      result->startNextPage();
    }
  }


  return result;
}

//----------------------------------------------------------------------------

QStringList ResultSheets::getReportLocators() const
{
  QStringList result;

  QString loc = tr("Result Sheets::");
  loc += tr("selected match");
  if (numMatches > 1)
  {
    loc += " " + tr("plus") + " " + QString::number(numMatches-1);
  }

  result.append(loc);

  return result;
}

//----------------------------------------------------------------------------

void ResultSheets::onMatchSelectionChanged(int newlySelectedMatchId)
{
  if (newlySelectedMatchId <= 0)
  {
    firstMatchNum = -1;
  } else {
    MatchMngr mm{db};
    auto ma = mm.getMatch(newlySelectedMatchId);
    assert(ma != nullptr);
    firstMatchNum = ma->getMatchNumber();
  }
}

//----------------------------------------------------------------------------

void ResultSheets::printMatchData(upSimpleReport& rep, const Match& ma) const
{
  QString header = tr("Match Number: ") + QString::number(ma.getMatchNumber());
  header += "\t\t\t\t" + tr("Court: ____________");
  rep->writeLine(header);

  header = tr("%1, Round %2").arg(ma.getCategory().getName()).arg(ma.getMatchGroup().getRound());
  rep->writeLine(header);

  // write player name(s)
  QString name1_Line1 = "??";
  QString name2_Line1 = "??";
  QString name1_Line2;
  QString name2_Line2;
  QString team1;
  QString team2;
  if (ma.hasPlayerPair1())
  {
    PlayerPair pp1 = ma.getPlayerPair1();
    name1_Line1 = pp1.getPlayer1().getDisplayName_FirstNameFirst();
    team1 = pp1.getDisplayName_Team();
    if (pp1.hasPlayer2())
    {
      name1_Line1 += " /";
      name1_Line2 = pp1.getPlayer2().getDisplayName_FirstNameFirst();
    }
  } else {
    int symbolicName = ma.getSymbolicPlayerPair1Name();
    if (symbolicName != 0)
    {
      name1_Line1 = (symbolicName > 0) ? tr("Winner of Match #") : tr("Loser of Match #");
      name1_Line1 += QString::number(abs(symbolicName));
    }
  }
  if (ma.hasPlayerPair2())
  {
    PlayerPair pp2 = ma.getPlayerPair2();
    name2_Line1 = pp2.getPlayer1().getDisplayName_FirstNameFirst();
    team2 = pp2.getDisplayName_Team();
    if (pp2.hasPlayer2())
    {
      name2_Line1 += " /";
      name2_Line2 = pp2.getPlayer2().getDisplayName_FirstNameFirst();
    }
  } else {
    int symbolicName = ma.getSymbolicPlayerPair2Name();
    if (symbolicName != 0)
    {
      name2_Line1 = (symbolicName > 0) ? tr("Winner of Match #") : tr("Loser of Match #");
      name2_Line1 += QString::number(abs(symbolicName));
    }
  }
  rep->writeLine("\t" + name1_Line1 + "\t:\t" + name2_Line1, RESULTSHEET_NAME_STYLE);
  if (!(name1_Line2.isEmpty()) || !(name2_Line2.isEmpty()))
  {
    rep->writeLine("\t" + name1_Line2 + "\t\t" + name2_Line2, RESULTSHEET_NAME_STYLE);
  }

  // write the team names
  if (!(team1.isEmpty()) || !(team2.isEmpty()))
  {
    rep->writeLine("\t" + team1 + "\t\t" + team2, RESULTSHEET_TEAM_STYLE);
  }

  rep->skip(8.0);

  // print the score area
  for (int n=0; n < GAMES_PER_SHEET; ++n)
  {
    QString txt = QString::number(n+1) + ". ";
    txt += tr("Game:");
    txt += "\t__________     \t:\t     __________";
    rep->writeLine(txt, RESULTSHEET_GAMELABEL_STYLE);
    rep->skip(4.5);
  }

  // print umpire information
  REFEREE_MODE refMode = ma.getRefereeMode();
  if (refMode != REFEREE_MODE::NONE)
  {
    QString txt = tr("Umpire: ");
    upPlayer referee = ma.getAssignedReferee();

    if ((refMode != REFEREE_MODE::HANDWRITTEN) && (referee != nullptr))
    {
      txt += referee->getDisplayName_FirstNameFirst();
    }
    rep->writeLine(txt, RESULTSHEET_GAMELABEL_STYLE);
  }
}

//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


}
