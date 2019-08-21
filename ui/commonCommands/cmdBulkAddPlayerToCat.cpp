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

#include "cmdBulkAddPlayerToCat.h"
#include "ui/DlgSelectPlayer.h"
#include "CatMngr.h"

using namespace QTournament;

cmdBulkAddPlayerToCategory::cmdBulkAddPlayerToCategory(QWidget* p, const Category& _cat)
  :AbstractCommand(_cat.getDatabaseHandle(), p), cat(_cat)
{

}

//----------------------------------------------------------------------------

Error cmdBulkAddPlayerToCategory::exec()
{
  // check if can add more players anyway
  if (!(cat.canAddPlayers()))
  {
    QString msg = tr("Can't add any more player to the category. It's either\n");
    msg += tr("full or already started.");
    QMessageBox::warning(parentWidget, tr("Add players to category"), msg);

    return Error::CategoryClosedForMorePlayers;
  }

  // show a dialog for selecting the players
  DlgSelectPlayer dlg{db, parentWidget, DlgSelectPlayer::DlgContext::AddToCategory, cat};
  if (dlg.exec() != QDialog::Accepted)
  {
    return Error::OK;
  }

  // add all selected players to the category
  CatMngr cm{db};
  for (const Player& pl : dlg.getSelectedPlayers())
  {
    Error err = cm.addPlayerToCategory(pl, cat);

    if (err != Error::OK)
    {
      QString msg = tr("Could not add player\n\n");
      msg += "     " + pl.getDisplayName_FirstNameFirst() + "\n\n";
      msg += tr("to the category.");
      QMessageBox::warning(parentWidget, tr("Add players to category"), msg);
    }
  }

  return Error::OK;
}

