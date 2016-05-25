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

#ifndef CMDBULKADDPLAYERTOCAT_H
#define CMDBULKADDPLAYERTOCAT_H

#include <QObject>

#include "AbstractCommand.h"
#include "Category.h"

using namespace QTournament;

class cmdBulkAddPlayerToCategory : public QObject, AbstractCommand
{
  Q_OBJECT

public:
  cmdBulkAddPlayerToCategory(QWidget* p, const Category& _cat);
  virtual ERR exec() override;
  virtual ~cmdBulkAddPlayerToCategory() {}

protected:
  Category cat;
};

#endif // CMDREGISTERPLAYER_H
