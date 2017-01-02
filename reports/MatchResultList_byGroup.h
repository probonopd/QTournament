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

#ifndef MATCHRESULTLISTBYGROUP_H
#define MATCHRESULTLISTBYGROUP_H

#include <functional>

#include <QObject>

#include "reports/AbstractReport.h"
#include "TournamentDB.h"
#include "TournamentDataDefs.h"

using namespace SqliteOverlay;

namespace QTournament
{
  class MatchResultList_ByGroup : public QObject, public AbstractReport
  {
    Q_OBJECT

  public:
    MatchResultList_ByGroup(TournamentDB* _db, const QString& _name, const Category& _cat, int _grpNum);

    virtual upSimpleReport regenerateReport() override;
    virtual QStringList getReportLocators() const override;

  private:
    Category cat;
    int grpNum;
  };

}
#endif // MATCHRESULTLISTBYGROUP_H
