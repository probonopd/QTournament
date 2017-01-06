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
#include "MatchMngr.h"
#include "ui/GuiHelpers.h"
#include "MatchLogItemDelegate.h"

MatchLogItemDelegate::MatchLogItemDelegate(TournamentDB* _db, QObject* parent)
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

void MatchLogItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  MatchMngr mm{db};
  int matchId = index.data(Qt::UserRole).toInt();
  auto ma = mm.getMatch(matchId);
  if (ma == nullptr) return;

  // Fill the cell with the selection color, if necessary
  bool isItemSelected = false;
  if(option.state & QStyle::State_Selected)
  {
    QColor bgColor = option.palette.color(QPalette::Highlight);
    painter->fillRect(option.rect, bgColor);
    isItemSelected = true;
  }

  // paint logic for the second column, the match description
  if (index.column() == 4)
  {
    paintMatchInfoCell(painter, option, *ma, isItemSelected);
  } else {
    // for any other column just draw the plain text content
    if (isItemSelected)
    {
      GuiHelpers::drawFormattedText(painter, option.rect, index.data(Qt::DisplayRole).toString(),
                                    Qt::AlignVCenter|Qt::AlignCenter, true, false, normalFont, QColor(Qt::white), 1.0);
    } else {
      painter->drawText(option.rect, Qt::AlignVCenter|Qt::AlignCenter, index.data(Qt::DisplayRole).toString());
    }
  }
}

//----------------------------------------------------------------------------

QSize MatchLogItemDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  QString txt = index.data(Qt::DisplayRole).toString();
  int width = fntMetrics.width(txt) + 2 * ITEM_MARGIN;   // this is actually a wrong value for doubles matches, but it's worst case estimate

  // this doesn't work, because option.state is not yet updated
  // to QtStyle::State_Selected when sizeHint is called for a freshly
  // selected item
  //int height = (option.state & QStyle::State_Selected) ? ITEM_ROW_HEIGHT_SELECTED : ITEM_ROW_HEIGHT;

  //int row = index.row();
  //int height = (row == selectedRow) ? ITEM_ROW_HEIGHT_SELECTED : ITEM_ROW_HEIGHT;

  return QSize(width, ITEM_ROW_HEIGHT);
}

//----------------------------------------------------------------------------

void MatchLogItemDelegate::setSelectedRow(int _selRow)
{
  selectedRow = _selRow;
}

//----------------------------------------------------------------------------

void MatchLogItemDelegate::paintMatchInfoCell(QPainter* painter, const QStyleOptionViewItem& option, const Match& ma, bool isSelected) const
{
  // get the left and the right part of the player names
  QString txtLeft1;
  QString txtLeft2;
  QString txtRight1;
  QString txtRight2;
  ma.getDisplayNameTextItems(tr("Winner"), tr("Loser"), txtLeft1, txtLeft2, txtRight1, txtRight2);
  if (!(txtLeft2.isEmpty()))
  {
    txtLeft1 += " / " + txtLeft2;
  }
  if (!(txtRight2.isEmpty()))
  {
    txtRight1 += " / " + txtRight2;
  }

  // get the score string
  auto sc = ma.getScore();
  QString txtScore;
  if (sc != nullptr)
  {
    txtScore = sc->toString();
    txtScore.replace(",", ",   ");
    txtScore.replace(":", " : ");
  } else {
    txtScore = "--";
  }

  // determine the colors for the score, the left and the right string
  QColor scoreColor(isSelected ? Qt::white : Qt::black);
  QColor leftColor(isSelected ? Qt::white : Qt::black);   // default: black / white
  QColor rightColor(isSelected ? Qt::white : Qt::black);
  if (ma.getState() == QTournament::OBJ_STATE::STAT_MA_FINISHED)
  {
    auto w = ma.getWinner();
    auto l = ma.getLoser();
    auto ppLeft = ma.getPlayerPair1();

    if ((w != nullptr) && (l != nullptr))
    {
      if (*w == ppLeft)
      {
        leftColor = isSelected ? QColor{Qt::green} : QColor{Qt::darkGreen};
        rightColor = isSelected ? QColor{255,80,80} : QColor{Qt::red};
      } else {
        leftColor = isSelected ? QColor{255,80,80} : QColor{Qt::red};
        rightColor = isSelected ? QColor{Qt::green} : QColor{Qt::darkGreen};
      }
    }
  }

  // now estimate the height of the two text rows
  double textHeight = (2 + ITEM_TEXT_ROW_SKIP_PERC) * fntMetrics.height();

  // calc the vertical margin
  QRect r = option.rect;
  double vertMargin = (r.height() - textHeight) / 2.0;

  // draw the left player names
  int x0 = r.x() + ITEM_MARGIN;
  int y0 = r.y() + vertMargin;
  double baseline = y0 + fntMetrics.ascent();
  GuiHelpers::drawFormattedText(painter, x0, baseline, txtLeft1, isSelected, false, normalFont, leftColor);

  // draw the colon and the right player names
  double leftNameWidth = GuiHelpers::getFormattedTextSize(painter, txtLeft1, isSelected, false, normalFont).width();
  double colonWidth = GuiHelpers::getFormattedTextSize(painter, ":", isSelected, false, normalFont).width();
  GuiHelpers::drawFormattedText(painter, x0 + leftNameWidth + 2 * ITEM_MARGIN, baseline, ":", isSelected, false, normalFont, scoreColor);
  GuiHelpers::drawFormattedText(painter, x0 + leftNameWidth + 4 * ITEM_MARGIN + colonWidth, baseline, txtRight1, isSelected, false, normalFont, rightColor);


  // second line: draw the match score
  baseline += (1 + ITEM_TEXT_ROW_SKIP_PERC) * fntMetrics.height();
  GuiHelpers::drawFormattedText(painter, x0, baseline, txtScore, isSelected, false, normalFont, scoreColor);
}

//----------------------------------------------------------------------------
