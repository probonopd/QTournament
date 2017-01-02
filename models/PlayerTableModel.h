/*
 *    This is QTournament, a badminton tournament management program.
 *    Copyright (C) 2014 - 2017  Volker Knollmann
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

#ifndef PLAYERTABLEMODEL_H
#define	PLAYERTABLEMODEL_H

#include <QAbstractTableModel>

#include <SqliteOverlay/DbTab.h>
#include "TournamentDB.h"
#include "Player.h"


namespace QTournament
{

  class Tournament;

  class PlayerTableModel : public QAbstractTableModel
  {
    Q_OBJECT

  public:
    static constexpr int COLUMN_COUNT = 5;  // number of columns in the model
    static constexpr int COL_NAME = 0;
    static constexpr int FILL_COL = 4;   // an extra column (empty) just to fill up the empty space in the view

    PlayerTableModel (TournamentDB* _db);
    int rowCount(const QModelIndex & parent = QModelIndex()) const;
    int columnCount(const QModelIndex & parent = QModelIndex()) const;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

  private:
    TournamentDB* db;
    SqliteOverlay::DbTab* playerTab;
    SqliteOverlay::DbTab* teamTab;
    SqliteOverlay::DbTab* catTab;
    
  public slots:
    void onBeginCreatePlayer();
    void onEndCreatePlayer(int newPlayerSeqNum);
    void onPlayerRenamed(const Player& p);
    void onTeamRenamed(int teamSeqNum);
    void onPlayerStatusChanged(int playerId, int playerSeqNum);
    void onBeginDeletePlayer(int playerSeqNum);
    void onEndDeletePlayer();
    void onBeginResetModel();
    void onEndResetModel();

  };

}

#endif	/* PLAYERTABLEMODEL_H */

