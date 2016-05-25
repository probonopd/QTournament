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
    TeamListModel (TournamentDB* _db);
    int rowCount (const QModelIndex &parent = QModelIndex ()) const;
    QVariant data (const QModelIndex &index, int role) const;
    QVariant headerData (int section, Qt::Orientation orientation,
                         int role = Qt::DisplayRole) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    
  private:
    TournamentDB* db;
    SqliteOverlay::DbTab* teamTab;
    
  public slots:
    void onBeginCreateTeam();
    void onEndCreateTeam(int newTeamSeqNum);
    void onTeamRenamed(int teamSeqNum);
  };

}
#endif	/* TEAMLISTMODEL_H */

