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
constexpr int RefereeSelectionDelegate::WINNER_TAG;
constexpr int RefereeSelectionDelegate::LOSER_TAG;
constexpr int RefereeSelectionDelegate::NEUTRAL_TAG;

RefereeSelectionDelegate::RefereeSelectionDelegate(TournamentDB* _db, QObject* parent)
  : QStyledItemDelegate(parent), db(_db), normalFont(QFont()),
    fntMetrics(QFontMetricsF(normalFont)),
    fntMetrics_Large(fntMetrics), // this a dummy value only
    selectedRow{-1}
{
  largeFont = QFont();
  largeFont.setPointSizeF(largeFont.pointSizeF() * LARGE_TEXT_SIZE_FAC);
  fntMetrics_Large = QFontMetricsF(largeFont);
}

//----------------------------------------------------------------------------

void RefereeSelectionDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  PlayerMngr pm{db};
  int playerId = index.data(Qt::UserRole).toInt();
  auto p = pm.getPlayer_up(playerId);
  if (p == nullptr) return;

  OBJ_STATE plStat = p->getState();

  // Fill the cell with the selection color, if necessary
  // or with the color that indicates the player state
  int col = index.column();
  bool isItemSelected = (option.state & QStyle::State_Selected);
  if (isItemSelected || (col == 0))
  {
    QColor bgColor = option.palette.color(QPalette::Highlight);

    if (col == 0)
    {
      if (DelegateItemLED::state2color.contains(plStat))
      {
        bgColor = DelegateItemLED::state2color[plStat];
      }
    }

    painter->fillRect(option.rect, bgColor);
  }

  //
  // determine the font color
  //

  // default: set to white if we're selected
  QColor fntColor{0,0,0};
  if (isItemSelected)
  {
    fntColor = QColor(Qt::white);
  }

  // override with red / green in the name column if
  // a winner / loser tag is set
  if (col == 1)
  {
    int tag = index.data(Qt::UserRole + 1).toInt();

    if (tag == WINNER_TAG)
    {
      fntColor = isItemSelected ? QColor{Qt::green} : QColor{Qt::darkGreen};
    }

    if (tag == LOSER_TAG)
    {
      fntColor = isItemSelected ? QColor{255,80,80} : QColor{Qt::red};
    }
  }
  painter->setPen(QPen(fntColor));

  // determine the font weight
  QFont fnt;
  fnt.setBold(isItemSelected);
  painter->setFont(fnt);

  // determine the horizontal alignment and margins
  int horAlign = Qt::AlignCenter;
  QRect contentRect = option.rect;
  if ((col == 1) || (col == 2)) // name or team
  {
    horAlign = Qt::AlignLeft;
    contentRect.adjust(ITEM_MARGIN, 0, 0, 0);
  }

  // actually draw the text
  painter->drawText(contentRect, Qt::AlignVCenter | horAlign, index.data(Qt::DisplayRole).toString());
}

//----------------------------------------------------------------------------

QSize RefereeSelectionDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  QString txt = index.data(Qt::DisplayRole).toString();
  int width = fntMetrics.width(txt) + 2 * ITEM_MARGIN;

  // this doesn't work, because option.state is not yet updated
  // to QtStyle::State_Selected when sizeHint is called for a freshly
  // selected item
  //int height = (option.state & QStyle::State_Selected) ? ITEM_ROW_HEIGHT_SELECTED : ITEM_ROW_HEIGHT;

  //int row = index.row();
  //int height = (row == selectedRow) ? ITEM_ROW_HEIGHT_SELECTED : ITEM_ROW_HEIGHT;

  return QSize(width, ITEM_ROW_HEIGHT);
}

//----------------------------------------------------------------------------

/*void RefereeSelectionDelegate::setSelectedRow(int _selRow)
{
  selectedRow = _selRow;
}*/

//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
