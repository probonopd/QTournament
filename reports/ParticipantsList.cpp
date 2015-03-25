#include "ParticipantsList.h"
#include "SimpleReportGenerator.h"
#include "TableWriter.h"

#include "PlayerMngr.h"

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

upSimpleReport ParticipantsList::regenerateReport()
{
  upSimpleReport result = createEmptyReport_Portrait();

  setHeaderAndHeadline(result.get(), tr("List of Participants"));

  // do we have any participants at all?
  PlayerMngr* pm = Tournament::getPlayerMngr();
  PlayerList pl = pm->getAllPlayers();
  if (pl.size() == 0)
  {
    result->writeLine(tr("No participants are registered yet."));
    return result;
  }

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
    tw.appendRow(rowContent);
  }
  tw.setNextPageContinuationCaption(tr("List of Participants (cont.)"));
  tw.write(result.get());

  return result;
}

//----------------------------------------------------------------------------

QStringList ParticipantsList::getReportLocators()
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


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


}
