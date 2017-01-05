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

#include "MatchItemDelegate.h"

#include <QPainter>

#include "Match.h"
#include "DelegateItemLED.h"
#include "MatchMngr.h"

using namespace QTournament;

MatchItemDelegate::MatchItemDelegate(TournamentDB* _db, QObject* parent)
: QStyledItemDelegate(parent), db(_db), proxy(nullptr), fntMetrics(QFontMetrics(QFont()))
{
}

//----------------------------------------------------------------------------

void MatchItemDelegate::setProxy(QAbstractProxyModel *_proxy)
{
  proxy = _proxy;
}

//----------------------------------------------------------------------------

void MatchItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  // if necessary, apply a conversion between the proxy model's row number
  // and the source model's row number
  int row = index.row();
  if (proxy)
  {
    row = (proxy->mapToSource(index)).row();
  }
  MatchMngr mm{db};
  auto ma = mm.getMatchBySeqNum(row);
  
  painter->save();

  // Fill the cell with the selection color, if necessary
  if(option.state & QStyle::State_Selected)
  {
    QColor bgColor = option.palette.color(QPalette::Highlight);
    painter->fillRect(option.rect, bgColor);

    // draw text in highlighted cells in white bold text
    painter->setPen(QPen(QColor(Qt::white)));
    QFont fnt;
    fnt.setBold(true);
    painter->setFont(fnt);
  }
  
  QRect r = option.rect;
  
  // paint logic for the second column, the match description
  if (index.column() == 1)
  {
    // draw a status indicator ("LED light")
    DelegateItemLED{}(painter, r, ITEM_MARGIN, ITEM_STAT_INDICATOR_SIZE, ma->getState(), Qt::blue);

    // draw the name
    r.adjust(2 * ITEM_MARGIN + ITEM_STAT_INDICATOR_SIZE, 0, 0, 0);
    QString txt = ma->getDisplayName(tr("Winner"), tr("Loser"));
    painter->drawText(r, Qt::AlignVCenter|Qt::AlignLeft, txt);
  } else {
    painter->drawText(option.rect, Qt::AlignCenter, index.data(Qt::DisplayRole).toString());
  }
 
  painter->restore();
}

//----------------------------------------------------------------------------

QSize MatchItemDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
  QString txt = index.data(Qt::DisplayRole).toString();
  int width = fntMetrics.width(txt) + 2 * ITEM_MARGIN;
  
  if (index.column() == 1)
  {
    width += ITEM_MARGIN + ITEM_STAT_INDICATOR_SIZE;
  }
  
  // this doesn't work, because option.state is not yet updated
  // to QtStyle::State_Selected when sizeHint is called for a freshly
  // selected item
  //int height = (option.state & QStyle::State_Selected) ? ITEM_ROW_HEIGHT_SELECTED : ITEM_ROW_HEIGHT;

  int row = index.row();
  int height = (row == selectedRow) ? ITEM_ROW_HEIGHT_SELECTED : ITEM_ROW_HEIGHT;
  //int height = ITEM_ROW_HEIGHT;

  return QSize(width, height);
}

//----------------------------------------------------------------------------

void MatchItemDelegate::setSelectedRow(int _selRow)
{
  selectedRow = _selRow;
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
    

