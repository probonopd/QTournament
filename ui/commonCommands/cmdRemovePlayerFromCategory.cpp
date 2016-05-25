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

#include <QObject>
#include <QMessageBox>

#include "cmdRemovePlayerFromCategory.h"
#include "CatMngr.h"

cmdRemovePlayerFromCategory::cmdRemovePlayerFromCategory(QWidget* p, const Player& _pl, const Category& _cat)
  :AbstractCommand(_pl.getDatabaseHandle(), p), pl(_pl), cat(_cat)
{

}

//----------------------------------------------------------------------------

ERR cmdRemovePlayerFromCategory::exec()
{
  ERR err;
  CatMngr cm{db};

  err = cm.removePlayerFromCategory(pl, cat);

  if (err == OK) return OK;

  QString msg;
  if (err == PLAYER_NOT_REMOVABLE_FROM_CATEGORY)
  {
    msg = tr("The player cannot be removed from the category anymore,\n");
    msg += tr("e.g., because the category has already been started");
  }
  else   // unspecific error
  {
    msg = tr("Something went wrong when removing the player from the category. This shouldn't happen.\n\n");
    msg += tr("For the records: error code = ") + QString::number(static_cast<int> (err));
  }

  QMessageBox::warning(parentWidget, tr("Remove player from category"), msg);

  return err;
}
