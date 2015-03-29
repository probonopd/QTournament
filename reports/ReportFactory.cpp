#include "ReportFactory.h"

#include "ParticipantsList.h"
#include "MatchResultList.h"
#include "MatchResultList_byGroup.h"
#include "CatMngr.h"
#include "CatRoundStatus.h"

namespace QTournament
{
  constexpr char ReportFactory::REP__PARTLIST_BY_NAME[];
  constexpr char ReportFactory::REP__PARTLIST_BY_TEAM[];
  constexpr char ReportFactory::REP__RESULTS[];
  constexpr char ReportFactory::REP__RESULTS_BY_GROUP[];


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
    CatMngr* cm = Tournament::getCatMngr();

    // we can always generate a participants list
    result.append(REP__PARTLIST_BY_NAME);
    result.append(REP__PARTLIST_BY_TEAM);

    // we can print result lists for all finished or running rounds
    // in all categories
    for (Category cat : cm->getAllCategories())
    {
      OBJ_STATE catState = cat.getState();
      if ((catState == STAT_CAT_CONFIG) || (catState == STAT_CAT_FROZEN))
      {
        continue;  // match results for "unstarted" categories
      }

      int id = cat.getId();
      QString repName = REP__RESULTS;
      repName += "," + QString::number(id) + ",";

      CatRoundStatus crs = cat.getRoundStatus();
      for (int round=1; round <= crs.getFinishedRoundsCount(); ++round)
      {
        result.append(repName + QString::number(round));
      }
      for (int round : crs.getCurrentlyRunningRoundNumbers())
      {
        result.append(repName + QString::number(round));
      }

      // we can also print result lists for all categories with
      // round robin groups
      if (cat.getMatchSystem() == GROUPS_WITH_KO)
      {
        QString repName = REP__RESULTS_BY_GROUP;
        repName += "," + QString::number(id) + ",";
        for (MatchGroup mg : Tournament::getMatchMngr()->getMatchGroupsForCat(cat, 1))
        {
          int grpNum = mg.getGroupNumber();
          if (grpNum < 0) continue;
          result.append(repName + QString::number(grpNum));
        }
      }
    }


    return result;
  }

//----------------------------------------------------------------------------

  upAbstractReport ReportFactory::getReportByName(const QString& repName) const
  {
    QString pureRepName = repName.split(",")[0];

    if (pureRepName == REP__PARTLIST_BY_NAME)
    {
      return upAbstractReport(new ParticipantsList(db, REP__PARTLIST_BY_NAME, ParticipantsList::SORT_BY_NAME));
    }
    if (pureRepName == REP__PARTLIST_BY_TEAM)
    {
      return upAbstractReport(new ParticipantsList(db, REP__PARTLIST_BY_TEAM, ParticipantsList::SORT_BY_TEAM));
    }

    // result lists by round
    if (pureRepName == REP__RESULTS)
    {
      int catId = repName.split(",")[1].toInt();
      int round = repName.split(",")[2].toInt();
      Category cat = Tournament::getCatMngr()->getCategoryById(catId);
      return upAbstractReport(new MatchResultList(db, repName, cat, round));
    }

    // result lists by group
    if (pureRepName == REP__RESULTS_BY_GROUP)
    {
      int catId = repName.split(",")[1].toInt();
      int grpNum = repName.split(",")[2].toInt();
      Category cat = Tournament::getCatMngr()->getCategoryById(catId);
      return upAbstractReport(new MatchResultList_ByGroup(db, repName, cat, grpNum));
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

