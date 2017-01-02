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

#include "cmdRegisterPlayer.h"
#include "PlayerMngr.h"

cmdRegisterPlayer::cmdRegisterPlayer(QWidget* p, const Player& _pl)
  :AbstractCommand(_pl.getDatabaseHandle(), p), pl(_pl)
{

}

//----------------------------------------------------------------------------

ERR cmdRegisterPlayer::exec()
{
  ERR err;
  PlayerMngr pm{db};

  err = pm.setWaitForRegistration(pl, false);

  if (err != OK)   // this shouldn't happen
  {
    QString msg = tr("Something went wrong during player registration. This shouldn't happen.\n\n");
    msg += tr("For the records: error code = ") + QString::number(static_cast<int> (err));
    QMessageBox::warning(parentWidget, tr("WTF??"), msg);
  }

  return err;
}

