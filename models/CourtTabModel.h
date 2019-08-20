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

#ifndef COURTTABLEMODEL_H
#define	COURTTABLEMODEL_H

#include <QAbstractTableModel>
#include <QTimer>

#include <SqliteOverlay/DbTab.h>
#include "TournamentDB.h"
#include "Court.h"

namespace QTournament
{

  class Tournament;

  class CourtTableModel : public QAbstractTableModel
  {
    Q_OBJECT

  public:
    static constexpr int CourtNumColId = 0;  // id of the column with the court number
    static constexpr int DurationColId = 2;  // id of the column with match duration
    static constexpr int ColumnCount = 3;  // number of columns in the model

    static constexpr int DurationUpdatePeriod_ms = 10000;   // update every 10 seconds

    CourtTableModel (const QTournament::TournamentDB& _db);
    int rowCount(const QModelIndex & parent = QModelIndex()) const;
    int columnCount(const QModelIndex & parent = QModelIndex()) const;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

  private:
    std::reference_wrapper<const QTournament::TournamentDB> db;
    SqliteOverlay::DbTab courtTab;
    std::unique_ptr<QTimer> durationUpdateTimer;

  public slots:
    void onBeginCreateCourt();
    void onEndCreateCourt(int newCourtSeqNum);
    void onCourtStatusChanged(int courtId, int courtSeqNum);
    void onDurationUpdateTimerElapsed();
    void onBeginDeleteCourt(int courtSeqNum);
    void onEndDeleteCourt();
  };

}
#endif	/* COURTTABLEMODEL_H */

