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
#include <cmath>

#include <QDateTime>

#include "MatchTimePredictor.h"
#include "CourtMngr.h"
#include "MatchMngr.h"
#include "Match.h"
#include "TournamentDataDefs.h"
#include "CentralSignalEmitter.h"
#include "MatchMngr.h"
#include "CatMngr.h"

namespace QTournament {

  MatchTimePredictor::MatchTimePredictor(TournamentDB* _db)
    :db(_db), totalMatchTime_secs(0), nMatches(0), lastMatchFinishTime(0)
  {
    resetPrediction();
  }

  //----------------------------------------------------------------------------

  int MatchTimePredictor::getGlobalAverageMatchDuration__secs()
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

  int MatchTimePredictor::getAverageMatchDurationForCat__secs(const Category& cat)
  {
    int catId = cat.getId();
    int cnt;
    unsigned long catTime;
    tie(cnt, catTime) = catId2MatchTime[catId];
    if (cnt < NUM_INITIALLY_ASSUMED_MATCHES)
    {
      // blend with the global average if we don't have enough
      // data points in this cat
      int avg = getGlobalAverageMatchDuration__secs();
      catTime += (NUM_INITIALLY_ASSUMED_MATCHES - cnt) * avg;
      cnt = NUM_INITIALLY_ASSUMED_MATCHES;
    }

    return catTime / cnt;
  }

  //----------------------------------------------------------------------------

  vector<MatchTimePrediction> MatchTimePredictor::getMatchTimePrediction()
  {
    updatePrediction();

    return lastPrediction;
  }

  //----------------------------------------------------------------------------

  MatchTimePrediction MatchTimePredictor::getPredictionForMatch(const Match& ma, bool refreshCache)
  {
    if (refreshCache)
    {
      updatePrediction();
    }

    int maId = ma.getId();

    // find the value for the match in the prediction list
    auto it = find_if(lastPrediction.begin(), lastPrediction.end(),
                      [&maId](const MatchTimePrediction& mtp) { return (mtp.matchId == maId);});

    // return an "empty" match time prediction if we have no match
    if (it == lastPrediction.end())
    {
      MatchTimePrediction mtp;
      mtp.estCourtNum = -1;
      mtp.estFinishTime__UTC = 0;
      mtp.estStartTime__UTC = 0;
      mtp.matchId = maId;
      return mtp;
    }

    // in all other cases return the data set we've just found
    return *it;
  }

  //----------------------------------------------------------------------------

