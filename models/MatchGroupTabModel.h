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

#ifndef MATCHGROUPTABLEMODEL_H
#define	MATCHGROUPTABLEMODEL_H

#include <QAbstractTableModel>

#include <SqliteOverlay/DbTab.h>
#include "TournamentDB.h"
#include "MatchGroup.h"

namespace QTournament
{

  class Tournament;

  class MatchGroupTableModel : public QAbstractTableModel
  {
    Q_OBJECT

  public:
    static constexpr int StateColId = 4;  // id of the column with the match group state
    static constexpr int StageSeqColId = 5;  // id of the column with the stage sequence number
    static constexpr int ColumnCount = 6;  // number of columns in the model

    MatchGroupTableModel (const QTournament::TournamentDB& _db);
    int rowCount(const QModelIndex & parent = QModelIndex()) const;
    int columnCount(const QModelIndex & parent = QModelIndex()) const;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

  private:
    std::reference_wrapper<const QTournament::TournamentDB> db;
    SqliteOverlay::DbTab mgTab;
    
  public slots:
    void onBeginCreateMatchGroup();
    void onEndCreateMatchGroup(int newMatchGroupSeqNum);
    void onMatchGroupStatusChanged(int matchGroupId, int matchGroupSeqNum);
    void onBeginResetModel();
    void onEndResetModel();

  signals:
    void triggerFilterUpdate();

  };

}
#endif	/* MATCHGROUPTABLEMODEL_H */

