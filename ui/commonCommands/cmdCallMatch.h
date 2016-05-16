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

#ifndef CMDCALLMATCH_H
#define CMDCALLMATCH_H

#include <QObject>

#include "AbstractCommand.h"
#include "Match.h"
#include "Court.h"

using namespace QTournament;

class cmdCallMatch : public QObject, AbstractCommand
{
  Q_OBJECT

public:
  cmdCallMatch(QWidget* p, const Match& _ma, const Court& _co);
  virtual ERR exec() override;
  virtual ~cmdCallMatch() {}

protected:
  Match ma;
  Court co;
};

#endif // CMDCALLMATCH_H
