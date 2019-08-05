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

#ifndef CMDUNREGISTERPLAYER_H
#define CMDUNREGISTERPLAYER_H

#include <QObject>

#include "AbstractCommand.h"
#include "Player.h"


class cmdUnregisterPlayer : public QObject, AbstractCommand
{
  Q_OBJECT

public:
  cmdUnregisterPlayer(QWidget* p, const QTournament::Player& _pl);
  virtual QTournament::ERR exec() override;
  virtual ~cmdUnregisterPlayer() {}

protected:
  QTournament::Player pl;
};

#endif // CMDREGISTERPLAYER_H
