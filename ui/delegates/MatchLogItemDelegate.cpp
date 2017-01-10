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


void MatchLogItemDelegate::paintSelectedCell(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index, int srcRowId) const
{
  if (index.column() == 4)
  {
    MatchMngr mm{db};
    int matchId = index.data(Qt::UserRole).toInt();
    auto ma = mm.getMatch(matchId);
    if (ma == nullptr) return;

    paintMatchInfoCell(painter, option, *ma, true);
  } else {
    GuiHelpers::drawFormattedText(painter, option.rect, index.data(Qt::DisplayRole).toString(),
                                  Qt::AlignVCenter|Qt::AlignCenter, true, false, normalFont, QColor(Qt::white), 1.0);
  }
}

//----------------------------------------------------------------------------

void MatchLogItemDelegate::paintUnselectedCell(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index, int srcRowId) const
{
  if (index.column() == 4)
  {
    MatchMngr mm{db};
    int matchId = index.data(Qt::UserRole).toInt();
    auto ma = mm.getMatch(matchId);
    if (ma == nullptr) return;

    paintMatchInfoCell(painter, option, *ma, false);
  } else {
    painter->drawText(option.rect, Qt::AlignVCenter|Qt::AlignCenter, index.data(Qt::DisplayRole).toString());
  }

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
  double textHeight = (2 + ItemTextRowSkip_Perc) * fntMetrics.height();

  // calc the vertical margin
  QRect r = option.rect;
  double vertMargin = (r.height() - textHeight) / 2.0;

  // draw the left player names
  int x0 = r.x() + ItemMargin;
  int y0 = r.y() + vertMargin;
  double baseline = y0 + fntMetrics.ascent();
  GuiHelpers::drawFormattedText(painter, x0, baseline, txtLeft1, isSelected, false, normalFont, leftColor);

  // draw the colon and the right player names
  double leftNameWidth = GuiHelpers::getFormattedTextSize(painter, txtLeft1, isSelected, false, normalFont).width();
  double colonWidth = GuiHelpers::getFormattedTextSize(painter, ":", isSelected, false, normalFont).width();
  GuiHelpers::drawFormattedText(painter, x0 + leftNameWidth + 2 * ItemMargin, baseline, ":", isSelected, false, normalFont, scoreColor);
  GuiHelpers::drawFormattedText(painter, x0 + leftNameWidth + 4 * ItemMargin + colonWidth, baseline, txtRight1, isSelected, false, normalFont, rightColor);


  // second line: draw the match score
  baseline += (1 + ItemTextRowSkip_Perc) * fntMetrics.height();
  GuiHelpers::drawFormattedText(painter, x0, baseline, txtScore, isSelected, false, normalFont, scoreColor);
}

//----------------------------------------------------------------------------
