/* 
 * File:   PlayerTableModel.h
 * Author: volker
 *
 * Created on March 17, 2014, 7:51 PM
 */

#ifndef CATTABLEMODEL_H
#define	CATTABLEMODEL_H

#include <QAbstractTableModel>

#include "DbTab.h"
#include "TournamentDB.h"
#include "Player.h"

using namespace QTournament;

class CategoryTableModel : public QAbstractTableModel
{
  Q_OBJECT
  
public:
  static constexpr int COLUMN_COUNT = 8;  // number of columns in the model

  static constexpr int COL_NAME = 0;
  static constexpr int COL_FINISHED_ROUNDS = 3;
  static constexpr int COL_CURRENT_ROUND = 2;
  static constexpr int COL_TOTAL_ROUNDS = 1;
  static constexpr int COL_UNFINISHED_MATCHES = 4;
  static constexpr int COL_RUNNING_MATCHES = 5;
  static constexpr int COL_WAITING_MATCHES = 6;
  static constexpr int COL_TOTAL_MATCHES = 7;

  CategoryTableModel (TournamentDB* _db);
  int rowCount(const QModelIndex & parent = QModelIndex()) const;
  int columnCount(const QModelIndex & parent = QModelIndex()) const;
  QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
  
  private:
    TournamentDB* db;
    dbOverlay::DbTab catTab;
    
public slots:
    void onBeginCreateCategory();
    void onEndCreateCategory(int newCatSeqNum);
    void onBeginDeleteCategory(int catSeqNum);
    void onEndDeleteCategory();
    void onBeginResetModel();
    void onEndResetModel();

};

#endif	/* CATTABLEMODEL_H */

