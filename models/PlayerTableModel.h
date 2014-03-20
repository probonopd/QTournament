/* 
 * File:   PlayerTableModel.h
 * Author: volker
 *
 * Created on March 17, 2014, 7:51 PM
 */

#ifndef PLAYERTABLEMODEL_H
#define	PLAYERTABLEMODEL_H

#include <QAbstractTableModel>

#include "DbTab.h"
#include "TournamentDB.h"
#include "Player.h"

using namespace QTournament;

class PlayerTableModel : public QAbstractTableModel
{
  Q_OBJECT
  
public:
  PlayerTableModel (TournamentDB* _db);
  int rowCount(const QModelIndex & parent = QModelIndex()) const;
  int columnCount(const QModelIndex & parent = QModelIndex()) const;
  QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
  ERR createNewPlayer (const QString& firstName, const QString& lastName, SEX sex, const QString& teamName);
  
  private:
    TournamentDB* db;
    dbOverlay::DbTab playerTab;
    dbOverlay::DbTab teamTab;
    dbOverlay::DbTab catTab;
    
public slots:
  void onPlayerCreated(const Player& newPlayer);
  void onTeamRenamed(int teamSeqNum);

};

#endif	/* PLAYERTABLEMODEL_H */

