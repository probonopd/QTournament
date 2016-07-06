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

#ifndef MATCHTIMEPREDICTOR_H
#define MATCHTIMEPREDICTOR_H

#include <vector>

#include <QObject>

#include "DbTab.h"
#include "TournamentDB.h"
#include "Match.h"

using namespace std;
using namespace SqliteOverlay;

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
    MatchTimePredictor(TournamentDB* _db);

    // getters
    int getAverageMatchTime__secs();
    vector<MatchTimePrediction> getMatchTimePrediction();
    MatchTimePrediction getPredictionForMatch(const Match& ma, bool refreshCache = false);
    void updatePrediction();
    void resetPrediction();

  private:
    static constexpr int DEFAULT_MATCH_TIME__SECS = 25 * 60;  // 25 minutes
    static constexpr int GRACE_TIME_BETWEEN_MATCHES__SECS = 60;
    static constexpr int COURTS_IS_BUSY_AND_PREDICTION_WRONG__CORRECTION_OFFSET__SECS = 5 * 60;
    static constexpr int NUM_INITIALLY_ASSUMED_MATCHES = 5;

    TournamentDB* db;
    unsigned long totalMatchTime_secs;
    int nMatches;
    time_t lastMatchFinishTime;

    vector<MatchTimePrediction> lastPrediction;

    void updateAvgMatchTimeFromDatabase();
  };

}

#endif // MATCHTIMEPREDICTOR_H
