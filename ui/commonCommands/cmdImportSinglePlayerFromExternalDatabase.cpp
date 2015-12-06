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

#include <QObject>
#include <QMessageBox>

#include "Tournament.h"
#include "cmdImportSinglePlayerFromExternalDatabase.h"
#include "ui/DlgImportPlayer.h"
#include "ui/DlgPickPlayerSex.h"
#include "ui/dlgEditPlayer.h"
#include "cmdCreatePlayerFromDialog.h"

cmdImportSinglePlayerFromExternalDatabase::cmdImportSinglePlayerFromExternalDatabase(QWidget* p, int _preselectedCatId)
  :AbstractCommand(p), preselectedCatId(_preselectedCatId)
{

}

//----------------------------------------------------------------------------

ERR cmdImportSinglePlayerFromExternalDatabase::exec()
{
  // make sure we have an external database open
  auto tnmt = Tournament::getActiveTournament();
  PlayerMngr* pm = tnmt->getPlayerMngr();
  if (!(pm->hasExternalPlayerDatabaseOpen()))
  {
    QString msg = tr("No valid database for player import open.");
    QMessageBox::warning(parentWidget, tr("Import player"), msg);
    return EPD__NOT_OPENED;
  }

  ExternalPlayerDB* extDb = pm->getExternalPlayerDatabaseHandle();

  // show a search-and-select dialog
  DlgImportPlayer dlg{parentWidget, extDb};
  if (dlg.exec() != QDialog::Accepted)
  {
    return OK;
  }

  // get the selected ID
  int extId = dlg.getSelectedExternalPlayerId();
  if (extId < 0)
  {
    QString msg = tr("No valid player selection found");
    QMessageBox::warning(parentWidget, tr("Import player"), msg);
    return INVALID_ID;
  }

  // get the selected player from the database
  auto extPlayer = extDb->getPlayer(extId);
  if (extPlayer == nullptr)
  {
    QString msg = tr("No valid player selection found");
    QMessageBox::warning(parentWidget, tr("Import player"), msg);
    return INVALID_ID;
  }

  // if the player has no valid sex assigned,
  // show a selection dialog
  upExternalPlayerDatabaseEntry finalPlayerData;
  if (extPlayer->getSex() == DONT_CARE)
  {
    DlgPickPlayerSex dlgPickSex{parentWidget, extPlayer->getFirstname() + " " + extPlayer->getLastname()};
    if (dlgPickSex.exec() != QDialog::Accepted)
    {
      return OK;
    }

    finalPlayerData = make_unique<ExternalPlayerDatabaseEntry>(
          extPlayer->getFirstname(),
          extPlayer->getLastname(),
          dlgPickSex.getSelectedSex()
          );
  } else {
    finalPlayerData = std::move(extPlayer);
  }

  // make sure the player's sex fits to a possibly preselected
  // category
  if (preselectedCatId > 0)
  {
    CatMngr* cm = tnmt->getCatMngr();
    auto cat = cm->getCategory(preselectedCatId);

    // was the provided ID valid? If not, invalidate it
    if (cat == nullptr) preselectedCatId = -1;

    // may we add a player of the selected sex to this category?
    SEX selSex = finalPlayerData->getSex();
    if (cat->getAddState(selSex) != CAN_JOIN)
    {
      QString msg = tr("%1 cannot be added to this category.");
      msg = msg.arg((selSex == M) ? tr("A male player") : tr("A female player"));
      QMessageBox::warning(parentWidget, tr("Import player"), msg);
      return INVALID_SEX;
    }
  }


  // now we have all player details, so we can prepare
  // the insert-player-dialog to allow the user the
  // selection of a team and possible category assignments
  DlgEditPlayer dlgCreate{parentWidget, *finalPlayerData, preselectedCatId};

  // let an external command do the rest of the work
  cmdCreatePlayerFromDialog cmd{parentWidget, &dlgCreate};
  return cmd.exec();
}

