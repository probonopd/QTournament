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


void CatTabPlayerItemDelegate::paintSelectedCell(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index, int srcRowId) const
{
  commonPaint(painter, option, index);
}

//----------------------------------------------------------------------------

void CatTabPlayerItemDelegate::paintUnselectedCell(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index, int srcRowId) const
{
  commonPaint(painter, option, index);
}

//----------------------------------------------------------------------------

void CatTabPlayerItemDelegate::commonPaint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  // get the rectangle that's available for painting the pair item
  QRect r = option.rect;

  // add the left margin
  r.adjust(ItemLeftMargin, 0, 0, 0);

  // paint the row number
  QRect numberRect = r;
  numberRect.setWidth(maxNumberColumnWidth);
  painter->drawText(numberRect, Qt::AlignVCenter|Qt::AlignRight, QString::number(index.row() + 1) + ".");

  // paint the text
  QRect textRect = r;
  textRect.adjust(maxNumberColumnWidth + NumberTextGap, 0, 0, 0);
  painter->drawText(textRect, Qt::AlignVCenter|Qt::AlignLeft, index.data().toString());
  //painter->restore();
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
    

