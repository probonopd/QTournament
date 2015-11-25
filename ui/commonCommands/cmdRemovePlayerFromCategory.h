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

#ifndef CMDREMOVE_PLAYER_FROM_CAT__H
#define CMDREMOVE_PLAYER_FROM_CAT__H

#include <QObject>

#include "AbstractCommand.h"
#include "Player.h"
#include "Category.h"

using namespace QTournament;

class cmdRemovePlayerFromCategory : public QObject, AbstractCommand
{
  Q_OBJECT

public:
  cmdRemovePlayerFromCategory(QWidget* p, const Player& _pl, const Category& _cat);
  virtual ERR exec() override;
  virtual ~cmdRemovePlayerFromCategory() {}

protected:
  Player pl;
  Category cat;
};

#endif // CMDREGISTERPLAYER_H
