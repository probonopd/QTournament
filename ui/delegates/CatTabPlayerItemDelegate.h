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

#ifndef CATTABPLAYERITEMDELEGATE_H
#define	CATTABPLAYERITEMDELEGATE_H

#include <QStyledItemDelegate>
#include <QFontMetrics>

#include "TournamentDB.h"
#include "BaseItemDelegate.h"


class CatTabPlayerItemDelegate : public BaseItemDelegate
{
public:
  static constexpr int ItemHeight = 20;
  static constexpr int ItemLeftMargin = 5;
  static constexpr int NumberTextGap = 3;
  CatTabPlayerItemDelegate(QObject* parent = nullptr, bool _showListIndex = true)
    :BaseItemDelegate{ItemHeight, -1, parent}, showListIndex{_showListIndex},
      maxNumberColumnWidth{fntMetrics.horizontalAdvance("88.")} {}

protected:
  virtual void paintSelectedCell(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index, int srcRowId) const override;
  virtual void paintUnselectedCell(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index, int srcRowId) const override;
  void commonPaint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;

private:
  bool showListIndex;
  int maxNumberColumnWidth;
} ;

#endif	/* CATTABPLAYERITEMDELEGATE_H */

