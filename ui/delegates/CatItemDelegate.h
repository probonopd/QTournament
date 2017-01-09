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

#ifndef CATITEMDELEGATE_H
#define	CATITEMDELEGATE_H

#include <QStyledItemDelegate>
#include <QFontMetrics>
#include <QAbstractProxyModel>

#include "TournamentDB.h"
#include "BaseItemDelegate.h"

using namespace QTournament;

class CatItemDelegate : public BaseItemDelegate
{
public:
  CatItemDelegate(TournamentDB* _db, QObject* parent = 0)
    :BaseItemDelegate(_db, CatItemRowHeight, -1, parent) {}

protected:
  static constexpr int CatItemRowHeight = 30;
  static constexpr int CatItemStatusIndicatorSize = 15;
  static constexpr int CatItemMargin = 5;

  virtual void paintSelectedCell(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index, int srcRowId) const override;
  virtual void paintUnselectedCell(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index, int srcRowId) const override;
  void commonPaint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index, int srcRowId) const;
} ;

#endif	/* CATITEMDELEGATE_H */

