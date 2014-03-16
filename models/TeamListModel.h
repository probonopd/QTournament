/* 
 * File:   TeamListModel.h
 * Author: volker
 *
 * Created on March 14, 2014, 9:04 PM
 */

#ifndef TEAMLISTMODEL_H
#define	TEAMLISTMODEL_H

#include <QAbstractListModel>

#include "TournamentDB.h"
#include "TournamentDataDefs.h"
#include "DbTab.h"

namespace QTournament
{

  class TeamListModel : public QAbstractListModel
  {
    Q_OBJECT

  public:
    TeamListModel (TournamentDB* _db);
    int rowCount (const QModelIndex &parent = QModelIndex ()) const;
    QVariant data (const QModelIndex &index, int role) const;
    QVariant headerData (int section, Qt::Orientation orientation,
                         int role = Qt::DisplayRole) const;

  private:
    TournamentDB* db;
    dbOverlay::DbTab teamTab;
  };

}
#endif	/* TEAMLISTMODEL_H */

