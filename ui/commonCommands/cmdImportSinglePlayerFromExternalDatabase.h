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

#ifndef CMDIMPORTSINGLEPLAYERFROMEXTERNALDATABASE_H
#define CMDIMPORTSINGLEPLAYERFROMEXTERNALDATABASE_H

#include <QObject>

#include "AbstractCommand.h"
#include "Category.h"


class cmdImportSinglePlayerFromExternalDatabase : public QObject, AbstractCommand
{
  Q_OBJECT

public:
  cmdImportSinglePlayerFromExternalDatabase(const QTournament::TournamentDB& _db, QWidget* p, int _preselectedCatId=-1);
  virtual QTournament::ERR exec() override;
  virtual ~cmdImportSinglePlayerFromExternalDatabase() {}

protected:
  int preselectedCatId;
};

#endif // CMDIMPORTSINGLEPLAYERFROMEXTERNALDATABASE_H
