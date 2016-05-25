/*
 *    This is QTournament, a badminton tournament management program.
 *    Copyright (C) 2014 - 2016  Volker Knollmann
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

#ifndef PLOTSTANDINGS_H
#define PLOTSTANDINGS_H

#include <QObject>

#include "AbstractReportElement.h"
#include "RankingMngr.h"
#include "RankingEntry.h"

using namespace QTournament;

class plotStandings : public QObject, AbstractReportElement
{
  Q_OBJECT

public:
  plotStandings(SimpleReportGenerator* _rep, const RankingEntryList& _rel, const QString& tabName);
  virtual QRectF plot(const QPointF& topLeft = QPointF(-1, -1));
  virtual ~plotStandings() {}

protected:
  RankingEntryList rel;
  QString tableName;
};

#endif // PLOTSTANDINGS_H