  void MatchTimePredictor::updateAvgMatchTimeFromDatabase()
  {
    // ensure that we have catId2MatchTime entry for each category
    CatMngr cm{db};
    for (const Category& c : cm.getAllCategories())
    {
      int catId = c.getId();
      auto it = catId2MatchTime.find(catId);
      if (it != catId2MatchTime.end()) continue;

      // insert an empty element
      tuple<int, unsigned long> empty = make_tuple(0, 0);
      catId2MatchTime[catId] = empty;
    }

    // find all matches that have been finished since the last update
    WhereClause wc;
    wc.addIntCol(MA_FINISH_TIME, ">", lastMatchFinishTime);
    wc.addIntCol(GENERIC_STATE_FIELD_NAME, static_cast<int>(STAT_MA_FINISHED));
    wc.setOrderColumn_Asc(MA_FINISH_TIME);

    DbTab* maTab = db->getTab(TAB_MATCH);
    auto it = maTab->getRowsByWhereClause(wc);
    MatchMngr mm{db};
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


      // update the accumulated match times
      int matchDuration_secs = finishTime - startTime;
      totalMatchTime_secs += matchDuration_secs;
      auto ma = mm.getMatch(row.getId());   // this is very expensive in terms of cycles and DB queries... especially since basically all information is already at hand
      int catId = ma->getCategory().getId();
      int cnt;
      unsigned long catTime;
      tie(cnt, catTime) = catId2MatchTime[catId];  // a key for this value MUST exist, see above
      ++cnt;
      catTime += matchDuration_secs;
      catId2MatchTime[catId] = make_tuple(cnt, catTime);

      lastMatchFinishTime = finishTime;  // we've ordered the results by finish time, see above
      ++nMatches;

      ++it;
    }
  }

  //----------------------------------------------------------------------------

  void MatchTimePredictor::updatePrediction()
  {
    // determine the available, not disabled courts
    CourtMngr cm{db};
    CourtList allCourts = cm.getAllCourts();
    allCourts.erase(remove_if(allCourts.begin(), allCourts.end(), [](Court& c){ return (c.getState() == STAT_CO_DISABLED);}),
        allCourts.end());

    // if we don't have any courts at all, we can't make any predictions
    if (allCourts.size() == 0)
    {
      lastPrediction.clear();
      CentralSignalEmitter::getInstance()->matchTimePredictionChanged(-1, 0);
      return;
    }

    // take all recently finished matches into account
    // for the average match time
    updateAvgMatchTimeFromDatabase();

    // set up a list of court numbers along with the
    // expected time when they'll be free again
    MatchMngr mm{db};
    time_t now = time(nullptr);
    deque<tuple<int, int>> courtFreeList;
    for (const Court& c : allCourts)
    {
      int coNum = c.getNumber();

      // default value for empty courts
      int finishTime = now - GRACE_TIME_BETWEEN_MATCHES__SECS;  // will be added again later

      upMatch ma = mm.getMatchForCourt(c);
      if (ma != nullptr)
      {
        QDateTime start = ma->getStartTime();
        int avgMatchTime = getAverageMatchDurationForCat__secs(*ma);
        if (!(start.isNull()))  // getStartTime returns NULL-time on error or if court is empty
        {
          finishTime = start.toTime_t() + avgMatchTime;

          // handle a special case here:
          //
          // if the court is in use and the avgMatchTime is
          // less than the actual running time of the match,
          // the predicted finishTime can be in the past!
          //
          // in this case we simply assume that the court
          // will be ready in five minutes because the match
          // must be close to its end
          if (finishTime < now)
          {
            finishTime = now + COURTS_IS_BUSY_AND_PREDICTION_WRONG__CORRECTION_OFFSET__SECS;
          }
        }
      }

      courtFreeList.push_back(make_tuple(coNum, finishTime));
    }

    // define a lambda for sorting courts according to their
    // availability
    auto courtSortFunc = [](const tuple<int, int>& c1, const tuple<int, int>& c2) {
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
    };

    // sort the list so that the earliest free court is first
    std::sort(courtFreeList.begin(), courtFreeList.end(), courtSortFunc);

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

    bool needsAnotherSorting = true;   // explanation at the end of the while() loop
    while (!(it.isEnd()))
    {
      TabRow matchRow = *it;
      auto ma = mm.getMatch(matchRow.getId());
      int avgMatchTime = getAverageMatchDurationForCat__secs(*ma);

      // get the earliest available court, which is always the first
      // court in the list
      int coNum;
      int coFree;
      tie(coNum, coFree) = courtFreeList.front();
      courtFreeList.pop_front();

      // calc start and finish time
      //
      // round start and finish time to full minutes
      // to achieve synchronized / harmonized UI updates
      time_t start = coFree + GRACE_TIME_BETWEEN_MATCHES__SECS;
      time_t finish = start + avgMatchTime;
      start = round(start / 60.0) * 60;
      finish = round(finish / 60.0) * 60;

      // prepare a new prediction element
      struct MatchTimePrediction mtp;
      mtp.matchId = matchRow.getId();
      mtp.estStartTime__UTC = start;
      mtp.estFinishTime__UTC = finish;
      mtp.estCourtNum = coNum;

      // store the element
      result.push_back(mtp);

      // virtually allocate the court for a length of avgMatchTime
      //
      // this means implicitly that this court is last to become
      // available again and so we push it at the of the court list
      courtFreeList.push_back(make_tuple(coNum, finish));

      // exception: if we have currently running matches that last already
      // significantly longer than avgMatchTime, the pushed_back court is NOT
      // correctly positioned at the end of the list. Example: run two matches
      // on court 1 while there's still the first match on court 2 ongoing. In
      // this case, court 1 needs to go BEFORE court 2 and court 2 remains at the
      // end of the list.
      //
      // to solve this, we simply sort the list after every match BUT to avoid
      // unnecessary sort/runtime/comparisons in every loop iteration, we stop
      // doing this once the added element is still at the end of the deque AFTER sorting.
      //
      // if this criterion is met, no match longer than avgMatchTime is in the deque
      // and we may safely assume that the element push to the end of the list will
      // always represent the court that will be available last.
      if (needsAnotherSorting)
      {
        std::sort(courtFreeList.begin(), courtFreeList.end(), courtSortFunc);

        // check the element at the end of the list
        int coNumOld = coNum;
        tie(coNum, coFree) = courtFreeList.back();

        // stop sorting if it is still the same that we pushed there
        // earlier. or positively phrased: sort as long as the last
        // element does not remain the last element after sorting
        needsAnotherSorting = (coNum != coNumOld);
      }

      // next match
      ++it;
    }

    // inform everyone about the latest statistics
    time_t endOfLastMatch = result.size() > 0 ? result.back().estFinishTime__UTC : 0;
    CentralSignalEmitter::getInstance()->matchTimePredictionChanged(getGlobalAverageMatchDuration__secs(), endOfLastMatch);

    // cache the result
    lastPrediction = result;
  }

  //----------------------------------------------------------------------------

  void MatchTimePredictor::resetPrediction()
  {
    totalMatchTime_secs = 0;
    nMatches = 0;
    lastMatchFinishTime = 0;
    lastPrediction.clear();
    catId2MatchTime.clear();

    updateAvgMatchTimeFromDatabase();
    updatePrediction();  // will emit signals to reset e.g., the progess bar in the scheduler.
  }

  //----------------------------------------------------------------------------


  //----------------------------------------------------------------------------


  //----------------------------------------------------------------------------


  //----------------------------------------------------------------------------


  //----------------------------------------------------------------------------


  //----------------------------------------------------------------------------


  //----------------------------------------------------------------------------


}
