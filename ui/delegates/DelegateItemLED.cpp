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


const QMap<ObjState, QColor> DelegateItemLED::state2color = {
  {CAT_FROZEN, Qt::darkYellow},
  {CAT_IDLE, Qt::darkGreen},
  {CAT_PLAYING, Qt::green},
  {CAT_FINALIZED, Qt::darkRed},
  {PL_IDLE, Qt::green},
  {PL_PLAYING, Qt::red},
  {PL_WAIT_FOR_REGISTRATION, Qt::lightGray},
  {PL_REFEREE, Qt::blue},
  {MA_READY, Qt::green},
  {MA_BUSY, Qt::red},
  {MA_WAITING, Qt::yellow},
  {CAT_WAIT_FOR_INTERMEDIATE_SEEDING, Qt::yellow},
//  {, Qt::},
//  {, Qt::},
};

DelegateItemLED::DelegateItemLED()
{
}

void DelegateItemLED::operator ()(QPainter* painter, QRect itemArea, int margin, int size, ObjState state, QColor defaultColor)
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
