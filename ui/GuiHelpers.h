/*
 *    This is QTournament, a badminton tournament management program.
 *    Copyright (C) 2014 - 2015  Volker Knollmann
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

#ifndef GUIHELPERS_H
#define GUIHELPERS_H

#include <QObject>
#include <QString>
#include <QtGlobal>
#include <QPainter>
#include <QFont>
#include <QColor>

#include "Match.h"
#include "Court.h"

class GuiHelpers : public QObject
{
  Q_OBJECT

public:
  GuiHelpers();
  ~GuiHelpers();
  static QString groupNumToString(int grpNum);
  static QString groupNumToLongString(int grpNum);
  static QString prepCall(const QTournament::Match& ma, const QTournament::Court& co, int nCall=0);

  static void drawFormattedText(QPainter* painter, QRect r, const QString& s, int alignmentFlags=Qt::AlignVCenter|Qt::AlignLeft,
                                bool isBold=false, bool isItalics=false, QFont fnt=QFont(), QColor fntColor = QColor(0,0,0),
                                double fntSizeFac = 1.0);
  static void drawFormattedText(QPainter* painter, QRect r, const QString& s, int alignmentFlags=Qt::AlignVCenter|Qt::AlignLeft,
                                bool isBold=false, bool isItalics=false, double fntSizeFac = 1.0);
  static void execWalkover(QWidget* parent, const QTournament::Match& ma, int playerNum);
};

#endif // GUIHELPERS_H
