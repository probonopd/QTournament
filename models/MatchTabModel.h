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

#ifndef MATCHTABLEMODEL_H
#define	MATCHTABLEMODEL_H

#include <vector>
#include <QAbstractTableModel>

#include "DbTab.h"
#include "TournamentDB.h"
#include "Match.h"
#include "MatchTimePredictor.h"

namespace QTournament
{

  class Tournament;

  class MatchTableModel : public QAbstractTableModel
  {
    Q_OBJECT

  public:
    static constexpr int STATE_COL_ID = 5;  // id of the column with the match state
    static constexpr int MATCH_NUM_COL_ID = 0;  // id of the column with the match number
    static constexpr int REFEREE_MODE_COL_ID = 6;  // id of the column with the referee node
    static constexpr int EST_START_COL_ID = 7;  // id of the column with the estimated start time
    static constexpr int EST_END_COL_ID = 8;  // id of the column with the estimated finish time
    static constexpr int EST_COURT_COL_ID = 9;  // id of the column with the estimated court
    static constexpr int COLUMN_COUNT = 10;  // number of columns in the model

    MatchTableModel (TournamentDB* _db);
    int rowCount(const QModelIndex & parent = QModelIndex()) const;
    int columnCount(const QModelIndex & parent = QModelIndex()) const;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    QModelIndex getIndex(int row, int col);

  private:
    TournamentDB* db;
    SqliteOverlay::DbTab* matchTab;
    unique_ptr<MatchTimePredictor> matchTimePredictor;
    vector<MatchTimePrediction> predictedMatchTimes;
    MatchTimePrediction getMatchTimePredictionForMatch(const Match& ma) const;
    
  public slots:
    void onBeginCreateMatch();
    void onEndCreateMatch(int newMatchSeqNum);
    void onMatchStatusChanged(int matchId, int matchSeqNum, OBJ_STATE fromState, OBJ_STATE toState);
    void onBeginResetModel();
    void onEndResetModel();
    void recalcPrediction();

  };

}
#endif	/* MATCHGROUPTABLEMODEL_H */

