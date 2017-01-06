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
  if (ma == nullptr) return;  // shouldn't happen
  
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
  
  // paint logic for the second column, the match description
  if (index.column() == 1)
  {
    if (index.row() == selectedRow)
    {
      paintSelectedMatchCell(painter, option, index, *ma);
    } else {
      paintUnselectedMatchCell(painter, option, index, *ma);
    }
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

void MatchItemDelegate::paintSelectedMatchCell(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index, const Match& ma) const
{
  //
  // assumption: painter->save and ->restore will be handled by the caller!!
  //

  QRect r = option.rect;

  // draw a status indicator ("LED light") and the match description in one row
  //
  // the row starts ITEM_MARGIN from left / right side and with 2 x ITEM_MARGIN margin
  // at its top and 1 x ITEM_MARGIN at its bottom
  QRectF nameRect = r.adjusted(0, ITEM_MARGIN, -ITEM_MARGIN, 0);  // right and top
  nameRect.setHeight(fntMetrics.height() + 2 * ITEM_MARGIN);     // set height to text + top/bottom margin
  DelegateItemLED{}(painter, nameRect.toRect(), ITEM_MARGIN, ITEM_STAT_INDICATOR_SIZE, ma.getState(), Qt::blue);
  nameRect.adjust(2 * ITEM_MARGIN + ITEM_STAT_INDICATOR_SIZE, 0, 0, 0);
  QString txt = ma.getDisplayName(tr("Winner"), tr("Loser"));
  painter->drawText(nameRect.toRect(), Qt::AlignVCenter|Qt::AlignLeft, txt);

  // for all further text lines, indent the usable rectangle by 4 x ITEM_MARGIN
  // to the right
  nameRect.adjust(4 * ITEM_MARGIN, 0, 0, 0);

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
  if (ma.hasPlayerPair1())
  {
    auto pp = ma.getPlayerPair1();
    drawPlayerStatus(painter, nameRect, pp.getPlayer1());
    nameRect.adjust(0, rowOffset, 0, rowOffset);

    if (pp.hasPlayer2())
    {
      drawPlayerStatus(painter, nameRect, pp.getPlayer2());
      nameRect.adjust(0, rowOffset, 0, rowOffset);
    }
  }
  if (ma.hasPlayerPair2())
  {
    auto pp = ma.getPlayerPair2();
    drawPlayerStatus(painter, nameRect, pp.getPlayer1());
    nameRect.adjust(0, rowOffset, 0, rowOffset);

    if (pp.hasPlayer2())
    {
      drawPlayerStatus(painter, nameRect, pp.getPlayer2());
      nameRect.adjust(0, rowOffset, 0, rowOffset);
    }
  }

  // if we have an umpire assigned show that status as well
  auto ref = ma.getAssignedReferee();
  if (ref != nullptr)
  {
    drawPlayerStatus(painter, nameRect, *ref);
    nameRect.adjust(0, rowOffset, 0, rowOffset);
  }

  // maybe we have symbolic names ("Loser of match xyz", etc).
  // then we indicate the status of the dependend match
  nameRect.adjust(0, rowOffset, 0, rowOffset);   // an extra gap between players and matches
  int symName = ma.getSymbolicPlayerPair1Name();
  if (symName != 0)
  {
    drawMatchStatus(painter, nameRect, abs(symName));
    nameRect.adjust(0, rowOffset, 0, rowOffset);
  }
  symName = ma.getSymbolicPlayerPair2Name();
  if (symName != 0)
  {
    drawMatchStatus(painter, nameRect, abs(symName));
    nameRect.adjust(0, rowOffset, 0, rowOffset);
  }

}

//----------------------------------------------------------------------------

void MatchItemDelegate::paintUnselectedMatchCell(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index, const Match& ma) const
{
  //
  // assumption: painter->save and ->restore will be handled by the caller!!
  //

  QRect r = option.rect;

  // draw a status indicator ("LED light")
  DelegateItemLED{}(painter, r, ITEM_MARGIN, ITEM_STAT_INDICATOR_SIZE, ma.getState(), Qt::blue);

  // draw the name
  r.adjust(2 * ITEM_MARGIN + ITEM_STAT_INDICATOR_SIZE, 0, 0, 0);
  QString txt = ma.getDisplayName(tr("Winner"), tr("Loser"));
  painter->drawText(r, Qt::AlignVCenter|Qt::AlignLeft, txt);
}

//----------------------------------------------------------------------------

void MatchItemDelegate::drawPlayerStatus(QPainter* painter, const QRectF& r, const Player& p) const
{
  DelegateItemLED{}(painter, r.toRect(), ITEM_MARGIN, 0.5 * ITEM_STAT_INDICATOR_SIZE, p.getState(), Qt::white);

  QString txt = GuiHelpers::getStatusSummaryForPlayer(p);
  txt = p.getDisplayName_FirstNameFirst() + txt;
  painter->drawText(r.adjusted(2 * ITEM_MARGIN + 0.5 * ITEM_STAT_INDICATOR_SIZE, 0, 0, 0).toRect(), Qt::AlignVCenter|Qt::AlignLeft, txt);
}

//----------------------------------------------------------------------------

void MatchItemDelegate::drawMatchStatus(QPainter* painter, const QRectF& r, int matchNum) const
{
  MatchMngr mm{db};

  auto ma = mm.getMatchByMatchNum(matchNum);
  if (ma == nullptr) return;

  // paint a status LED
  OBJ_STATE maStat = ma->getState();
  DelegateItemLED{}(painter, r.toRect(), ITEM_MARGIN, 0.5 * ITEM_STAT_INDICATOR_SIZE, maStat, Qt::white);

  // add some text, dependend on the match state
  QString txt = tr("Match %1 ");
  txt = txt.arg(matchNum);
  if (maStat == STAT_MA_RUNNING)
  {
    txt += tr(" currently running on court %1 since %2");
    auto co = ma->getCourt();
    if (co != nullptr)
    {
      txt = txt.arg(co->getNumber());
    } else {
      txt = txt.arg("??");
    }
    txt = txt.arg(GuiHelpers::qdt2durationString(ma->getStartTime()));
  }
  if ((maStat == STAT_MA_BUSY) || (maStat == STAT_MA_WAITING))
  {
    txt += tr("is not yet ready to be called.");
  }
  if (maStat == STAT_MA_READY)
  {
    txt += tr("is ready to be called.");
  }
  if ((maStat == STAT_MA_FUZZY) || (maStat == STAT_MA_INCOMPLETE))
  {
    txt += tr("is incomplete and depends on other match results.");
  }

  painter->drawText(r.adjusted(2 * ITEM_MARGIN + 0.5 * ITEM_STAT_INDICATOR_SIZE, 0, 0, 0).toRect(), Qt::AlignVCenter|Qt::AlignLeft, txt);
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
    

