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

#include <QDebug>

#include "DelegateItemLED.h"


const QMap<OBJ_STATE, QColor> DelegateItemLED::state2color = {
  {STAT_CAT_FROZEN, Qt::darkYellow},
  {STAT_CAT_IDLE, Qt::darkGreen},
  {STAT_CAT_PLAYING, Qt::green},
  {STAT_CAT_FINALIZED, Qt::darkRed},
  {STAT_PL_IDLE, Qt::green},
  {STAT_PL_PLAYING, Qt::red},
  {STAT_PL_WAIT_FOR_REGISTRATION, Qt::lightGray},
  {STAT_PL_REFEREE, Qt::blue},
  {STAT_MA_READY, Qt::green},
  {STAT_MA_BUSY, Qt::red},
  {STAT_MA_WAITING, Qt::yellow},
  {STAT_CAT_WAIT_FOR_INTERMEDIATE_SEEDING, Qt::yellow},
//  {STAT_, Qt::},
//  {STAT_, Qt::},
};

DelegateItemLED::DelegateItemLED()
{
}

void DelegateItemLED::operator ()(QPainter* painter, QRect itemArea, int margin, int size, OBJ_STATE state, QColor defaultColor)
{
  if (state2color.keys().contains(state))
  {
    this->operator ()(painter, itemArea, margin, size, state2color[state]);
  } else {
    this->operator ()(painter, itemArea, margin, size, defaultColor);
    //qDebug() << "Applied default color to unknown state " << static_cast<int>(state);
  }
}

void DelegateItemLED::operator ()(QPainter* painter, QRect itemArea, int margin, int size, QColor fillColor)
{
  QRect indicatorBox = itemArea;
  indicatorBox.adjust(margin, 0, 0, 0);
  int topY = itemArea.center().y() - size / 2;
  indicatorBox.setTop(topY);
  indicatorBox.setWidth(size);
  indicatorBox.setHeight(size);
  painter->save();
  painter->setBrush(QBrush(fillColor));
  QPen zeroPen = QPen();
  zeroPen.setStyle(Qt::NoPen);
  painter->setPen(zeroPen);
  painter->drawEllipse(indicatorBox);
  painter->restore();

}
