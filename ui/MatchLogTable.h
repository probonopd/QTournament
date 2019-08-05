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
#include <QMenu>
#include <QAction>

#include "TournamentDB.h"
#include "Match.h"
#include "delegates/MatchLogItemDelegate.h"
#include "CommonMatchTableWidget.h"


class MatchLogTable : public CommonMatchTableWidget
{
  Q_OBJECT

public:
  MatchLogTable(QWidget* parent);
  virtual ~MatchLogTable() {}
  std::optional<QTournament::Match> getSelectedMatch() const;

public slots:
  void onMatchStatusChanged(int maId, int maSeqNum, QTournament::OBJ_STATE oldStat, QTournament::OBJ_STATE newStat);
  void onCategoryRemoved();

protected slots:
  void onModMatchResultTriggered();
  void onContextMenuRequested(const QPoint& pos);

protected:
  virtual void hook_onDatabaseOpened() override;

  void fillFromDatabase();

  std::unique_ptr<QMenu> contextMenu;
  QAction* actModMatchResult;

  void initContextMenu();
};

#endif // MATCHLOGTABLE_H
