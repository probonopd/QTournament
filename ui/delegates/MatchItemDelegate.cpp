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
#include "../GuiHelpers.h"

using namespace QTournament;


void MatchItemDelegate::paintSelectedCell(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index, int srcRowId) const
{
  painter->setPen(QPen(QColor(Qt::white)));
  painter->setFont(normalFontBold);

  if (index.column() == 1)
  {
    paintSelectedMatchCell(painter, option, srcRowId);
  } else {
    painter->drawText(option.rect, Qt::AlignCenter, index.data(Qt::DisplayRole).toString());
  }
}

//----------------------------------------------------------------------------

void MatchItemDelegate::paintUnselectedCell(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index, int srcRowId) const
{
  if (index.column() == 1)
  {
    paintUnselectedMatchCell(painter, option, srcRowId);
  } else {
    painter->drawText(option.rect, Qt::AlignCenter, index.data(Qt::DisplayRole).toString());
  }
}

//----------------------------------------------------------------------------

void MatchItemDelegate::paintSelectedMatchCell(QPainter* painter, const QStyleOptionViewItem& option, int srcRowId) const
{
  MatchMngr mm{*db};
  auto ma = mm.getMatchBySeqNum(srcRowId);
  if (!ma) return;  // shouldn't happen

  QRect r = option.rect;

  // draw a status indicator ("LED light") and the match description in one row
  //
  // the row starts ITEM_MARGIN from left / right side and with 2 x ITEM_MARGIN margin
  // at its top and 1 x ITEM_MARGIN at its bottom
  QRectF nameRect = r.adjusted(0, ItemMargin, -ItemMargin, 0);  // right and top
  nameRect.setHeight(fntMetrics.height() + 2 * ItemMargin);     // set height to text + top/bottom margin
  DelegateItemLED{}(painter, nameRect.toRect(), ItemMargin, ItemStatusIndicatorSize, ma->getState(), Qt::blue);
  nameRect.adjust(2 * ItemMargin + ItemStatusIndicatorSize, 0, 0, 0);
  QString txt = ma->getDisplayName(tr("Winner"), tr("Loser"));
  painter->drawText(nameRect.toRect(), Qt::AlignVCenter|Qt::AlignLeft, txt);

  // for all further text lines, indent the usable rectangle by 2 x ITEM_MARGIN
  // to the right
  nameRect.adjust(2 * ItemMargin, 0, 0, 0);

  // pre-calc the offset between two text lines
  double rowOffset = (1 + LINE_SKIP_PERC) * fntMetrics.height();

  // move the rect below the match description
  nameRect.adjust(0, rowOffset, 0, rowOffset);

  // continue with non-bold font although we're in a selected row
  QFont fnt;
  fnt.setBold(false);
  painter->setFont(fnt);

  //
  // now display the status of each player below the match description
  //
  bool hasPlayersDisplayed = false;
  if (ma->hasPlayerPair1())
  {
    auto pp = ma->getPlayerPair1();
    drawPlayerStatus(painter, nameRect, pp.getPlayer1());
    nameRect.adjust(0, rowOffset, 0, rowOffset);
    hasPlayersDisplayed = true;

    if (pp.hasPlayer2())
    {
      drawPlayerStatus(painter, nameRect, pp.getPlayer2());
      nameRect.adjust(0, rowOffset, 0, rowOffset);
    }
  }
  if (ma->hasPlayerPair2())
  {
    auto pp = ma->getPlayerPair2();
    drawPlayerStatus(painter, nameRect, pp.getPlayer1());
    nameRect.adjust(0, rowOffset, 0, rowOffset);
    hasPlayersDisplayed = true;

    if (pp.hasPlayer2())
    {
      drawPlayerStatus(painter, nameRect, pp.getPlayer2());
      nameRect.adjust(0, rowOffset, 0, rowOffset);
    }
  }

  // if we have an umpire assigned show that status as well
  auto ref = ma->getAssignedReferee();
  if (ref)
  {
    drawPlayerStatus(painter, nameRect, *ref);
    nameRect.adjust(0, rowOffset, 0, rowOffset);
    hasPlayersDisplayed = true;
  }

  // maybe we have symbolic names ("Loser of match xyz", etc).
  // then we indicate the status of the dependend match
  if (hasPlayersDisplayed)
  {
    nameRect.adjust(0, rowOffset, 0, rowOffset);   // an extra gap between players and matches
  }
  int symName = ma->getSymbolicPlayerPair1Name();
  if (symName != 0)
  {
    drawMatchStatus(painter, nameRect, abs(symName));
    nameRect.adjust(0, rowOffset, 0, rowOffset);
  }
  symName = ma->getSymbolicPlayerPair2Name();
  if (symName != 0)
  {
    drawMatchStatus(painter, nameRect, abs(symName));
    nameRect.adjust(0, rowOffset, 0, rowOffset);
  }

}

