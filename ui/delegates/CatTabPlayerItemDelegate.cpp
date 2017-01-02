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
  // determine the maximum width of the index number column
  maxNumberColumnWidth = fntMetrics.width("88.");
}

//----------------------------------------------------------------------------

void CatTabPlayerItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
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

  // paint the row number
  QRect numberRect = r;
  numberRect.setWidth(maxNumberColumnWidth);
  painter->drawText(numberRect, Qt::AlignVCenter|Qt::AlignRight, QString::number(index.row() + 1) + ".");

  // paint the text
  QRect textRect = r;
  textRect.adjust(maxNumberColumnWidth + NUMBER_TEXT_GAP, 0, 0, 0);
  painter->drawText(textRect, Qt::AlignVCenter|Qt::AlignLeft, index.data().toString());
  painter->restore();
}

//----------------------------------------------------------------------------

QSize CatTabPlayerItemDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
  int width = maxNumberColumnWidth + NUMBER_TEXT_GAP + fntMetrics.width(index.data().toString());
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
    

