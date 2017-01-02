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

#ifndef PLAYERITEMDELEGATE_H
#define	PLAYERITEMDELEGATE_H

#include <QStyledItemDelegate>
#include <QFontMetrics>
#include <QAbstractProxyModel>

#include "TournamentDB.h"

using namespace QTournament;

class PlayerItemDelegate : public QStyledItemDelegate
{
public:
  PlayerItemDelegate(TournamentDB* _db, QObject* parent = 0);
  void setProxy(QAbstractProxyModel* _proxy);
  void paint (QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const;
  QSize sizeHint (const QStyleOptionViewItem& option, const QModelIndex& index ) const;
  
private:
  TournamentDB* db;
  QAbstractProxyModel* proxy;
  QFontMetrics fntMetrics;
} ;

#endif	/* PLAYERITEMDELEGATE_H */

