/* 
 * File:   PlayerTableModel.h
 * Author: volker
 *
 * Created on March 17, 2014, 7:51 PM
 */

#ifndef COURTTABLEMODEL_H
#define	COURTTABLEMODEL_H

#include <QAbstractTableModel>
#include <QTimer>

#include "DbTab.h"
#include "TournamentDB.h"
#include "Court.h"

namespace QTournament
{

  class Tournament;

  class CourtTableModel : public QAbstractTableModel
  {
    Q_OBJECT

  public:
    static constexpr int COURT_NUM_COL_ID = 0;  // id of the column with the court number
    static constexpr int DURATION_COL_ID = 2;  // id of the column with match duration
    static constexpr int COLUMN_COUNT = 3;  // number of columns in the model

    static constexpr int DURATION_UPDATE_PERIOD__MS = 10000;   // update every 10 seconds

    CourtTableModel (TournamentDB* _db);
    int rowCount(const QModelIndex & parent = QModelIndex()) const;
    int columnCount(const QModelIndex & parent = QModelIndex()) const;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

  private:
    TournamentDB* db;
    SqliteOverlay::DbTab* courtTab;
    unique_ptr<QTimer> durationUpdateTimer;

  public slots:
    void onBeginCreateCourt();
    void onEndCreateCourt(int newCourtSeqNum);
    void onCourtStatusChanged(int courtId, int courtSeqNum);
    void onDurationUpdateTimerElapsed();

  };

}
#endif	/* COURTTABLEMODEL_H */

