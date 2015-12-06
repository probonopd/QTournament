/* 
 * File:   PlayerTableModel.h
 * Author: volker
 *
 * Created on March 17, 2014, 7:51 PM
 */

#ifndef MATCHGROUPTABLEMODEL_H
#define	MATCHGROUPTABLEMODEL_H

#include <QAbstractTableModel>

#include "DbTab.h"
#include "TournamentDB.h"
#include "MatchGroup.h"

namespace QTournament
{

  class Tournament;

  class MatchGroupTableModel : public QAbstractTableModel
  {
    Q_OBJECT

  public:
    static constexpr int STATE_COL_ID = 4;  // id of the column with the match group state
    static constexpr int STAGE_SEQ_COL_ID = 5;  // id of the column with the stage sequence number
    static constexpr int COLUMN_COUNT = 6;  // number of columns in the model

    MatchGroupTableModel (Tournament* tnmt);
    int rowCount(const QModelIndex & parent = QModelIndex()) const;
    int columnCount(const QModelIndex & parent = QModelIndex()) const;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

  private:
    TournamentDB* db;
    dbOverlay::DbTab mgTab;
    
  public slots:
    void onBeginCreateMatchGroup();
    void onEndCreateMatchGroup(int newMatchGroupSeqNum);
    void onMatchGroupStatusChanged(int matchGroupId, int matchGroupSeqNum);
    void onBeginResetModel();
    void onEndResetModel();

  signals:
    void triggerFilterUpdate();

  };

}
#endif	/* MATCHGROUPTABLEMODEL_H */

