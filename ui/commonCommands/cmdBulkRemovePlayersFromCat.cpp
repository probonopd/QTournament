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

#include "cmdBulkRemovePlayersFromCat.h"
#include "ui/DlgSelectPlayer.h"
#include "CatMngr.h"

using namespace QTournament;

cmdBulkRemovePlayersFromCategory::cmdBulkRemovePlayersFromCategory(QWidget* p, const Category& _cat)
  :AbstractCommand(_cat.getDatabaseHandle(), p), cat(_cat)
{

}

//----------------------------------------------------------------------------

Error cmdBulkRemovePlayersFromCategory::exec()
{
  // show a dialog for selecting the players
  DlgSelectPlayer dlg{db, parentWidget, DlgSelectPlayer::DlgContext::RemoveFromCategory, cat};
  if (dlg.exec() != QDialog::Accepted)
  {
    return Error::OK;
  }

  // remove all selected players from the category
  CatMngr cm{db};

  for (const Player& pl : dlg.getSelectedPlayers())
  {
    Error err = cm.removePlayerFromCategory(pl, cat);

    if (err != Error::OK)
    {
      QString msg = tr("Could not remove player\n\n");
      msg += "     " + pl.getDisplayName_FirstNameFirst() + "\n\n";
      msg += tr("from the category.");
      QMessageBox::warning(parentWidget, tr("Remove players from category"), msg);
    }
  }

  return Error::OK;
}

