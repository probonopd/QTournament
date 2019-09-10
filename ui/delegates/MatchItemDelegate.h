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

#ifndef MATCHITEMDELEGATE_H
#define	MATCHITEMDELEGATE_H

#include <QStyledItemDelegate>
#include <QFontMetrics>
#include <QAbstractProxyModel>

#include "TournamentDB.h"
#include "BaseItemDelegate.h"


namespace QTournament
{
  class Match;
  class Player;
}

class MatchItemDelegate : public BaseItemDelegate
{
  Q_OBJECT

public:
  static constexpr int ItemRowHeight = 30;
  static constexpr int ItemRowHeightSelected = 120;
  static constexpr int ItemStatusIndicatorSize = 15;
  static constexpr int ItemMargin = 5;

  MatchItemDelegate(const QTournament::TournamentDB* _db, QObject* parent = nullptr)
    :BaseItemDelegate{ItemRowHeight, ItemRowHeightSelected, parent}, db{_db} {}

private:
  static constexpr double LINE_SKIP_PERC = 0.2;

  virtual void paintSelectedCell(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index, int srcRowId) const override;
  virtual void paintUnselectedCell(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index, int srcRowId) const override;

  void paintSelectedMatchCell(QPainter* painter, const QStyleOptionViewItem& option, int srcRowId) const;
  void paintUnselectedMatchCell(QPainter* painter, const QStyleOptionViewItem& option, int srcRowId) const;

  void drawPlayerStatus(QPainter* painter, const QRectF& r, const QTournament::Player& p) const;
  void drawMatchStatus(QPainter* painter, const QRectF& r, int matchNum) const;

  const QTournament::TournamentDB* db;
};

#endif	/* MATCHITEMDELEGATE_H */

