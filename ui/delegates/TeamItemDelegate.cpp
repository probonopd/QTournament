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


#include "TeamItemDelegate.h"

#include <QPainter>

#include "Team.h"
#include "TeamMngr.h"

using namespace QTournament;

TeamItemDelegate::TeamItemDelegate(TournamentDB* _db, QObject* parent)
: QStyledItemDelegate(parent), db(_db), /*proxy(nullptr),*/ fntMetrics(QFontMetrics(QFont()))
{
}

//----------------------------------------------------------------------------

/*void TeamItemDelegate::setProxy(QAbstractProxyModel* _proxy)
{
  proxy = _proxy;
}*/

//----------------------------------------------------------------------------

void TeamItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  // if necessary, apply a conversion between the proxy model's row number
  // and the source model's row number
  int row = index.row();
  //if (proxy)
  //{
  //  row = (proxy->mapToSource(index)).row();
  //}

  TeamMngr tm{db};
  Team t = tm.getTeamBySeqNum(row);

  painter->save();
  
  // Paint the background in the selection color, if necessary
  QColor bgColor = option.palette.color(QPalette::Background);
  if(option.state & QStyle::State_Selected)
  {
    bgColor = option.palette.color(QPalette::Highlight);
    painter->fillRect(option.rect, bgColor);

    // draw text in highlighted cells in white bold text
    painter->setPen(QPen(QColor(Qt::white)));
    QFont fnt;
    fnt.setBold(true);
    painter->setFont(fnt);
  }
  
  QRect r = option.rect;
  
  // paint logic for the first column, the name
  if (index.column() == 0)
  {
    // draw the name
    r.adjust(ITEM_MARGIN, 0, 0, 0);
    QString txt = t.getName();
    painter->drawText(r, Qt::AlignVCenter|Qt::AlignLeft, txt);
  } else {
    painter->drawText(option.rect, Qt::AlignCenter, index.data(Qt::DisplayRole).toString());
  }
 
  painter->restore();
}

//----------------------------------------------------------------------------

QSize TeamItemDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
  QString txt = index.data(Qt::DisplayRole).toString() + 2 * ITEM_MARGIN;
  int width = fntMetrics.width(txt);
  
  return QSize(width, ITEM_HEIGHT);
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
    

