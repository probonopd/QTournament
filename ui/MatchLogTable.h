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

#ifndef MATCHLOGTABLE_H
#define MATCHLOGTABLE_H

#include <QTableWidget>
#include <QAbstractItemDelegate>

#include "TournamentDB.h"
#include "Match.h"
#include "delegates/MatchLogItemDelegate.h"

using namespace QTournament;

class MatchLogTable : public QTableWidget
{
  Q_OBJECT

public:
  static constexpr int IDX_MATCH_NUM_COL = 0;
  static constexpr int IDX_MATCH_INFO_COL = 1;
  static constexpr int IDX_START_TIME_COL = 2;
  static constexpr int IDX_FINISH_TIME_COL = 3;
  static constexpr int IDX_DURATION_COL = 4;
  static constexpr int IDX_COURT_COL = 5;

  MatchLogTable(QWidget* parent);
  virtual ~MatchLogTable();

  void setDatabase(TournamentDB* _db);

private slots:
  void onSelectionChanged(const QItemSelection&selectedItem, const QItemSelection&deselectedItem);

protected:
  static constexpr int MAX_NUMERIC_COL_WIDTH = 90;
  static constexpr int REL_WIDTH_NUMERIC_COL = 1;
  static constexpr int REL_WIDTH_MATCH_INFO = 10;

  TournamentDB* db;
  QAbstractItemDelegate* defaultDelegate;
  unique_ptr<MatchLogItemDelegate> logItemDelegate;

  virtual void resizeEvent(QResizeEvent *_event) override;
  void autosizeColumns();

  void fillFromDatabase();
  void prependMatch(const Match& ma);
};

#endif // MATCHLOGTABLE_H
