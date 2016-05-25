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

#include "PairItemDelegate.h"

#include <QtGlobal>
#include <QPainter>
#include <QFont>

#include "PlayerPair.h"
#include "PlayerMngr.h"

#define PAIR_ITEM_ROW_HEIGHT 40
#define PAIR_ITEM_PLAYERNAME_COL 0, 0, 0
#define PAIR_ITEM_TEAMNAME_COL__REGULAR 0, 0, 255
#define PAIR_ITEM_TEAMNAME_COL__HIGHLIGHT 255, 255, 255
#define PAIR_ITEM_STAT_INDICATOR_SIZE 15
#define PAIR_ITEM_MARGIN 5
#define PAIR_ITEM_TEAMNAME_SHRINK_FAC 0.8
#define PAIR_ITEM_INDEX_NUMBER_SPACE 25

using namespace QTournament;

PairItemDelegate::PairItemDelegate(TournamentDB* _db, QObject* parent, bool _showListIndex)
: QStyledItemDelegate(parent), db(_db), fntMetrics(QFontMetrics(QFont())), showListIndex(_showListIndex)
{
}

//----------------------------------------------------------------------------

void PairItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  PlayerMngr pm{db};
  PlayerPair pp = pm.getPlayerPair(index.data(Qt::UserRole).toInt());
  QString playerName = pp.getDisplayName();
  QString teamName = pp.getDisplayName_Team();

  // Preceed the item with an index number?
  if (showListIndex)
  {
  }
  
  // Paint the background in the selection color if necessary
  QColor teamFontColor(PAIR_ITEM_TEAMNAME_COL__REGULAR);
  if(option.state & QStyle::State_Selected)
  {
    painter->fillRect(option.rect, option.palette.color(QPalette::Highlight));
    teamFontColor = QColor(PAIR_ITEM_TEAMNAME_COL__HIGHLIGHT);
  }
  
  // get the rectangle that's available for painting the pair item
  QRect r = option.rect;

  // apply the left and right margin
  r.adjust(PAIR_ITEM_MARGIN, 0, -PAIR_ITEM_MARGIN, 0);

  if (showListIndex)
  {
    QString playerIndex = QString::number(index.row() + 1) + ". ";
    QRect rPlayerIndex = r.adjusted(0, PAIR_ITEM_MARGIN, 0, -r.height() / 2);
    rPlayerIndex.setWidth(PAIR_ITEM_INDEX_NUMBER_SPACE);
    painter->drawText(rPlayerIndex, Qt::AlignVCenter|Qt::AlignRight, playerIndex);

    // shift all further text painting left, so that the index number
    // builds a separate "column"
    r.adjust(PAIR_ITEM_INDEX_NUMBER_SPACE, 0, 0, 0);
  }

  QRect rPlayerName = r.adjusted(0, PAIR_ITEM_MARGIN, 0, -r.height() / 2);
  painter->drawText(rPlayerName, Qt::AlignVCenter|Qt::AlignLeft, playerName);
  
  QFont teamFont = QFont();
  teamFont.setItalic(true);
  teamFont.setPointSizeF(teamFont.pointSizeF() * PAIR_ITEM_TEAMNAME_SHRINK_FAC);
  painter->save();
  painter->setPen(QPen(teamFontColor));
  painter->setFont(teamFont);
  QRect rTeamName = r.adjusted(0, r.height() / 2, 0, -PAIR_ITEM_MARGIN);
  painter->drawText(rTeamName, Qt::AlignVCenter|Qt::AlignLeft, teamName);
  painter->restore();
}

//----------------------------------------------------------------------------

QSize PairItemDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
  PlayerMngr pm{db};
  PlayerPair pp = pm.getPlayerPair(index.data(Qt::UserRole).toInt());
  QString playerName = pp.getDisplayName();
  QString teamName = pp.getDisplayName_Team();
  
  int width = qMax(fntMetrics.width(playerName), fntMetrics.width(teamName));
  width += 2 * PAIR_ITEM_MARGIN;
  
  return QSize(width, PAIR_ITEM_ROW_HEIGHT);
}

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

