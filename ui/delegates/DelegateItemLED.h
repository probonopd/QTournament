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

#ifndef DELEGATEITEMLED_H
#define DELEGATEITEMLED_H

#include <QPainter>
#include <QStyledItemDelegate>
#include <QMap>
#include "TournamentDataDefs.h"

/**
 * @brief A functor for painting a "LED indicator" (a circle with a status color) in a delegate's paint area
 */


class DelegateItemLED
{
public:
  DelegateItemLED();

  void operator()(QPainter* painter, QRect itemArea, int margin, int size, QColor fillColor);
  void operator()(QPainter* painter, QRect itemArea, int margin, int size, QTournament::ObjState state, QColor defaultColor=Qt::white);

  const static QMap<QTournament::ObjState, QColor> state2color;
};

#endif // DELEGATEITEMLED_H
