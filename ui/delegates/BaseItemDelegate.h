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

#ifndef BASEITEMDELEGATE_H
#define BASEITEMDELEGATE_H

#include <QStyledItemDelegate>
#include <QAbstractProxyModel>
#include <QFontMetrics>
#include <QFont>
#include <QPainter>

#include "TournamentDB.h"


class BaseItemDelegate : public QStyledItemDelegate
{
  Q_OBJECT

public:
  static constexpr double DEFAULT_LARGE_FONT_FAC = 1.2;
  static constexpr double DEFAULT_SMALL_FONT_FAC = 0.8;

  BaseItemDelegate(int _defaultRowHeight, int _selectedRowHeight = -1, QObject* parent = nullptr);
  void setProxy(QAbstractProxyModel* _proxy) { proxy = _proxy; }
  virtual void paint (QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const override;
  virtual QSize sizeHint (const QStyleOptionViewItem& option, const QModelIndex& index ) const override;
  void setSelectedRow(int _selRow) { selectedRow = _selRow; }
  int getSelectedRow() const { return selectedRow; }

  virtual void paintSelectedCell(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index, int srcRowId) const;
  virtual void paintUnselectedCell(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index, int srcRowId) const;

protected:
  QAbstractProxyModel* proxy;
  QFont normalFont;
  QFont largeFont;
  QFont smallFont;
  QFont normalFontBold;
  QFont largeFontBold;
  QFontMetrics fntMetrics;
  QFontMetrics fntMetricsLarge;
  QFontMetrics fntMetricsSmall;
  int selectedRow;
  int defaultRowHeight;
  int selectedRowHeight;
};

#endif // BASEITEMDELEGATE_H
