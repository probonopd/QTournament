#include <QList>

#include "ParticipantsList.h"
#include "SimpleReportGenerator.h"
#include "TableWriter.h"

#include "PlayerMngr.h"
#include "TeamMngr.h"
#include "Team.h"

namespace QTournament
{


ParticipantsList::ParticipantsList(TournamentDB* _db, const QString& _name, int _sortCriterion)
  :AbstractReport(_db, _name), sortCriterion(_sortCriterion)
{
  // apply a default criterion if no reasonable argument was provided
  if ((sortCriterion != SORT_BY_NAME) && (sortCriterion != SORT_BY_TEAM))
  {
    sortCriterion = SORT_BY_NAME;
  }
}

//----------------------------------------------------------------------------

ParticipantsList::~ParticipantsList()
{

}

//----------------------------------------------------------------------------

upSimpleReport ParticipantsList::regenerateReport() const
{
  PlayerMngr* pm = Tournament::getPlayerMngr();
  PlayerList pl = pm->getAllPlayers();

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

  result.append(loc);

  return result;
}

//----------------------------------------------------------------------------

void ParticipantsList::createNameSortedReport(upSimpleReport& rep) const
{
  PlayerMngr* pm = Tournament::getPlayerMngr();
  PlayerList pl = pm->getAllPlayers();

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
  PlayerMngr* pm = Tournament::getPlayerMngr();
  TeamMngr* tm = Tournament::getTeamMngr();

  QList<Team> tl = tm->getAllTeams();

  // do we have any teams at all?
  if (tl.size() == 0) return;

  upSimpleReport result = createEmptyReport_Portrait();
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
    double skip = SKIP_BEFORE_NEW_TEAM__MM;
    if (isFirstTeam)
    {
      skip = 0.0;
      isFirstTeam = false;
    }
    if (!(rep->hasSpaceForAnotherLine(INTERMEDIATEHEADLINE_STYLE, skip)))
    {
      rep->startNextPage();
    }
    rep->skip(skip);
    rep->writeLine(t.getName(), INTERMEDIATEHEADLINE_STYLE);
    rep->addHorLine();

    // get the players of the team and sort them by name
    PlayerList pl = tm->getPlayersForTeam(t);
    std::sort(pl.begin(), pl.end(), PlayerMngr::getPlayerSortFunction_byName());

    // write all player names for this team in two columns
    for (int i=0; i < pl.size(); i+=2)
    {
      // start a new page, if necessary
      if (!(rep->hasSpaceForAnotherLine(QString())))
      {
        rep->startNextPage();
        rep->writeLine(t.getName() + tr(" (cont.)"), INTERMEDIATEHEADLINE_STYLE);
        rep->addHorLine();
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

QString ParticipantsList::getCommaSepCatListForPlayer(const Player& p) const
{
  QString result;

  QList<Category> catList = p.getAssignedCategories();
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
