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

#ifndef MATCHRESULTLIST_H
#define MATCHRESULTLIST_H

#include <functional>

#include <QObject>

#include "reports/AbstractReport.h"
#include "TournamentDB.h"
#include "TournamentDataDefs.h"

using namespace SqliteOverlay;

namespace QTournament
{
  class MatchResultList : public QObject, public AbstractReport
  {
    Q_OBJECT

  public:
    MatchResultList(TournamentDB* _db, const QString& _name, const Category& _cat, int _round);

    virtual upSimpleReport regenerateReport() override;
    virtual QStringList getReportLocators() const override;

  private:
    Category cat;
    int round;
  };

}
#endif // MATCHRESULTLIST_H
