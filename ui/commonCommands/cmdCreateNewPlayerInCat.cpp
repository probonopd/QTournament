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

#include "cmdCreateNewPlayerInCat.h"
#include "ui/dlgEditPlayer.h"
#include "ui/DlgPickPlayerSex.h"
#include "cmdCreatePlayerFromDialog.h"
#include "CatMngr.h"

cmdCreateNewPlayerInCat::cmdCreateNewPlayerInCat(QWidget* p, const Category& _cat)
  :AbstractCommand(_cat.getDatabaseHandle(), p), cat(_cat)
{

}

//----------------------------------------------------------------------------

ERR cmdCreateNewPlayerInCat::exec()
{
  // check if can add more players anyway
  if (!(cat.canAddPlayers()))
  {
    QString msg = tr("Can't add any more player to the category. It's either\n");
    msg += tr("full or already started.");
    QMessageBox::warning(parentWidget, tr("Create new player in category"), msg);

    return CATEGORY_CLOSED_FOR_MORE_PLAYERS;
  }

  // show a dialog for selecting the new player's sex
  DlgPickPlayerSex dlgSex{parentWidget, QString()};
  if (dlgSex.exec() != QDialog::Accepted)
  {
    return ERR::OK;
  }
  SEX selectedSex = dlgSex.getSelectedSex();

  // check if we can add a player of the selected
  // sex to the category
  if (cat.getAddState(selectedSex) != CatAddState::CanJoin)
  {
    QString msg = tr("Can't add a %1 to the category.");
    msg = msg.arg((selectedSex == M) ? tr("male player") : tr("female player"));
    QMessageBox::warning(parentWidget, tr("Create new player in category"), msg);

    return ERR::INVALID_SEX;
  }

  // prepare a dialog for creating a new player
  DlgEditPlayer dlgCreate{db, parentWidget, selectedSex, cat};

  // let an external command do the rest of the work
  cmdCreatePlayerFromDialog cmd{db, parentWidget, &dlgCreate};
  return cmd.exec();
}

