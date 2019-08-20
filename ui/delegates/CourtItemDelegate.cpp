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


#include "CourtItemDelegate.h"

#include <QPainter>

#include <QDateTime>

#include "Match.h"
#include "DelegateItemLED.h"
#include "ui/GuiHelpers.h"
#include "CourtMngr.h"
#include "MatchGroup.h"

using namespace QTournament;


void CourtItemDelegate::paintMatchInfoCell_Selected(QPainter *painter, const QStyleOptionViewItem &option, const Match &ma) const
{
  // do we need one or two rows for the player names?
  bool isDoubles = ma.getPlayerPair1().hasPlayer2();

  // calc the vertical margin
  // because we want the content to be vertically centered in the cell,
  // the vertical margin depends on the match type
  //

  // start with the height of the info rows
  double infoBlockHeight = (3 + 2 * ItemTextRowSkip_Perc) * fntMetrics.height();

  // now estimate the height of the player name block
  //
  // include 2 * ITEM_MARGIN as some extra space between the player names
  // and the info block
  double playerNameHeight = (1 + ItemTextRowSkip_Perc) * fntMetricsLarge.height();
  if (isDoubles) playerNameHeight *= 2;
  playerNameHeight += 2 * ItemMargin;

  // put everything together and calculate the margin
  double totalTextHeight = playerNameHeight + infoBlockHeight;
  double vertMargin = (ItemRowHeightSelected - totalTextHeight) / 2.0;

  // draw the player names
  QRect r = option.rect;
  int x0 = r.x() + ItemMargin;
  int y0 = r.y() + vertMargin;
  QRectF playerRect(r.x(), r.y() + vertMargin, r.width(), playerNameHeight - 2 * ItemMargin);
  GuiHelpers::drawTwoLinePlayerPairNames_Centered(painter, playerRect, ma, "", "", ItemTextRowSkip_Perc, true, false, largeFont, QColor(Qt::yellow));

  // draw the first info line with the match number and the category
  QString txt = tr("Match number:");
  txt += " " + QString::number(ma.getMatchNumber()) + "    ";
  txt += tr("Category:");
  txt += " " + ma.getCategory().getName() + "    ";
  txt += tr("Round: ") + QString::number(ma.getMatchGroup().getRound());
  double baseline = y0 + playerNameHeight + fntMetrics.ascent();
  GuiHelpers::drawFormattedText(painter, x0, baseline, txt, true, false, normalFont, QColor(Qt::white));

  // draw a second info line with call times
  QDateTime startTime = ma.getStartTime();
  assert(startTime.isValid());
  txt = tr("Start time: ") + startTime.toString("HH:mm");

  auto callTimeList = ma.getAdditionalCallTimes();
  if (!(callTimeList.isEmpty()))
  {
    txt += ", " + tr("additional calls at ");
    for (QDateTime call : callTimeList)
    {
      txt += call.toString("HH:mm") + ", ";
    }
    txt.chop(2);
  }
  baseline += fntMetrics.height() * (1 + ItemTextRowSkip_Perc);
  GuiHelpers::drawFormattedText(painter, x0, baseline, txt, true, false, normalFont, QColor(Qt::white));

  // draw the third info line with umpire information
  txt = tr("Umpire: ");
  REFEREE_MODE refMode = ma.get_RAW_RefereeMode();
  if (refMode == REFEREE_MODE::NONE)
  {
    txt += tr("none");
  }
  else if (refMode == REFEREE_MODE::HANDWRITTEN)
  {
    txt += tr("manually assigned");
  }
  else
  {
    upPlayer referee = ma.getAssignedReferee();
    assert(referee != nullptr);
    txt += referee->getDisplayName_FirstNameFirst();
  }
  baseline += fntMetrics.height() * (1 + ItemTextRowSkip_Perc);
  GuiHelpers::drawFormattedText(painter, x0, baseline, txt, true, false, normalFont, QColor(Qt::white));
}

//----------------------------------------------------------------------------

void CourtItemDelegate::paintSelectedCell(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index, int srcRowId) const
{
  CourtMngr cm{db};
  auto co = cm.getCourtBySeqNum(srcRowId);
  auto ma = co->getMatch();

  if (index.column() != 1)
  {
    GuiHelpers::drawFormattedText(painter, option.rect, index.data(Qt::DisplayRole).toString(),
                                  Qt::AlignVCenter|Qt::AlignCenter, true, false, normalFont, QColor(Qt::white), 1.0);
  } else {
    if (ma != nullptr)
    {
      paintMatchInfoCell_Selected(painter, option, *ma);
    } else {
      paintCourtStatus(painter, option, *co, true);
    }
  }
}

//----------------------------------------------------------------------------

void CourtItemDelegate::paintUnselectedCell(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index, int srcRowId) const
{
  CourtMngr cm{db};
  auto co = cm.getCourtBySeqNum(srcRowId);
  auto ma = co->getMatch();

  if (index.column() != 1)
  {
    painter->drawText(option.rect, Qt::AlignVCenter|Qt::AlignCenter, index.data(Qt::DisplayRole).toString());
  } else {
    if (ma != nullptr)
    {
      // calculate the position of the text baseline
      double vertMargin = (ItemRowHeight - fntMetrics.height()) / 2.0;
      double yBaseline = option.rect.y() + vertMargin + fntMetrics.ascent();

      // draw a simple, single line match info
      GuiHelpers::drawFormattedText(painter, option.rect.x() + ItemMargin, yBaseline,
                                    ma->getDisplayName("", ""));
    } else {
      paintCourtStatus(painter, option, *co, false);
    }
  }
}

//----------------------------------------------------------------------------

void CourtItemDelegate::paintCourtStatus(QPainter* painter, const QStyleOptionViewItem& option, const Court& co, bool isSelected) const
{
  QString label;
  ObjState stat = co.getState();
  bool manual = co.isManualAssignmentOnly();

  // set a default color for text items
  QColor txtCol = isSelected ? QColor(Qt::white) : QColor(Qt::darkGray);

  if ((stat == ObjState::CO_AVAIL) && !manual)
  {
    label = tr("(free)");
    txtCol = isSelected ? QColor(Qt::green) : QColor(Qt::darkGreen);
  }
  else if ((stat == ObjState::CO_AVAIL) && manual)
  {
    label = tr("(free, manual match assignment only)");
    txtCol = isSelected ? QColor(Qt::green) : QColor(Qt::darkGreen);
  }
  else if (stat == ObjState::CO_DISABLED)
  {
    label = tr("(disabled)");
    txtCol = QColor(Qt::red);
  }
  GuiHelpers::drawFormattedText(painter, option.rect, label, Qt::AlignVCenter | Qt::AlignCenter, isSelected, true, QFont(), txtCol);
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
    