//----------------------------------------------------------------------------

void MatchItemDelegate::paintUnselectedMatchCell(QPainter* painter, const QStyleOptionViewItem& option, int srcRowId) const
{
  MatchMngr mm{*db};
  auto ma = mm.getMatchBySeqNum(srcRowId);
  if (!ma) return;  // shouldn't happen

  QRect r = option.rect;

  // draw a status indicator ("LED light")
  DelegateItemLED{}(painter, r, ItemMargin, ItemStatusIndicatorSize, ma->getState(), Qt::blue);

  // draw the name
  r.adjust(2 * ItemMargin + ItemStatusIndicatorSize, 0, 0, 0);
  QString txt = ma->getDisplayName(tr("Winner"), tr("Loser"));
  painter->drawText(r, Qt::AlignVCenter|Qt::AlignLeft, txt);
}

//----------------------------------------------------------------------------

void MatchItemDelegate::drawPlayerStatus(QPainter* painter, const QRectF& r, const Player& p) const
{
  DelegateItemLED{}(painter, r.toRect(), ItemMargin, 0.5 * ItemStatusIndicatorSize, p.getState(), Qt::white);

  QString txt = GuiHelpers::getStatusSummaryForPlayer(p);
  txt = p.getDisplayName_FirstNameFirst() + txt;
  painter->drawText(r.adjusted(2 * ItemMargin + 0.5 * ItemStatusIndicatorSize, 0, 0, 0).toRect(), Qt::AlignVCenter|Qt::AlignLeft, txt);
}

//----------------------------------------------------------------------------

void MatchItemDelegate::drawMatchStatus(QPainter* painter, const QRectF& r, int matchNum) const
{
  MatchMngr mm{*db};

  auto ma = mm.getMatchByMatchNum(matchNum);
  if (!ma) return;

  // paint a status LED
  ObjState maStat = ma->getState();
  DelegateItemLED{}(painter, r.toRect(), ItemMargin, 0.5 * ItemStatusIndicatorSize, maStat, Qt::white);

  // add some text, dependend on the match state
  QString txt = tr("Match %1 ");
  txt = txt.arg(matchNum);
  if (maStat == ObjState::MA_Running)
  {
    txt += tr(" currently running on court %1 since %2");
    auto co = ma->getCourt(nullptr);
    if (co)
    {
      txt = txt.arg(co->getNumber());
    } else {
      txt = txt.arg("??");
    }
    txt = txt.arg(GuiHelpers::qdt2durationString(ma->getStartTime()));
  }
  if ((maStat == ObjState::MA_Busy) || (maStat == ObjState::MA_Waiting))
  {
    txt += tr("is not yet ready to be called.");
  }
  if (maStat == ObjState::MA_Ready)
  {
    txt += tr("is ready to be called.");
  }
  if ((maStat == ObjState::MA_Fuzzy) || (maStat == ObjState::MA_Incomplete))
  {
    txt += tr("is incomplete and depends on other match results.");
  }

  painter->drawText(r.adjusted(2 * ItemMargin + 0.5 * ItemStatusIndicatorSize, 0, 0, 0).toRect(), Qt::AlignVCenter|Qt::AlignLeft, txt);
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
    

