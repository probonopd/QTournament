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

#include "PairItemDelegate.h"

#include <QtGlobal>
#include <QPainter>
#include <QFont>

#include "PlayerPair.h"
#include "PlayerMngr.h"

#define PAIR_ITEM_PLAYERNAME_COL 0, 0, 0
#define PAIR_ITEM_TEAMNAME_COL_REGULAR 0, 0, 255
#define PAIR_ITEM_TEAMNAME_COL_HIGHLIGHT 255, 255, 255

using namespace QTournament;


void PairItemDelegate::paintSelectedCell(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index, int srcRowId) const
{
  commonPaint(painter, option, index, QColor(PAIR_ITEM_TEAMNAME_COL_HIGHLIGHT));
}

void PairItemDelegate::paintUnselectedCell(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index, int srcRowId) const
{
  commonPaint(painter, option, index, QColor(PAIR_ITEM_TEAMNAME_COL_REGULAR));
}

void PairItemDelegate::commonPaint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index, const QColor& teamFontColor) const
{
  PlayerMngr pm{*db};
  PlayerPair pp = pm.getPlayerPair(index.data(Qt::UserRole).toInt());
  QString playerName = pp.getDisplayName();
  QString teamName = pp.getDisplayName_Team();

  // get the rectangle that's available for painting the pair item
  QRect r = option.rect;

  // apply the left and right margin
  r.adjust(PairItemMargin, 0, -PairItemMargin, 0);

  if (showListIndex)
  {
    QString playerIndex = QString::number(index.row() + 1) + ". ";
    QRect rPlayerIndex = r.adjusted(0, PairItemMargin, 0, -r.height() / 2);
    rPlayerIndex.setWidth(PairItemIndexNumberSpace);
    painter->drawText(rPlayerIndex, Qt::AlignVCenter|Qt::AlignRight, playerIndex);

    // shift all further text painting left, so that the index number
    // builds a separate "column"
    r.adjust(PairItemIndexNumberSpace, 0, 0, 0);
  }

  QRect rPlayerName = r.adjusted(0, PairItemMargin, 0, -r.height() / 2);
  painter->drawText(rPlayerName, Qt::AlignVCenter|Qt::AlignLeft, playerName);

  painter->setPen(QPen(teamFontColor));
  painter->setFont(smallFont);
  QRect rTeamName = r.adjusted(0, r.height() / 2, 0, -PairItemMargin);
  painter->drawText(rTeamName, Qt::AlignVCenter|Qt::AlignLeft, teamName);
}
