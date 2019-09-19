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

#ifndef PAIRITEMDELEGATE_H
#define	PAIRITEMDELEGATE_H

#include <QStyledItemDelegate>
#include <QFontMetrics>

#include "TournamentDB.h"
#include "BaseItemDelegate.h"


class PairItemDelegate : public BaseItemDelegate
{
public:
  static constexpr int PairItemRowHeight = 40;
  static constexpr int PairNameRole = Qt::UserRole + 1;
  static constexpr int TeamNameRole = Qt::UserRole + 2;

  PairItemDelegate(QObject* parent = nullptr, bool _showListIndex = false)
    :BaseItemDelegate{PairItemRowHeight, -1, parent}, showListIndex{_showListIndex},
     teamFont{smallFont}
  {
    teamFont.setItalic(true);
  }

protected:
  static constexpr int PairItemStatusIndicatorSize = 15;
  static constexpr int PairItemMargin = 5;
  static constexpr int PairItemIndexNumberSpace = 25;

  virtual void paintSelectedCell(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index, int srcRowId) const override;
  virtual void paintUnselectedCell(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index, int srcRowId) const override;
  void commonPaint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index, const QColor& teamFontColor) const;

private:
  bool showListIndex;
  QFont teamFont;
} ;

#endif	/* PAIRITEMDELEGATE_H */

