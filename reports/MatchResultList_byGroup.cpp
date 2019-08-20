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

#include <QList>

#include "MatchResultList_byGroup.h"
#include <SimpleReportGeneratorLib/SimpleReportGenerator.h>
#include <SimpleReportGeneratorLib/TableWriter.h>

#include "PlayerMngr.h"
#include "TeamMngr.h"
#include "Team.h"
#include "CatRoundStatus.h"
#include "ui/GuiHelpers.h"
#include "MatchGroup.h"
#include "MatchMngr.h"

namespace QTournament
{


MatchResultList_ByGroup::MatchResultList_ByGroup(TournamentDB* _db, const QString& _name, const Category& _cat, int _grpNum)
  :AbstractReport(_db, _name), cat(_cat), grpNum(_grpNum)
{
  // make sure that the requested group is a round-robin group
  // and the group is actually existing
  if (grpNum < 1)
  {
    throw std::runtime_error("Requested match results report for invalid group.");
  }
  MatchMngr mm{db};
  for (MatchGroup mg : mm.getMatchGroupsForCat(cat, 1))
  {
    if (mg.getGroupNumber() == grpNum) return;  // okay, the round-robin group number exists at least in the first round
  }

  throw std::runtime_error("Requested match results report for invalid group.");
}

//----------------------------------------------------------------------------

upSimpleReport MatchResultList_ByGroup::regenerateReport()
{
  // collect the match groups with the requested match group number and
  // search in all rounds
  MatchMngr mm{db};
  MatchGroupList mgl = mm.getMatchGroupsForCat(cat);
  MatchGroupList filteredList;
  for (MatchGroup mg: mgl)
  {
    if (mg.getGroupNumber() == grpNum) filteredList.push_back(mg);
  }

  // sort match groups by round number
  if (filteredList.size() > 1)
  {
    std::sort(filteredList.begin(), filteredList.end(), [](MatchGroup& mg1, MatchGroup& mg2){
      if (mg1.getRound() < mg2.getRound()) return true;
      return false;
    });
  }

  upSimpleReport result = createEmptyReport_Portrait();
  QString repName = cat.getName() + tr(" -- Results of Group ") + QString::number(grpNum);
  setHeaderAndHeadline(result.get(), repName);

  for (MatchGroup mg : filteredList)
  {
    int round = mg.getRound();
    printIntermediateHeader(result, tr("Round ") + QString::number(round));

    MatchList maList = mg.getMatches();
    std::sort(maList.begin(), maList.end(), [](Match& ma1, Match& ma2)
    {
      return ma1.getMatchNumber() < ma2.getMatchNumber();
    });

    printMatchList(result, maList, PlayerPairList(), tr("Results of round ") + QString::number(round) + tr(" (cont.)"), true, false);

    if (mg.getState() != ObjState::MG_FINISHED)
    {
      result->skip(1.0);
      result->writeLine(tr("Note: this round is not finished yet; results for this group can be incomplete."));
    }

    result->skip(3.0);
  }

  // set header and footer
  setHeaderAndFooter(result, repName);

  return result;
}

//----------------------------------------------------------------------------

QStringList MatchResultList_ByGroup::getReportLocators() const
{
  QStringList result;

  QString loc = tr("Results::");
  loc += cat.getName() + tr("::by group::");
  loc += tr("Group ") + QString::number(grpNum);

  result.append(loc);

  return result;
}

//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


}
