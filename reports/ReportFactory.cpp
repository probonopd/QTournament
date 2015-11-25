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
  constexpr char ReportFactory::REP__RESULTS_AND_NEXT_MATCHES[];
  constexpr char ReportFactory::REP__BRACKET[];
  constexpr char ReportFactory::REP__MATRIX_AND_STANDINGS[];

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
      result.append(genRepName(REP__RESULTS_AND_NEXT_MATCHES, cat, 0));   // list of initial matches
      for (int round=1; round <= crs.getFinishedRoundsCount(); ++round)
      {
        result.append(genRepName(REP__RESULTS, cat, round));
        result.append(genRepName(REP__RESULTS_AND_NEXT_MATCHES, cat, round));
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

      // generate matrix-and-result sheets for all round-robin and group matches
      MATCH_SYSTEM msys = cat.getMatchSystem();
      int numFinishedRounds = crs.getFinishedRoundsCount();
      if (msys == GROUPS_WITH_KO)
      {
        // initial matches
        result.append(genRepName(REP__MATRIX_AND_STANDINGS, cat, 0));

        // a matrix for each finished round of the round-robin phase
        KO_Config cfg = KO_Config(cat.getParameter_string(GROUP_CONFIG));
        int numGroupRounds = cfg.getNumRounds();
        for (int round = 1; ((round <= numGroupRounds) && (round <= numFinishedRounds)); ++round)
        {
          result.append(genRepName(REP__MATRIX_AND_STANDINGS, cat, round));
        }
      }
      if (msys == ROUND_ROBIN)
      {
        // initial matches
        result.append(genRepName(REP__MATRIX_AND_STANDINGS, cat, 0));

        // a matrix for each finished round
        for (int round = 1; round <= numFinishedRounds; ++round)
        {
          result.append(genRepName(REP__MATRIX_AND_STANDINGS, cat, round));
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

    // we brute-force check all categories for the availability
    // of tournament bracket visualization data
    DbTab tabVis = db->getTab(TAB_BRACKET_VIS);
    for (Category cat : cm->getAllCategories())
    {
      int catId = cat.getId();
      if (tabVis.getMatchCountForColumnValue(BV_CAT_REF, catId) > 0)
      {
        result.append(genRepName(REP__BRACKET, cat, 0));
      }
    }

    // we can always generate result sheets
    result.append(genRepName(REP__RESULTSHEETS, 1, 0));
    result.append(genRepName(REP__RESULTSHEETS, 4, 0));
    result.append(genRepName(REP__RESULTSHEETS, 8, 0));
    result.append(genRepName(REP__RESULTSHEETS, 12, 0));


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

    // results and next Matches
    if (pureRepName == REP__RESULTS_AND_NEXT_MATCHES)
    {
      int catId = intParam1;
      int round = intParam2;
      Category cat = Tournament::getCatMngr()->getCategoryById(catId);
      return upAbstractReport(new ResultsAndNextMatches(db, repName, cat, round));
    }

    // brackets
    if (pureRepName == REP__BRACKET)
    {
      int catId = intParam1;
      Category cat = Tournament::getCatMngr()->getCategoryById(catId);
      return upAbstractReport(new BracketSheet(db, repName, cat));
    }

    // matrix with standings
    if (pureRepName == REP__MATRIX_AND_STANDINGS)
    {
      int catId = intParam1;
      int round = intParam2;
      Category cat = Tournament::getCatMngr()->getCategoryById(catId);
      return upAbstractReport(new MartixAndStandings(db, repName, cat, round));
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

