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
#include <QApplication>
#include <QString>

#include <SqliteOverlay/KeyValueTab.h>

#include "cmdSetOrChangePassword.h"
#include "OnlineMngr.h"
#include "ui/DlgPassword.h"

cmdSetOrChangePassword::cmdSetOrChangePassword(QWidget* p, TournamentDB* _db)
  :AbstractCommand(_db, p)
{

}

//----------------------------------------------------------------------------

ERR cmdSetOrChangePassword::exec()
{
  OnlineMngr* om = db->getOnlineManager();
  bool hasPw = om->hasSecretInDatabase();

  OnlineError oe;
  if (hasPw)
  {
    DlgPassword dlg{parentWidget, DlgPassword::DlgMode_ChangePassword};
    int rc = dlg.exec();
    if (rc != QDialog::Accepted) return ERR::WrongState;  // dummy return value

    oe = om->setPassword(dlg.getNewPassword(), dlg.getCurrentPassword());

  } else {

    DlgPassword dlg{parentWidget, DlgPassword::DlgMode_SetNewPassword};
    int rc = dlg.exec();
    if (rc != QDialog::Accepted) return ERR::WrongState;  // dummy return value;

    oe = om->setPassword(dlg.getNewPassword());
  }

  if (oe != OnlineError::Okay)
  {
    QString msg{tr("An error occurred an the password could not be stored.")};
    QMessageBox::warning(parentWidget, tr("Set password"), msg);
    return ERR::WrongState;  // dummy return value;
  }

  QString msg{tr("The password has been set successfully!")};
  QMessageBox::information(parentWidget, tr("Set password"), msg);

  return ERR::OK;
}

