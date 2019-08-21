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

#include "ReportFactory.h"

#include "ParticipantsList.h"
#include "MatchResultList.h"
#include "ResultsAndNextMatches.h"
#include "MatchResultList_byGroup.h"
#include "CatMngr.h"
#include "CatRoundStatus.h"
#include "Standings.h"
#include "InOutList.h"
#include "ResultSheets.h"
#include "BracketSheet.h"
#include "MatrixAndStandings.h"
#include "PureRoundRobinCategory.h"

namespace QTournament
{
  constexpr char ReportFactory::REP_PartListByName[];
  constexpr char ReportFactory::REP_PartListByTeam[];
  constexpr char ReportFactory::REP_PartListByCat[];
  constexpr char ReportFactory::REP_Results[];
  constexpr char ReportFactory::REP_ResultsByGroup[];
  constexpr char ReportFactory::REP_StandingsByCat[];
  constexpr char ReportFactory::REP_InOutByCat[];
  constexpr char ReportFactory::REP_ResultSheets[];
  constexpr char ReportFactory::REP_ResultsAndNextMatches[];
  constexpr char ReportFactory::REP_Bracket[];
  constexpr char ReportFactory::REP_MatrixAndStandings[];

  ReportFactory::ReportFactory(const TournamentDB& _db)
    : db(_db)
  {

  }

//----------------------------------------------------------------------------

