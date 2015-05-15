#include "ReportFactory.h"

#include "ParticipantsList.h"
#include "MatchResultList.h"
#include "MatchResultList_byGroup.h"
#include "CatMngr.h"
#include "CatRoundStatus.h"
#include "Standings.h"
#include "InOutList.h"
#include "ResultSheets.h"

namespace QTournament
{
  constexpr char ReportFactory::REP__PARTLIST_BY_NAME[];
  constexpr char ReportFactory::REP__PARTLIST_BY_TEAM[];
  constexpr char ReportFactory::REP__PARTLIST_BY_CATEGORY[];
  constexpr char ReportFactory::REP__RESULTS[];
  constexpr char ReportFactory::REP__RESULTS_BY_GROUP[];
  constexpr char ReportFactory::REP__STANDINGS_BY_CATEGORY[];
  constexpr char ReportFactory::REP__INOUTLIST_BY_CATEGORY[];
  constexpr char ReportFactory::REP__RESULTSHEETS[];

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
    result.append(REP__PARTLIST_BY_CATEGORY);

    // we can print result lists for all finished or running rounds
    // in all categories
    for (Category cat : cm->getAllCategories())
    {
      OBJ_STATE catState = cat.getState();
      if ((catState == STAT_CAT_CONFIG) || (catState == STAT_CAT_FROZEN))
      {
        continue;  // no reports for "unstarted" categories
      }

      CatRoundStatus crs = cat.getRoundStatus();
      for (int round=1; round <= crs.getFinishedRoundsCount(); ++round)
      {
        result.append(genRepName(REP__RESULTS, cat, round));
      }
      for (int round : crs.getCurrentlyRunningRoundNumbers())
      {
        result.append(genRepName(REP__RESULTS, cat, round));
      }

      // we can also print result lists for all categories with
      // round robin groups
      if (cat.getMatchSystem() == GROUPS_WITH_KO)
      {
        for (MatchGroup mg : Tournament::getMatchMngr()->getMatchGroupsForCat(cat, 1))
        {
          int grpNum = mg.getGroupNumber();
          if (grpNum < 0) continue;
          result.append(genRepName(REP__RESULTS_BY_GROUP, cat, grpNum));
        }
      }
    }


    // we can print ranking lists for all finished rounds
    // in all categories
    for (Category cat : cm->getAllCategories())
    {
      OBJ_STATE catState = cat.getState();
      if ((catState == STAT_CAT_CONFIG) || (catState == STAT_CAT_FROZEN))
      {
        continue;  // no rankings for "unstarted" categories
      }

      CatRoundStatus crs = cat.getRoundStatus();
      for (int round=1; round <= crs.getFinishedRoundsCount(); ++round)
      {
        result.append(genRepName(REP__STANDINGS_BY_CATEGORY, cat, round));
      }
    }

    // we brute-force check all categories and rounds for the availability
    // of in-out-lists
    for (Category cat : cm->getAllCategories())
    {
      CatRoundStatus crs = cat.getRoundStatus();
      for (int round=1; round <= crs.getFinishedRoundsCount(); ++round)
      {
        if (InOutList::isValidCatRoundCombination(cat, round))
        {
          result.append(genRepName(REP__INOUTLIST_BY_CATEGORY, cat, round));
        }
      }
    }

    // we can always generate result sheets
    result.append(genRepName(REP__RESULTSHEETS, 6, 0));
    result.append(genRepName(REP__RESULTSHEETS, 9, 0));


    return result;
  }

//----------------------------------------------------------------------------

  upAbstractReport ReportFactory::getReportByName(const QString& repName) const
  {
    QStringList repNameComponent = repName.split(",");
    QString pureRepName = repNameComponent[0];
    int intParam1 = 0;
    if (repNameComponent.size() > 1)
    {
      intParam1 = repNameComponent[1].toInt();
    }
    int intParam2 = 0;
    if (repNameComponent.size() > 2)
    {
      intParam2 = repNameComponent[2].toInt();
    }

    if (pureRepName == REP__PARTLIST_BY_NAME)
    {
      return upAbstractReport(new ParticipantsList(db, REP__PARTLIST_BY_NAME, ParticipantsList::SORT_BY_NAME));
    }
    if (pureRepName == REP__PARTLIST_BY_TEAM)
    {
      return upAbstractReport(new ParticipantsList(db, REP__PARTLIST_BY_TEAM, ParticipantsList::SORT_BY_TEAM));
    }
    if (pureRepName == REP__PARTLIST_BY_CATEGORY)
    {
      return upAbstractReport(new ParticipantsList(db, REP__PARTLIST_BY_CATEGORY, ParticipantsList::SORT_BY_CATEGORY));
    }

    // result lists by round
    if (pureRepName == REP__RESULTS)
    {
      int catId = intParam1;
      int round = intParam2;
      Category cat = Tournament::getCatMngr()->getCategoryById(catId);
      return upAbstractReport(new MatchResultList(db, repName, cat, round));
    }

    // result lists by group
    if (pureRepName == REP__RESULTS_BY_GROUP)
    {
      int catId = intParam1;
      int grpNum = intParam2;
      Category cat = Tournament::getCatMngr()->getCategoryById(catId);
      return upAbstractReport(new MatchResultList_ByGroup(db, repName, cat, grpNum));
    }

    // standings by category
    if (pureRepName == REP__STANDINGS_BY_CATEGORY)
    {
      int catId = intParam1;
      int round = intParam2;
      Category cat = Tournament::getCatMngr()->getCategoryById(catId);
      return upAbstractReport(new Standings(db, repName, cat, round));
    }

    // in-out-lists
    if (pureRepName == REP__INOUTLIST_BY_CATEGORY)
    {
      int catId = intParam1;
      int round = intParam2;
      Category cat = Tournament::getCatMngr()->getCategoryById(catId);
      return upAbstractReport(new InOutList(db, repName, cat, round));
    }

    // result sheets
    if (pureRepName == REP__RESULTSHEETS)
    {
      int numMatches = intParam1;
      return upAbstractReport(new ResultSheets(db, repName, numMatches));
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

  QString ReportFactory::genRepName(QString repBaseName, const Category& cat, int intParam) const
  {
    return genRepName(repBaseName, cat.getId(), intParam);
  }

//----------------------------------------------------------------------------

  QString ReportFactory::genRepName(QString repBaseName, int intParam1, int intParam2) const
  {
    QString repName = repBaseName;
    repName += "," + QString::number(intParam1);
    if (intParam2 > 0)
    {
     repName += "," + QString::number(intParam2);
    }

    return repName;
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

