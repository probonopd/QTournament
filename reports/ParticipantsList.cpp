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

#include <QList>

#include "ParticipantsList.h"
#include "SimpleReportGenerator.h"
#include "TableWriter.h"

#include "PlayerMngr.h"
#include "TeamMngr.h"
#include "CatMngr.h"
#include "Team.h"

namespace QTournament
{


ParticipantsList::ParticipantsList(TournamentDB* _db, const QString& _name, int _sortCriterion)
  :AbstractReport(_db, _name), sortCriterion(_sortCriterion)
{
  // apply a default criterion if no reasonable argument was provided
  if ((sortCriterion != SORT_BY_NAME) && (sortCriterion != SORT_BY_TEAM) && (sortCriterion != SORT_BY_CATEGORY))
  {
    sortCriterion = SORT_BY_NAME;
  }
}

//----------------------------------------------------------------------------

ParticipantsList::~ParticipantsList()
{

}

//----------------------------------------------------------------------------

upSimpleReport ParticipantsList::regenerateReport()
{
  PlayerMngr pm{db};
  PlayerList pl = pm.getAllPlayers();

  upSimpleReport result = createEmptyReport_Portrait();

  // do we have any participants at all?
  if (pl.size() == 0)
  {
    setHeaderAndHeadline(result.get(), tr("List of Participants"));
    result->writeLine(tr("No participants are registered yet."));
    return result;
  }

  // generate the actual report, depending on the sorting criterion
  if (sortCriterion == SORT_BY_NAME)
  {
    createNameSortedReport(result);
  }
  if (sortCriterion == SORT_BY_TEAM)
  {
    createTeamSortedReport(result);
  }
  if (sortCriterion == SORT_BY_CATEGORY)
  {
    createCategorySortedReport(result);
  }

  // set header and footer
  setHeaderAndFooter(result, tr("Participants List"));

  return result;
}

//----------------------------------------------------------------------------

QStringList ParticipantsList::getReportLocators() const
{
  QStringList result;

  QString loc = tr("Participants::");
  if (sortCriterion == SORT_BY_NAME)
  {
    loc += tr("by name");
  }
  if (sortCriterion == SORT_BY_TEAM)
  {
    loc += tr("by team");
  }
  if (sortCriterion == SORT_BY_CATEGORY)
  {
    loc += tr("by category");
  }

  result.append(loc);

  return result;
}

//----------------------------------------------------------------------------

void ParticipantsList::createNameSortedReport(upSimpleReport& rep) const
{
  PlayerMngr pm{db};
  PlayerList pl = pm.getAllPlayers();

  // do we have any participants at all?
  if (pl.size() == 0) return;

  upSimpleReport result = createEmptyReport_Portrait();
  setHeaderAndHeadline(rep.get(), tr("List of Participants"), tr("Sorted by name"));

  // sort the player list according by name
  std::sort(pl.begin(), pl.end(), PlayerMngr::getPlayerSortFunction_byName());

  // create a table of all participants
  QStringList header;
  header.append(tr("Name"));
  header.append(tr("Team"));
  header.append(tr("Categories"));
  SimpleReportLib::TabSet ts;
  ts.addTab(90, SimpleReportLib::TAB_JUSTIFICATION::TAB_LEFT);
  ts.addTab(145, SimpleReportLib::TAB_JUSTIFICATION::TAB_LEFT);
  SimpleReportLib::TableWriter tw(ts);
  tw.setHeader(header);
  for (Player p : pl)
  {
    QStringList rowContent;
    rowContent << p.getDisplayName();
    rowContent << p.getTeam().getName();
    rowContent << getCommaSepCatListForPlayer(p);
    tw.appendRow(rowContent);
  }
  tw.setNextPageContinuationCaption(tr("List of Participants (cont.)"));
  tw.write(rep.get());
}

//----------------------------------------------------------------------------

void ParticipantsList::createTeamSortedReport(upSimpleReport &rep) const
{
  TeamMngr tm{db};

  vector<Team> tl = tm.getAllTeams();

  // do we have any teams at all?
  if (tl.size() == 0) return;

  setHeaderAndHeadline(rep.get(), tr("List of Participants"), tr("Sorted by team"));
  rep->addTab(50.0, SimpleReportLib::TAB_LEFT);
  rep->addTab(110.0, SimpleReportLib::TAB_LEFT);
  rep->addTab(160.0, SimpleReportLib::TAB_LEFT);

  // sort the team list by name
  std::sort(tl.begin(), tl.end(), TeamMngr::getTeamSortFunction_byName());

  // create the actual report
  bool isFirstTeam = true;
  for (Team t : tl)
  {
    // start a new team section
    double skip = SKIP_BEFORE_INTERMEDIATE_HEADER__MM;
    if (isFirstTeam)
    {
      skip = 0.0;
      isFirstTeam = false;
    }
    if (!(rep->hasSpaceForAnotherLine(INTERMEDIATEHEADLINE_STYLE, skip)))
    {
      rep->startNextPage();
    }
    printIntermediateHeader(rep, t.getName(), skip);

    // get the players of the team and sort them by name
    PlayerList pl = tm.getPlayersForTeam(t);
    std::sort(pl.begin(), pl.end(), PlayerMngr::getPlayerSortFunction_byName());

    // write all player names for this team in two columns
    for (int i=0; i < pl.size(); i+=2)
    {
      // start a new page, if necessary
      if (!(rep->hasSpaceForAnotherLine(QString())))
      {
        rep->startNextPage();
        printIntermediateHeader(rep, t.getName() + tr(" (cont.)"));
      }

      QString txtLine;

      for (int col=0; col < 2; ++col)
      {
        if ((i + col) >= pl.size()) break;

        Player p = pl.at(i+col);

        txtLine += p.getDisplayName() + "\t";

        // collect the categories this player is registered for
        txtLine += getCommaSepCatListForPlayer(p);
        txtLine += "\t";
      }
      txtLine.chop(1);  // remove the last tab
      rep->writeLine(txtLine);
    }

  }
}

//----------------------------------------------------------------------------

void ParticipantsList::createCategorySortedReport(upSimpleReport& rep) const
{
  CatMngr cm{db};
  TeamMngr tm{db};

  vector<Category> allCats = cm.getAllCategories();

  if (rep == nullptr) rep = createEmptyReport_Portrait();

  // do we have any categories at all?
  if (allCats.empty())
  {
    setHeaderAndHeadline(rep.get(), tr("List of Participants"), tr("Sorted by category"));
    rep->writeLine(tr("No categories have been created yet."));
    return;
  }

  // prepare a two-column layout
  rep->addTab(95.0, SimpleReportLib::TAB_LEFT);

  // sort categories by name
  std::sort(allCats.begin(), allCats.end(), CatMngr::getCategorySortFunction_byName());

  // dump the participants of each category, sorted by team
  // and with each category starting on a new page
  bool isFirstPage = true;
  for (Category cat : allCats)
  {
    // append a new page for the next category, if necessary
    if (!isFirstPage)
    {
      rep->startNextPage();
    }
    isFirstPage = false;

    // set the page headline
    setHeaderAndHeadline(rep.get(), tr("Participants in category ") + cat.getName());

    // loop over the list of players in this category
    // and derive a list of all involved teams
    QStringList teamNameList;
    for (Player p : cat.getAllPlayersInCategory())
    {
      QString teamName = p.getTeam().getName();
      if (teamNameList.contains(teamName)) continue;
      teamNameList.append(teamName);
    }

    // skip this category if the player list is empty
    if (teamNameList.isEmpty())
    {
      rep->writeLine(tr("No players have been assigned to this category yet."));
      continue;
    }

    // sort the team names alphabetically
    teamNameList.sort();

    // list all players per team
    for (QString teamName : teamNameList)
    {
      printIntermediateHeader(rep, teamName);

      // create a list of all players in this team
      // and in this category
      Team team = tm.getTeam(teamName);
      PlayerList pl;
      for (Player p : tm.getPlayersForTeam(team))
      {
        if (cat.hasPlayer(p)) pl.push_back(p);
      }

      // sort the player list alphabetically
      std::sort(pl.begin(), pl.end(), PlayerMngr::getPlayerSortFunction_byName());

      // dump the player names in two columns
      QString txt;
      int colCount = 0;
      for (Player p : pl)
      {
        if (colCount > 0) txt += "\t";
        txt += p.getDisplayName();
        ++colCount;

        if (colCount == 2)
        {
          rep->writeLine(txt);
          colCount = 0;
          txt.clear();
        }
      }
      // dump the last line
      if (colCount > 0) rep->writeLine(txt);
    }
  }
}

//----------------------------------------------------------------------------

QString ParticipantsList::getCommaSepCatListForPlayer(const Player& p) const
{
  QString result;

  vector<Category> catList = p.getAssignedCategories();
  std::sort(catList.begin(), catList.end(), CatMngr::getCategorySortFunction_byName());
  for (Category c : catList)
  {
    result += c.getName() + ", ";
  }
  if (catList.size() > 0) result.chop(2);

  return result;
}

//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


}
