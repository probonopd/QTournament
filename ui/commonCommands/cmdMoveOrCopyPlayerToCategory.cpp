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

#include <QObject>
#include <QMessageBox>

#include "cmdMoveOrCopyPlayerToCategory.h"
#include "ui/DlgSelectPlayer.h"
#include "CatMngr.h"

cmdMoveOrCopyPlayerToCategory::cmdMoveOrCopyPlayerToCategory(QWidget* p, const Player& _pl, const Category& _srcCat, const Category& _dstCat, bool _isMove)
  :AbstractCommand(_srcCat.getDatabaseHandle(), p), pl(_pl), srcCat(_srcCat), dstCat(_dstCat), isMove(_isMove)
{

}

//----------------------------------------------------------------------------

Error cmdMoveOrCopyPlayerToCategory::exec()
{
  CatMngr cm{db};

  // check if the player is in the source category
  if (!(srcCat.hasPlayer(pl)))
  {
    QString msg = tr("The player is not assigned to the source category of this operation.");
    QMessageBox::warning(parentWidget, tr("Move or copy player"), msg);
    return Error::PlayerNotInCategory;
  }

  // if this is a move operation: make sure we can actually delete
  // the player from the source category
  if (isMove && (!(srcCat.canRemovePlayer(pl))))
  {
    QString msg = tr("The player cannot be removed from the source category of this operation.");
    QMessageBox::warning(parentWidget, tr("Move player"), msg);
    return Error::PlayerNotRemovableFromCategory;
  }

  // if this is a copy operation and the player is already in
  // the target category, there is nothing to do for us
  bool dstCatHasPlayer = dstCat.hasPlayer(pl);
  if (!isMove && dstCatHasPlayer)
  {
    return Error::OK;
  }

  // if this is a move operation and the player is already in
  // the target category, this boils down to a simple deletion
  if (isMove && dstCatHasPlayer)
  {
    Error err = cm.removePlayerFromCategory(pl, srcCat);

    if (err != Error::OK)   // shouldn't happen after the previous check, but anyway...
    {
      QString msg = tr("The player cannot be removed from the source category of this operation.");
      QMessageBox::warning(parentWidget, tr("Move player"), msg);
    }

    return err;
  }

  // try to add the player to the target category
  Error err = cm.addPlayerToCategory(pl, dstCat);
  if (err != Error::OK)
  {
    QString msg = tr("The player cannot be added to the target category of this operation.");
    QMessageBox::warning(parentWidget, tr("Move or copy player"), msg);
    return err;
  }

  // if this is a move operation, delete the player from the source
  if (isMove)
  {
    Error err = cm.removePlayerFromCategory(pl, srcCat);

    if (err != Error::OK)   // shouldn't happen after the previous check, but anyway...
    {
      QString msg = tr("The player cannot be removed from the source category of this operation.");
      QMessageBox::warning(parentWidget, tr("Move player"), msg);
    }

    return err;
  }

  return Error::OK;
}

