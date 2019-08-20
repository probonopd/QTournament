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

#ifndef CMDMOVEORCOPYPAIRTOCONFIGH
#define CMDMOVEORCOPYPAIRTOCONFIGH

#include <QObject>

#include "AbstractCommand.h"
#include "Category.h"
#include "PlayerPair.h"


class cmdMoveOrCopyPairToCategory : public QObject, AbstractCommand
{
  Q_OBJECT

public:
  cmdMoveOrCopyPairToCategory(QWidget* p, const QTournament::PlayerPair& _pp,
                              const QTournament::Category& _srcCat, const QTournament::Category& _dstCat,
                              bool _isMove=false);
  virtual QTournament::Error exec() override;
  virtual ~cmdMoveOrCopyPairToCategory() {}

protected:
  QTournament::PlayerPair pp;
  QTournament::Category srcCat;
  QTournament::Category dstCat;
  bool isMove;
};

#endif // CMDREGISTERPLAYER_H
