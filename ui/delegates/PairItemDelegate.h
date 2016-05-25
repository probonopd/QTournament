/*
 *    This is QTournament, a badminton tournament management program.
 *    Copyright (C) 2014 - 2016  Volker Knollmann
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

using namespace QTournament;

class PairItemDelegate : public QStyledItemDelegate
{
public:
  PairItemDelegate(TournamentDB* _db, QObject* parent = nullptr, bool _showListIndex = false);
  void paint (QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const;
  QSize sizeHint (const QStyleOptionViewItem& option, const QModelIndex& index ) const;
  
private:
  TournamentDB* db;
  QFontMetrics fntMetrics;
  bool showListIndex;
} ;

#endif	/* PAIRITEMDELEGATE_H */