  QStringList ReportFactory::getReportCatalogue() const
  {
    QStringList result;
    CatMngr cm{db};

    // we can always generate a participants list
    result.append(REP_PartListByName);
    result.append(REP_PartListByTeam);
    result.append(REP_PartListByCat);

    // we can print result lists for all finished or running rounds
    // in all categories
    for (Category cat : cm.getAllCategories())
    {
      ObjState catState = cat.getState();
      if ((catState == ObjState::CAT_Config) || (catState == ObjState::CAT_Frozen))
      {
        continue;  // no reports for "unstarted" categories
      }

      CatRoundStatus crs = cat.getRoundStatus();
      result.append(genRepName(REP_ResultsAndNextMatches, cat, 0));   // list of initial matches
      for (int round=1; round <= crs.getFinishedRoundsCount(); ++round)
      {
        result.append(genRepName(REP_Results, cat, round));
        result.append(genRepName(REP_ResultsAndNextMatches, cat, round));
      }
      for (int round : crs.getCurrentlyRunningRoundNumbers())
      {
        result.append(genRepName(REP_Results, cat, round));
      }

      // we can also print result lists for all categories with
      // round robin groups
      MatchMngr mm{db};
      if (cat.getMatchSystem() == MatchSystem::GroupsWithKO)
      {
        for (MatchGroup mg : mm.getMatchGroupsForCat(cat, 1))
        {
          int grpNum = mg.getGroupNumber();
          if (grpNum < 0) continue;
          result.append(genRepName(REP_ResultsByGroup, cat, grpNum));
        }
      }

      // generate matrix-and-result sheets for all round-robin and group matches
      MatchSystem msys = cat.getMatchSystem();
      int numFinishedRounds = crs.getFinishedRoundsCount();
      if (msys == MatchSystem::GroupsWithKO)
      {
        // initial matches
        result.append(genRepName(REP_MatrixAndStandings, cat, 0));

        // a matrix for each finished round of the round-robin phase
        KO_Config cfg = KO_Config(cat.getParameter_string(CatParameter::GroupConfig));
        int numGroupRounds = cfg.getNumRounds();
        for (int round = 1; ((round <= numGroupRounds) && (round <= numFinishedRounds)); ++round)
        {
          result.append(genRepName(REP_MatrixAndStandings, cat, round));
        }
      }
      if (msys == MatchSystem::RoundRobin)
      {
        // initial matches
        result.append(genRepName(REP_MatrixAndStandings, cat, 0));
        PureRoundRobinCategory rrCat{db, cat.rowRef()};
        int rpi = rrCat.getRoundCountPerIteration();
        int itCnt = rrCat.getIterationCount();
        for (int i=1; i < itCnt; ++i)
        {
          int firstRoundInIteration = i * rpi + 1;

          // note: negative round numbers denote: plot initial matches for the iteration
          // that starts with abs(firstRoundInIteration)
          result.append(genRepName(REP_MatrixAndStandings, cat, -firstRoundInIteration));
        }

        // a matrix for each finished round
        for (int round = 1; round <= numFinishedRounds; ++round)
        {
          result.append(genRepName(REP_MatrixAndStandings, cat, round));
        }
      }
    }


    // we can print ranking lists for all finished rounds
    // in all categories
    for (Category cat : cm.getAllCategories())
    {
      ObjState catState = cat.getState();
      if ((catState == ObjState::CAT_Config) || (catState == ObjState::CAT_Frozen))
      {
        continue;  // no rankings for "unstarted" categories
      }

      CatRoundStatus crs = cat.getRoundStatus();
      for (int round=1; round <= crs.getFinishedRoundsCount(); ++round)
      {
        result.append(genRepName(REP_StandingsByCat, cat, round));
      }
    }

    // we brute-force check all categories and rounds for the availability
    // of in-out-lists
    for (Category cat : cm.getAllCategories())
    {
      CatRoundStatus crs = cat.getRoundStatus();
      for (int round=1; round <= crs.getFinishedRoundsCount(); ++round)
      {
        if (InOutList::isValidCatRoundCombination(cat, round))
        {
          result.append(genRepName(REP_InOutByCat, cat, round));
        }
      }
    }

    // we brute-force check all categories for the availability
    // of tournament bracket visualization data
    SqliteOverlay::DbTab tabVis{db, TabBracketVis, false};
    for (Category cat : cm.getAllCategories())
    {
      int catId = cat.getId();
      if (tabVis.getMatchCountForColumnValue(BV_CatRef, catId) > 0)
      {
        result.append(genRepName(REP_Bracket, cat, 0));
      }
    }

    // we can always generate result sheets
    result.append(genRepName(REP_ResultSheets, 1, 0));
    result.append(genRepName(REP_ResultSheets, 4, 0));
    result.append(genRepName(REP_ResultSheets, 8, 0));
    result.append(genRepName(REP_ResultSheets, 12, 0));


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

    if (pureRepName == REP_PartListByName)
    {
      return upAbstractReport(new ParticipantsList(db, REP_PartListByName, ParticipantsList::SortByName));
    }
    if (pureRepName == REP_PartListByTeam)
    {
      return upAbstractReport(new ParticipantsList(db, REP_PartListByTeam, ParticipantsList::SortByTeam));
    }
    if (pureRepName == REP_PartListByCat)
    {
      return upAbstractReport(new ParticipantsList(db, REP_PartListByCat, ParticipantsList::SortByCategory));
    }

    CatMngr cm{db};

    // result lists by round
    if (pureRepName == REP_Results)
    {
      int catId = intParam1;
      int round = intParam2;
      Category cat = cm.getCategoryById(catId);
      return upAbstractReport(new MatchResultList(db, repName, cat, round));
    }

    // result lists by group
    if (pureRepName == REP_ResultsByGroup)
    {
      int catId = intParam1;
      int grpNum = intParam2;
      Category cat = cm.getCategoryById(catId);
      return upAbstractReport(new MatchResultList_ByGroup(db, repName, cat, grpNum));
    }

    // standings by category
    if (pureRepName == REP_StandingsByCat)
    {
      int catId = intParam1;
      int round = intParam2;
      Category cat = cm.getCategoryById(catId);
      return upAbstractReport(new Standings(db, repName, cat, round));
    }

    // in-out-lists
    if (pureRepName == REP_InOutByCat)
    {
      int catId = intParam1;
      int round = intParam2;
      Category cat = cm.getCategoryById(catId);
      return upAbstractReport(new InOutList(db, repName, cat, round));
    }

    // result sheets
    if (pureRepName == REP_ResultSheets)
    {
      int numMatches = intParam1;
      return upAbstractReport(new ResultSheets(db, repName, numMatches));
    }

    // results and next Matches
    if (pureRepName == REP_ResultsAndNextMatches)
    {
      int catId = intParam1;
      int round = intParam2;
      Category cat = cm.getCategoryById(catId);
      return upAbstractReport(new ResultsAndNextMatches(db, repName, cat, round));
    }

    // brackets
    if (pureRepName == REP_Bracket)
    {
      int catId = intParam1;
      Category cat = cm.getCategoryById(catId);
      return upAbstractReport(new BracketSheet(db, repName, cat));
    }

    // matrix with standings
    if (pureRepName == REP_MatrixAndStandings)
    {
      int catId = intParam1;
      int round = intParam2;
      Category cat = cm.getCategoryById(catId);
      return upAbstractReport(new MatrixAndStandings(db, repName, cat, round));
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
    repName += "," + QString::number(intParam2);

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

