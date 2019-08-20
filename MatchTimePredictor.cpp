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

  MatchTimePredictor::MatchTimePredictor(const TournamentDB& _db)
    :db(_db), totalMatchTime_secs(0), nMatches(0), lastMatchFinishTime(0)
  {
    resetPrediction();
  }

  //----------------------------------------------------------------------------

  int MatchTimePredictor::getGlobalAverageMatchDuration__secs()
  {
    // return pure database ("reality") values if we have a sufficiently
    // large number of real matches
    if (nMatches >= NumInitiallyAssumedMatches)
    {
      return totalMatchTime_secs / nMatches;
    }

    // return the default duration if we have no matches at all
    if (nMatches < 1)
    {
      return DefaultMatchTime_secs;
    }

    // blend default and real duration for the first few matches
    long matchTimeBlended = totalMatchTime_secs + \
                                     (NumInitiallyAssumedMatches - nMatches) * DefaultMatchTime_secs;

    return matchTimeBlended / NumInitiallyAssumedMatches;
  }

  //----------------------------------------------------------------------------

  int MatchTimePredictor::getAverageMatchDurationForCat__secs(const Category& cat)
  {
    int catId = cat.getId();
    auto [cnt, catTime] = catId2MatchTime[catId];
    if (cnt < NumInitiallyAssumedMatches)
    {
      // blend with the global average if we don't have enough
      // data points in this cat
      int avg = getGlobalAverageMatchDuration__secs();
      catTime += (NumInitiallyAssumedMatches - cnt) * avg;
      cnt = NumInitiallyAssumedMatches;
    }

    return catTime / cnt;
  }

  //----------------------------------------------------------------------------

  std::vector<MatchTimePrediction> MatchTimePredictor::getMatchTimePrediction()
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
      std::tuple<int, long> empty = std::tuple{0, 0l};
      catId2MatchTime[catId] = empty;
    }

    // find all matches that have been finished since the last update
    SqliteOverlay::WhereClause wc;
    wc.addCol(MA_FinishTime, ">", lastMatchFinishTime);
    wc.addCol(GenericStateFieldName, static_cast<int>(ObjState::MA_Finished));
    wc.setOrderColumn_Asc(MA_FinishTime);

    MatchMngr mm{db};
    for (SqliteOverlay::TabRowIterator it{db, TabMatch, wc}; it.hasData(); ++it)
    {
      const SqliteOverlay::TabRow& row = *it;

      // treat all times as ints, that's easier

      // check for the existence of the timestamps, because
      // walkovers might not have one
      auto startTime = row.getInt2(MA_StartTime);
      if (!startTime) continue;

      auto finishTime = row.getInt2(MA_FinishTime);
      if (!finishTime) continue;

      // update the accumulated match times
      int matchDuration_secs = *finishTime - *startTime;
      totalMatchTime_secs += matchDuration_secs;
      auto ma = mm.getMatch(row.id());   // this is very expensive in terms of cycles and DB queries... especially since basically all information is already at hand
      int catId = ma->getCategory().getId();
      auto [cnt, catTime] = catId2MatchTime[catId];  // a key for this value MUST exist, see above
      ++cnt;
      catTime += matchDuration_secs;
      catId2MatchTime[catId] = std::tuple{cnt, catTime};

      lastMatchFinishTime = *finishTime;  // we've ordered the results by finish time, see above
      ++nMatches;
    }
  }

  //----------------------------------------------------------------------------

  void MatchTimePredictor::updatePrediction()
  {
    // determine the available, not disabled courts
    CourtMngr cm{db};
    CourtList allCourts = cm.getAllCourts();
    allCourts.erase(remove_if(allCourts.begin(), allCourts.end(), [](Court& c){ return (c.getState() == ObjState::CO_Disabled);}),
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
    std::vector<std::tuple<int, int>> courtFreeList;
    for (const Court& c : allCourts)
    {
      int coNum = c.getNumber();

      // default value for empty courts
      int finishTime = now - GraceTimeBetweenMatches_secs;  // will be added again later

      auto ma = mm.getMatchForCourt(c);
      if (ma)
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
            finishTime = now + CourtIsBusyAndPredictionWrong_CorrectionOffset_secs;
          }
        }
      }

      courtFreeList.push_back(std::tuple{coNum, finishTime});
    }

    // define a lambda for comparing two court entries wrt
    // the time when they become available
    auto courtCmpFunc_less = [](const std::tuple<int, int>& c1, const std::tuple<int, int>& c2) {
      auto [c1Num, c1Free] = c1;
      auto [c2Num, c2Free] = c2;

      // if the courts have different times, sort by time
      if (c1Free != c2Free) return (c1Free < c2Free);

      // if they become available at the same time, sort
      // by court number
      return (c1Free < c2Free);
    };

    // prepare the result vector
    std::vector<MatchTimePrediction> result;

    // iterate over all queued, not running and not finished
    // matches and assign estimated start and end times
    SqliteOverlay::WhereClause wc;
    wc.addCol(MA_Num, ">", 0);   // the match needs to have a match number
    wc.addCol(GenericStateFieldName, "!=", static_cast<int>(ObjState::MA_Finished));  // the match is not finished
    wc.addCol(GenericStateFieldName, "!=", static_cast<int>(ObjState::MA_Running));  // the match is not running
    wc.setOrderColumn_Asc(MA_Num);

    for (SqliteOverlay::TabRowIterator it{db, TabMatch, wc}; it.hasData(); ++it)
    {
      const SqliteOverlay::TabRow& matchRow = *it;

      auto ma = mm.getMatch(matchRow.id());
      int avgMatchTime = getAverageMatchDurationForCat__secs(*ma);

      // get the earliest available court
      auto itNextAvailCout = std::min_element(begin(courtFreeList), end(courtFreeList), courtCmpFunc_less);
      auto [coNum, coFree] = *itNextAvailCout;

      // calc start and finish time
      //
      // round start and finish time to full minutes
      // to achieve synchronized / harmonized UI updates
      time_t start = coFree + GraceTimeBetweenMatches_secs;
      time_t finish = start + avgMatchTime;
      start = round(start / 60.0) * 60;
      finish = round(finish / 60.0) * 60;

      // prepare a new prediction element
      struct MatchTimePrediction mtp;
      mtp.matchId = matchRow.id();
      mtp.estStartTime__UTC = start;
      mtp.estFinishTime__UTC = finish;
      mtp.estCourtNum = coNum;

      // store the element
      result.push_back(mtp);

      // virtually allocate the court until the predicted
      // match finish time
      *itNextAvailCout = std::tuple{coNum, finish};
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
