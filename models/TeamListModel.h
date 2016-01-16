/* 
 * File:   TeamListModel.h
 * Author: volker
 *
 * Created on March 14, 2014, 9:04 PM
 */

#ifndef TEAMLISTMODEL_H
#define	TEAMLISTMODEL_H

#include <QAbstractListModel>

#include "TournamentDataDefs.h"
#include "TournamentDB.h"
#include "DbTab.h"
#include "Team.h"

namespace QTournament
{

  class Tournament;

  class TeamListModel : public QAbstractListModel
  {
    Q_OBJECT

  public:
    TeamListModel (Tournament* tnmt);
    int rowCount (const QModelIndex &parent = QModelIndex ()) const;
    QVariant data (const QModelIndex &index, int role) const;
    QVariant headerData (int section, Qt::Orientation orientation,
                         int role = Qt::DisplayRole) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    
  private:
    TournamentDB* db;
    SqliteOverlay::DbTab teamTab;
    
  public slots:
    void onBeginCreateTeam();
    void onEndCreateTeam(int newTeamSeqNum);
    void onTeamRenamed(int teamSeqNum);
  };

}
#endif	/* TEAMLISTMODEL_H */

