#include "ReportFactory.h"

#include "ParticipantsList.h"

namespace QTournament
{
  const QString ReportFactory::REP__PARTLIST_BY_NAME = QStringLiteral("ParticipantsListByName");


  ReportFactory::ReportFactory(TournamentDB* _db)
    : db(_db)
  {

  }

//----------------------------------------------------------------------------

  ReportFactory::~ReportFactory()
  {

  }

//----------------------------------------------------------------------------

  QStringList ReportFactory::getReportCatalogue() const
  {
    QStringList result;

    // we can always generate a participants list
    result.append(REP__PARTLIST_BY_NAME);

    return result;
  }

//----------------------------------------------------------------------------

  upAbstractReport ReportFactory::getReportByName(const QString& repName) const
  {
    if (repName == REP__PARTLIST_BY_NAME)
    {
      return upAbstractReport(new ParticipantsList(db, REP__PARTLIST_BY_NAME, ParticipantsList::SORT_BY_NAME));
    }

    return nullptr;
  }

//----------------------------------------------------------------------------

  std::vector<upAbstractReport> ReportFactory::getMissingReports(const QStringList& existingReportNames) const
  {
    QStringList allReps = getReportCatalogue();
    std::vector<upAbstractReport> result;

    for (QString repName : allReps)
    {
      if (existingReportNames.contains(repName)) continue;
      upAbstractReport newRep = getReportByName(repName);
      if (newRep != nullptr) result.push_back(std::move(newRep));
    }

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


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------

}

