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

using namespace QTournament;

class MatchGroupTableModel : public QAbstractTableModel
{
  Q_OBJECT
  
public:
  MatchGroupTableModel (TournamentDB* _db);
  int rowCount(const QModelIndex & parent = QModelIndex()) const;
  int columnCount(const QModelIndex & parent = QModelIndex()) const;
  QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
  
  private:
    TournamentDB* db;
    dbOverlay::DbTab mgTab;
    
public slots:
    void onBeginCreateMatchGroup();
    void onEndCreateMatchGroup(int newGrpSeqNum);

};

#endif	/* MATCHGROUPTABLEMODEL_H */

