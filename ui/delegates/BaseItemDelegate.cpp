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

#include "BaseItemDelegate.h"

using namespace QTournament;

BaseItemDelegate::BaseItemDelegate(int _defaultRowHeight, int _selectedRowHeight, QObject* parent)
  :QStyledItemDelegate{parent}, proxy{nullptr},
    normalFont{QFont{}}, largeFont{QFont{}}, smallFont{QFont{}}, normalFontBold{QFont{}}, largeFontBold{QFont{}}, fntMetrics{normalFont},
    fntMetricsLarge{QFont{}}, fntMetricsSmall{QFont{}}, selectedRow{-1},
    defaultRowHeight{_defaultRowHeight}, selectedRowHeight{_selectedRowHeight}
{
  largeFont.setPointSizeF(largeFont.pointSizeF() * DEFAULT_LARGE_FONT_FAC);
  fntMetricsLarge = QFontMetrics{largeFont};

  largeFontBold.setPointSizeF(largeFont.pointSizeF() * DEFAULT_LARGE_FONT_FAC);
  largeFontBold.setBold(true);

  normalFontBold.setBold(true);

  smallFont.setPointSizeF(largeFont.pointSizeF() * DEFAULT_SMALL_FONT_FAC);
  fntMetricsSmall = QFontMetrics{smallFont};
}

//----------------------------------------------------------------------------

void BaseItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  // if necessary, apply a conversion between the proxy model's row number
  // and the source model's row number
  int row = index.row();
  if (proxy)
  {
    row = (proxy->mapToSource(index)).row();
  }

  // save the current painter state
  painter->save();

  // one of the two paint-subfunction that shall be overriden
  // by derived classes
  bool isSelected = (option.state & QStyle::State_Selected);
  if (isSelected)
  {
    QColor bgColor = option.palette.color(QPalette::Highlight);
    painter->fillRect(option.rect, bgColor);

    paintSelectedCell(painter, option, index, row);
  } else {
    paintUnselectedCell(painter, option, index, row);
  }

  // restore and we're done
  painter->restore();
}

//----------------------------------------------------------------------------

QSize BaseItemDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  QString txt = index.data(Qt::DisplayRole).toString();
  int width = fntMetrics.horizontalAdvance(txt);  // this is wrong if we use the large font, but hey...

  int height = defaultRowHeight;
  if (selectedRowHeight >= 0)
  {
    int row = index.row();
    if (row == selectedRow) height = selectedRowHeight;
  }

  return QSize(width, height);
}

//----------------------------------------------------------------------------

void BaseItemDelegate::paintSelectedCell(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index, int srcRowId) const
{
  // Paint the background in the selection color
  QColor bgColor = option.palette.color(QPalette::Highlight);
  painter->fillRect(option.rect, bgColor);

  // draw text in highlighted cells in white bold text, centered in the cell
  painter->setPen(QPen(QColor(Qt::white)));
  painter->setFont(normalFontBold);
  painter->drawText(option.rect, Qt::AlignCenter|Qt::AlignHCenter, index.data(Qt::DisplayRole).toString());
}

//----------------------------------------------------------------------------

void BaseItemDelegate::paintUnselectedCell(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index, int srcRowId) const
{
  // draw text in regular cells in normal text, centered in the cell
  painter->drawText(option.rect, Qt::AlignCenter|Qt::AlignHCenter, index.data(Qt::DisplayRole).toString());
}
