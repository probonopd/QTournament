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

#define PLAYER_ITEM_ROW_HEIGHT 30
#define PLAYER_ITEM_MALE_BG_COL 215, 238, 244
#define PLAYER_ITEM_FEMALE_BG_COL 255, 220, 220
#define PLAYER_ITEM_STAT_INDICATOR_SIZE 15
#define PLAYER_ITEM_MARGIN 5

using namespace QTournament;

PlayerItemDelegate::PlayerItemDelegate(TournamentDB* _db, QObject* parent)
: QStyledItemDelegate(parent), db(_db), proxy(nullptr), fntMetrics(QFontMetrics(QFont()))
{
}

//----------------------------------------------------------------------------

void PlayerItemDelegate::setProxy(QAbstractProxyModel *_proxy)
{
  proxy = _proxy;
}

//----------------------------------------------------------------------------

void PlayerItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  // if necessary, apply a conversion between the proxy model's row number
  // and the source model's row number
  int row = index.row();
  if (proxy)
  {
    row = (proxy->mapToSource(index)).row();
  }
  PlayerMngr pm{db};
  auto p = pm.getPlayerBySeqNum(row);
  // no check for a nullptr here, the call above MUST succeed
  
  // use a gray text color if the player is state "WAIT_FOR_REGISTRATION"
  OBJ_STATE plStat = p->getState();
  QColor txtColor{(plStat == STAT_PL_WAIT_FOR_REGISTRATION) ? Qt::darkGray : Qt::black};

  // Paint the background, either in the selection color or in a color related
  // to the participant's sex
  QColor bgColor = QColor(PLAYER_ITEM_MALE_BG_COL);
  if(option.state & QStyle::State_Selected)
  {
    bgColor = option.palette.color(QPalette::Highlight);

    // use light gray for selected players in state WAIT_FOR_REGISTRATION,
    // else use white text
    txtColor = (plStat == STAT_PL_WAIT_FOR_REGISTRATION) ? QColor(Qt::lightGray) : QColor(Qt::white);
  } else {
    if (p->getSex() == F) bgColor = QColor(PLAYER_ITEM_FEMALE_BG_COL);
  }
  painter->fillRect(option.rect, bgColor);
  
  QRect r = option.rect;

  // paint logic for the first column, the name
  if (index.column() == 0)
  {
    // draw a status indicator ("LED light")
    DelegateItemLED{}(painter, r, PLAYER_ITEM_MARGIN, PLAYER_ITEM_STAT_INDICATOR_SIZE, p->getState());

    // draw the name
    r.adjust(2 * PLAYER_ITEM_MARGIN + PLAYER_ITEM_STAT_INDICATOR_SIZE, 0, 0, 0);
    QString txt = p->getDisplayName();
    painter->save();
    painter->setPen(txtColor);
    painter->drawText(r, Qt::AlignVCenter|Qt::AlignLeft, txt);
    painter->restore();
  } else {
    painter->save();
    painter->setPen(txtColor);
    painter->drawText(option.rect, Qt::AlignCenter, index.data(Qt::DisplayRole).toString());
    painter->restore();
  }
 
  
}

//----------------------------------------------------------------------------

QSize PlayerItemDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
  QString txt = index.data(Qt::DisplayRole).toString();
  int width = fntMetrics.width(txt) + 2 * PLAYER_ITEM_MARGIN;
  
  if (index.column() == 0)
  {
    width += PLAYER_ITEM_MARGIN + PLAYER_ITEM_STAT_INDICATOR_SIZE;
  }
  
  return QSize(width, PLAYER_ITEM_ROW_HEIGHT);
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
    

