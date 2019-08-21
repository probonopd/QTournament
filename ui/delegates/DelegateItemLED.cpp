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

using namespace QTournament;

const QMap<ObjState, QColor> DelegateItemLED::state2color = {
  {ObjState::CAT_Frozen, Qt::darkYellow},
  {ObjState::CAT_Idle, Qt::darkGreen},
  {ObjState::CAT_Playing, Qt::green},
  {ObjState::CAT_Finalized, Qt::darkRed},
  {ObjState::PL_Idle, Qt::green},
  {ObjState::PL_Playing, Qt::red},
  {ObjState::PL_WaitForRegistration, Qt::lightGray},
  {ObjState::PL_Referee, Qt::blue},
  {ObjState::MA_Ready, Qt::green},
  {ObjState::MA_Busy, Qt::red},
  {ObjState::MA_Waiting, Qt::yellow},
  {ObjState::CAT_WaitForIntermediateSeeding, Qt::yellow},
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
