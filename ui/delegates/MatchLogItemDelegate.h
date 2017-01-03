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

using namespace QTournament;

class MatchLogItemDelegate : public QStyledItemDelegate
{
  Q_OBJECT

public:
  static constexpr int ITEM_ROW_HEIGHT = 30;
  static constexpr int ITEM_ROW_HEIGHT_SELECTED = 140;
  static constexpr int ITEM_MARGIN = 5;

  static constexpr double LARGE_TEXT_SIZE_FAC = 1.2;
  static constexpr double ITEM_TEXT_ROW_SKIP_PERC = 0.2;

  MatchLogItemDelegate(TournamentDB* _db, QObject* parent = 0);

  void paint (QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const;
  QSize sizeHint (const QStyleOptionViewItem& option, const QModelIndex& index ) const;
  void setSelectedRow(int _selRow);

protected:
  TournamentDB* db;
  QFont normalFont;
  QFont largeFont;
  QFontMetricsF fntMetrics;
  QFontMetricsF fntMetrics_Large;
  int selectedRow;

  void paintMatchInfoCell_Unselected(QPainter* painter, const QStyleOptionViewItem& option, const Match& ma) const;
  void paintMatchInfoCell_Selected(QPainter* painter, const QStyleOptionViewItem& option, const Match& ma) const;

};

#endif // MATCHLOGITEMDELEGATE_H
