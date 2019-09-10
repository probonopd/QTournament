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

#ifndef MATCHLOGITEMDELEGATE_H
#define MATCHLOGITEMDELEGATE_H

#include <QStyledItemDelegate>
#include <QFontMetricsF>

#include "Match.h"
#include "TournamentDB.h"
#include "BaseItemDelegate.h"


class MatchLogItemDelegate : public BaseItemDelegate
{
  Q_OBJECT

public:
  static constexpr int ItemRowHeight = 40;
  static constexpr int ItemMargin = 5;
  static constexpr double ItemTextRowSkip_Perc = 0.2;

  MatchLogItemDelegate(const QTournament::TournamentDB* _db, QObject* parent = nullptr)
    :BaseItemDelegate{ItemRowHeight, -1, parent}, db{_db} {}

protected:
  virtual void paintSelectedCell(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index, int srcRowId) const override;
  virtual void paintUnselectedCell(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index, int srcRowId) const override;

  void paintMatchInfoCell(QPainter* painter, const QStyleOptionViewItem& option, const QTournament::Match& ma, bool isSelected) const;

private:
  const QTournament::TournamentDB* db;
};

#endif // MATCHLOGITEMDELEGATE_H
