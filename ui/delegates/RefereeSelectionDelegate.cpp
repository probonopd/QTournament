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
#include <QPainter>
#include <QDateTime>

#include "Match.h"
#include "PlayerMngr.h"
#include "ui/GuiHelpers.h"
#include "RefereeSelectionDelegate.h"
#include "DelegateItemLED.h"

// allocate static variables
constexpr int RefereeSelectionDelegate::WinnerTag;
constexpr int RefereeSelectionDelegate::LoserTag;
constexpr int RefereeSelectionDelegate::NeutralTag;


void RefereeSelectionDelegate::paintSelectedCell(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index, int srcRowId) const
{
  commonPaint(painter, option, index, true);
}

//----------------------------------------------------------------------------

void RefereeSelectionDelegate::paintUnselectedCell(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index, int srcRowId) const
{
  commonPaint(painter, option, index, false);
}

//----------------------------------------------------------------------------

void RefereeSelectionDelegate::commonPaint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index, bool isSelected) const
{
  PlayerMngr pm{db};
  int playerId = index.data(Qt::UserRole).toInt();
  auto p = pm.getPlayer2(playerId);
  if (p == nullptr) return;

  ObjState plStat = p->getState();

  // Fill the first cell with the color that indicates the player state
  int col = index.column();
  if ((col == 0) && (DelegateItemLED::state2color.contains(plStat)))
  {
    painter->fillRect(option.rect, DelegateItemLED::state2color[plStat]);
  }

  // determine the font color
  // default: set to white if we're selected
  QColor fntColor{0,0,0};
  if (isSelected)
  {
    fntColor = QColor(Qt::white);
  }

  // override with red / green in the name column if
  // a winner / loser tag is set
  if (col == 1)
  {
    int tag = index.data(Qt::UserRole + 1).toInt();

    if (tag == WinnerTag)
    {
      fntColor = isSelected ? QColor{Qt::green} : QColor{Qt::darkGreen};
    }

    if (tag == LoserTag)
    {
      fntColor = isSelected ? QColor{255,80,80} : QColor{Qt::red};
    }
  }
  painter->setPen(QPen(fntColor));

  // determine the font weight
  if (isSelected) painter->setFont(normalFontBold);

  // determine the horizontal alignment and margins
  int horAlign = Qt::AlignCenter;
  QRect contentRect = option.rect;
  if ((col == 1) || (col == 2)) // name or team
  {
    horAlign = Qt::AlignLeft;
    contentRect.adjust(ItemMargin, 0, 0, 0);
  }

  // actually draw the text
  painter->drawText(contentRect, Qt::AlignVCenter | horAlign, index.data(Qt::DisplayRole).toString());
}
