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

#ifndef CATTABLEMODEL_H
#define	CATTABLEMODEL_H

#include <QAbstractTableModel>

#include <SqliteOverlay/DbTab.h>
#include "TournamentDB.h"
#include "Player.h"

namespace QTournament
{

  class Tournament;

  class CategoryTableModel : public QAbstractTableModel
  {
    Q_OBJECT

  public:
    static constexpr int ColumnCount = 8;  // number of columns in the model

    static constexpr int ColName = 0;
    static constexpr int ColFinishedRounds = 3;
    static constexpr int ColCurrentRound = 2;
    static constexpr int ColTotalRounds = 1;
    static constexpr int ColUnfinishedMatches = 4;
    static constexpr int ColRunningMatches = 5;
    static constexpr int ColWaitingMatches = 6;
    static constexpr int ColTotalMatches = 7;

    CategoryTableModel (const QTournament::TournamentDB& _db);
    int rowCount(const QModelIndex & parent = QModelIndex()) const;
    int columnCount(const QModelIndex & parent = QModelIndex()) const;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

  private:
    std::reference_wrapper<const QTournament::TournamentDB> db;
    SqliteOverlay::DbTab catTab;
    
  public slots:
    void onBeginCreateCategory();
    void onEndCreateCategory(int newCatSeqNum);
    void onBeginDeleteCategory(int catSeqNum);
    void onEndDeleteCategory();
    void onBeginResetModel();
    void onEndResetModel();

  };

}

#endif	/* CATTABLEMODEL_H */

