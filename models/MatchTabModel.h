/* 
 * File:   PlayerTableModel.h
 * Author: volker
 *
 * Created on March 17, 2014, 7:51 PM
 */

#ifndef MATCHTABLEMODEL_H
#define	MATCHTABLEMODEL_H

#include <QAbstractTableModel>

#include "DbTab.h"
#include "TournamentDB.h"
#include "Match.h"

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
    static constexpr int COLUMN_COUNT = 7;  // number of columns in the model

    MatchTableModel (TournamentDB* _db);
    int rowCount(const QModelIndex & parent = QModelIndex()) const;
    int columnCount(const QModelIndex & parent = QModelIndex()) const;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    QModelIndex getIndex(int row, int col);

  private:
    TournamentDB* db;
    SqliteOverlay::DbTab* matchTab;
    
  public slots:
    void onBeginCreateMatch();
    void onEndCreateMatch(int newMatchSeqNum);
    void onMatchStatusChanged(int matchId, int matchSeqNum);
    void onBeginResetModel();
    void onEndResetModel();

  };

}
#endif	/* MATCHGROUPTABLEMODEL_H */

