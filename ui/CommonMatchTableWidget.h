/*
 *    This is QTournament, a badminton tournament management program.
 *    Copyright (C) 2014 - 2019  Volker Knollmann
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

#ifndef COMMONMATCHTABLEWIDGET_H
#define COMMONMATCHTABLEWIDGET_H

#include <QTableWidget>
#include <QAbstractItemDelegate>

#include "TournamentDB.h"
#include "Match.h"
#include "delegates/MatchLogItemDelegate.h"
#include "AutoSizingTable.h"


class CommonMatchTableWidget : public GuiHelpers::AutoSizingTableWidget_WithDatabase
{
  Q_OBJECT

public:
  static constexpr int IdxMatchNumCol = 0;
  static constexpr int IdxConfigCol = 1;
  static constexpr int IdxRoundCol = 2;
  static constexpr int IdxGrpCol = 3;
  static constexpr int IdxMatchInfoCol = 4;
  static constexpr int IdxStartTimeCol = 5;
  static constexpr int IdxFinishTimeCol = 6;
  static constexpr int IdxDurationCol = 7;
  static constexpr int IdxCourtCol = 8;
  static constexpr int IdxUmpireCol = 9;

  CommonMatchTableWidget(QWidget* parent);

  void insertMatch(int beforeRowIdx, const QTournament::Match& ma);
  void appendMatch(const QTournament::Match& ma);
  void appendMatchList(const QList<QTournament::Match>& maList) {
    for (const QTournament::Match& ma : maList) appendMatch(ma);
  }

protected slots:
  void onSelectionChanged(const QItemSelection&selectedItem, const QItemSelection&deselectedItem);

protected:
  static constexpr int MaxNumericColWidth = 90;
  static constexpr int RelNumericColWidth = 1;
  static constexpr int RelMatchInfoColWidth = 10;
  static constexpr int RelUmpireColWidth = 2;

  MatchLogItemDelegate* logItemDelegate;

  virtual void hook_onDatabaseOpened() override;
};

#endif // MATCHLOGTABLE_H
