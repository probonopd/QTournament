/* 
 * File:   PlayerTableModel.h
 * Author: volker
 *
 * Created on March 17, 2014, 7:51 PM
 */

#ifndef COURTTABLEMODEL_H
#define	COURTTABLEMODEL_H

#include <QAbstractTableModel>

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
    static constexpr int COLUMN_COUNT = 2;  // number of columns in the model

    CourtTableModel (TournamentDB* _db);
    int rowCount(const QModelIndex & parent = QModelIndex()) const;
    int columnCount(const QModelIndex & parent = QModelIndex()) const;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

  private:
    TournamentDB* db;
    SqliteOverlay::DbTab* courtTab;
    
  public slots:
    void onBeginCreateCourt();
    void onEndCreateCourt(int newCourtSeqNum);
    void onCourtStatusChanged(int courtId, int courtSeqNum);

  };

}
#endif	/* COURTTABLEMODEL_H */

