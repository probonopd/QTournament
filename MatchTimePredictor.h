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

#ifndef MATCHTIMEPREDICTOR_H
#define MATCHTIMEPREDICTOR_H

#include <vector>
#include <unordered_map>
#include <tuple>

#include <QObject>

#include <SqliteOverlay/DbTab.h>
#include "TournamentDB.h"
#include "Match.h"

namespace QTournament
{
  struct MatchTimePrediction
  {
    int matchId;
    time_t estStartTime__UTC;
    time_t estFinishTime__UTC;
    int estCourtNum;
  };

  //----------------------------------------------------------------------------

  class MatchTimePredictor : public QObject
  {
    Q_OBJECT

  public:
    // ctor
    MatchTimePredictor(const TournamentDB& _db);

    // getters
    int getGlobalAverageMatchDuration__secs();
    inline int getAverageMatchDurationForCat__secs(const Match& matchInCat) { return getAverageMatchDurationForCat__secs(matchInCat.getCategory()); }
    int getAverageMatchDurationForCat__secs(const Category& cat);
    std::vector<MatchTimePrediction> getMatchTimePrediction();
    MatchTimePrediction getPredictionForMatch(const Match& ma, bool refreshCache = false);
    void updatePrediction();
    void resetPrediction();

  private:
    static constexpr int DEFAULT_MATCH_TIME__SECS = 25 * 60;  // 25 minutes
    static constexpr int GRACE_TIME_BETWEEN_MATCHES__SECS = 60;
    static constexpr int COURTS_IS_BUSY_AND_PREDICTION_WRONG__CORRECTION_OFFSET__SECS = 5 * 60;
    static constexpr int NUM_INITIALLY_ASSUMED_MATCHES = 5;

    std::reference_wrapper<const QTournament::TournamentDB> db;
    unsigned long totalMatchTime_secs;
    int nMatches;
    time_t lastMatchFinishTime;

    std::unordered_map<int, std::tuple<int, unsigned long>> catId2MatchTime;

    std::vector<MatchTimePrediction> lastPrediction;

    void updateAvgMatchTimeFromDatabase();
  };

}

#endif // MATCHTIMEPREDICTOR_H
