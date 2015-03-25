#include "ParticipantsList.h"
#include "SimpleReportGenerator.h"

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

  result->writeLine("Hello World", "H1");

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
