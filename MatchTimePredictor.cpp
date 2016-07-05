/*
 *    This is QTournament, a badminton tournament management program.
 *    Copyright (C) 2014 - 2016  Volker Knollmann
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

#include <ctime>
#include <deque>

#include <QDateTime>

#include "MatchTimePredictor.h"
#include "CourtMngr.h"
#include "MatchMngr.h"
#include "Match.h"
#include "TournamentDataDefs.h"

namespace QTournament {

  MatchTimePredictor::MatchTimePredictor(TournamentDB* _db)
    :db(_db), totalMatchTime_secs(0), nMatches(0), lastMatchFinishTime(0)
  {
    updateAvgMatchTimeFromDatabase();
  }

  //----------------------------------------------------------------------------

  int MatchTimePredictor::getAverageMatchTime__secs()
  {
    // return pure database ("reality") values if we have a sufficiently
    // large number of real matches
    if (nMatches >= NUM_INITIALLY_ASSUMED_MATCHES)
    {
      return totalMatchTime_secs / ((long) nMatches);
    }

    // return the default duration if we have no matches at all
    if (nMatches < 1)
    {
      return DEFAULT_MATCH_TIME__SECS;
    }

    // blend default and real duration for the first few matches
    unsigned long matchTimeBlended = totalMatchTime_secs + \
                                     (NUM_INITIALLY_ASSUMED_MATCHES - nMatches) * DEFAULT_MATCH_TIME__SECS;

    return matchTimeBlended / ((long) NUM_INITIALLY_ASSUMED_MATCHES);
  }

  //----------------------------------------------------------------------------

  vector<MatchTimePrediction> MatchTimePredictor::getMatchTimePrediction()
  {
    // determine the available, not disabled courts
    CourtMngr cm{db};
    CourtList allCourts = cm.getAllCourts();
    remove_if(allCourts.begin(), allCourts.end(), [](Court& c){ return (c.getState() == STAT_CO_DISABLED);});

    // if we don't have any courts at all, we can't make any predictions
    if (allCourts.size() == 0)
    {
      return vector<MatchTimePrediction>();
    }

    // set up a list of court numbers along with the
    // expected time when they'll be free again
    MatchMngr mm{db};
    time_t now = time(nullptr);
    int avgMatchTime = getAverageMatchTime__secs();
    deque<tuple<int, int>> courtFreeList;
    for (const Court& c : allCourts)
    {
      int coNum = c.getNumber();
      int finishTime = now;

      upMatch ma = mm.getMatchForCourt(c);
      if (ma != nullptr)
      {
        QDateTime start = ma->getStartTime();
        if (!(start.isNull()))  // getStartTime returns NULL-time on error
        {
          finishTime = start.toTime_t() + avgMatchTime;
        }
      }

      courtFreeList.push_back(make_tuple(coNum, finishTime));
    }

    // sort the list so that the earliest free court is first
    std::sort(courtFreeList.begin(), courtFreeList.end(),
              [](const tuple<int, int>& c1, const tuple<int, int>& c2) {
      int c1Num;
      int c1Free;
      int c2Num;
      int c2Free;
      tie(c1Num, c1Free) = c1;
      tie(c2Num, c2Free) = c2;

      // if the courts have different times, sort by time
      if (c1Free != c2Free) return (c1Free < c2Free);

      // if they become available at the same time, sort
      // by court number
      return (c1Free < c2Free);
    });

    // prepare the result vector
    vector<MatchTimePrediction> result;

    // iterate over all queued, not running and not finished
    // matches and assign estimated start and end times
    WhereClause wc;
    wc.addIntCol(MA_NUM, ">", 0);   // the match needs to have a match number
    wc.addIntCol(GENERIC_STATE_FIELD_NAME, "!=", static_cast<int>(STAT_MA_FINISHED));  // the match is not finished
    wc.addIntCol(GENERIC_STATE_FIELD_NAME, "!=", static_cast<int>(STAT_MA_RUNNING));  // the match is not running
    wc.setOrderColumn_Asc(MA_NUM);
    DbTab* maTab = db->getTab(TAB_MATCH);
    auto it = maTab->getRowsByWhereClause(wc);

    while (!(it.isEnd()))
    {
      TabRow matchRow = *it;

      // get the earliest available court, which is always the first
      // court in the list
      int coNum;
      int coFree;
      tie(coNum, coFree) = courtFreeList.front();
      courtFreeList.pop_front();

      // prepare a new prediction element
      struct MatchTimePrediction mtp;
      mtp.matchId = matchRow.getId();
      mtp.estStartTime__UTC = coFree;
      mtp.estFinishTime__UTC = coFree + avgMatchTime;
      mtp.estCourtNum = coNum;

      // store the element
      result.push_back(mtp);

      // virtually allocate the court for a length of avgMatchTime
      //
      // this means implicitly that this court is last to become
      // available again and so we push it at the of the court list
      courtFreeList.push_back(make_tuple(coNum, coFree + avgMatchTime));

      // next match
      ++it;
    }

    return result;
  }

  //----------------------------------------------------------------------------

  void MatchTimePredictor::updateAvgMatchTimeFromDatabase()
  {
    // find all matches that have been finished since the last update
    WhereClause wc;
    wc.addIntCol(MA_FINISH_TIME, ">", lastMatchFinishTime);
    wc.addIntCol(GENERIC_STATE_FIELD_NAME, static_cast<int>(STAT_MA_FINISHED));
    wc.setOrderColumn_Asc(MA_FINISH_TIME);

    DbTab* maTab = db->getTab(TAB_MATCH);
    auto it = maTab->getRowsByWhereClause(wc);
    while (!(it.isEnd()))
    {
      TabRow row = *it;

      // treat all times as ints, that's easier

      // check for the existence of the timestamps, because
      // walkovers might not have one
      auto _startTime = row.getInt2(MA_START_TIME);
      if (_startTime->isNull())
      {
        ++it;
        continue;
      }
      int startTime = _startTime->get();

      auto _finishTime = row.getInt2(MA_FINISH_TIME);
      if (_finishTime->isNull())
      {
        ++it;
        continue;
      }
      int finishTime = _finishTime->get();


      // update the accumulated match time
      totalMatchTime_secs += (finishTime - startTime);
      lastMatchFinishTime = finishTime;  // we've ordered the results by finish time, see above

      ++it;
    }
  }

  //----------------------------------------------------------------------------


  //----------------------------------------------------------------------------


  //----------------------------------------------------------------------------


  //----------------------------------------------------------------------------


  //----------------------------------------------------------------------------


  //----------------------------------------------------------------------------


  //----------------------------------------------------------------------------


}
