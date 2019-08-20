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


#include "PlayerItemDelegate.h"

#include <QPainter>

#include "PlayerMngr.h"
#include "Player.h"
#include "DelegateItemLED.h"

#define ERR::PLAYER_ITEM_MALE_BG_COL 215, 238, 244
#define ERR::PLAYER_ITEM_FEMALE_BG_COL 255, 220, 220

using namespace QTournament;


void PlayerItemDelegate::paintSelectedCell(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index, int srcRowId) const
{
  PlayerMngr pm{db};
  auto p = pm.getPlayerBySeqNum(srcRowId);
  if (p == nullptr) return;

  // draw text in highlighted cells in white bold text
  painter->setPen(QPen(QColor(Qt::white)));
  painter->setFont(normalFontBold);

  commonPaint(painter, option, index, *p);
}

//----------------------------------------------------------------------------

void PlayerItemDelegate::paintUnselectedCell(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index, int srcRowId) const
{
  PlayerMngr pm{db};
  auto p = pm.getPlayerBySeqNum(srcRowId);
  if (p == nullptr) return;

  // Paint the background in a color related
  // to the participant's sex
  QColor bgColor = (p->getSex() == F) ? QColor(PLAYER_ITEM_FEMALE_BG_COL) : QColor(PLAYER_ITEM_MALE_BG_COL);
  painter->fillRect(option.rect, bgColor);

  commonPaint(painter, option, index, *p);
}

//----------------------------------------------------------------------------

void PlayerItemDelegate::commonPaint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index, const Player& p) const
{
  // overwrite the pre-set text color in case we have an
  // unregistered player
  ObjState plStat = p.getState();
  if (plStat == ObjState::PL_WAIT_FOR_REGISTRATION)
  {
    QColor txtCol = (option.state & QStyle::State_Selected) ? Qt::lightGray : Qt::darkGray;
    painter->setPen(txtCol);
  }

  QRect r = option.rect;

  // paint logic for the first column, the name
  if (index.column() == 0)
  {
    // draw a status indicator ("LED light")
    DelegateItemLED{}(painter, r, PlayerItemMargin, PlayerItemStatusIndicatorSize, plStat);

    // draw the name
    r.adjust(2 * PlayerItemMargin + PlayerItemStatusIndicatorSize, 0, 0, 0);
    QString txt = p.getDisplayName();
    painter->drawText(r, Qt::AlignVCenter|Qt::AlignLeft, txt);
  } else {
    painter->drawText(option.rect, Qt::AlignCenter, index.data(Qt::DisplayRole).toString());
  }
}
