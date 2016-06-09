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

#include <QtGlobal>
#include <QPainter>
#include <QFont>

#include "CatTabPlayerItemDelegate.h"

#include "PlayerPair.h"
#include "PlayerMngr.h"

using namespace QTournament;

CatTabPlayerItemDelegate::CatTabPlayerItemDelegate(QObject* parent, bool _showListIndex)
: QStyledItemDelegate(parent), fntMetrics(QFontMetrics(QFont())), showListIndex(_showListIndex)
{
}

//----------------------------------------------------------------------------

void CatTabPlayerItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  QString itemText = getItemText(index);

  painter->save();
  
  // Paint the background in the selection color if necessary
  if (option.state & QStyle::State_Selected)
  {
    painter->fillRect(option.rect, option.palette.color(QPalette::Highlight));

    // draw text in highlighted cells in white bold text
    painter->setPen(QPen(QColor(Qt::white)));
    QFont fnt;
    fnt.setBold(true);
    painter->setFont(fnt);
  }
  
  // get the rectangle that's available for painting the pair item
  QRect r = option.rect;

  // add the left margin
  r.adjust(ITEM_LEFT_MARGIN, 0, 0, 0);

  // paint the text
  painter->drawText(r, Qt::AlignVCenter|Qt::AlignLeft, itemText);
  painter->restore();
}

//----------------------------------------------------------------------------

QString CatTabPlayerItemDelegate::getItemText(const QModelIndex& index) const
{
  QString itemText = index.data().toString();

  // Preceed the item with an index number?
  if (showListIndex)
  {
    itemText = QString::number(index.row() + 1) + ". " + itemText;
  }

  return itemText;
}

//----------------------------------------------------------------------------

QSize CatTabPlayerItemDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
  QString txt = getItemText(index);

  int width = fntMetrics.width(txt);
  width += ITEM_LEFT_MARGIN;
  
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
    

