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
#include <QInputDialog>

#include "cmdUnlockKeystore.h"
#include "OnlineMngr.h"

cmdUnlockKeystore::cmdUnlockKeystore(QWidget* p, TournamentDB* _db)
  :AbstractCommand(_db, p)
{

}

//----------------------------------------------------------------------------

ERR cmdUnlockKeystore::exec()
{
  OnlineMngr* om = db->getOnlineManager();

  // if the user hasn't supplied a password yet,
  // there's nothing we can unlock
  if (!(om->hasSecretInDatabase()))
  {
    return ERR::WrongState;  // dummy value
  }

  // if the keystore is already unlocked,
  // we're done
  if (om->isUnlocked())
  {
    return ERR::OK;
  }

  // ask for a password and try to unlock the keystore
  while (true)
  {
    QString pw;

    QInputDialog pwDlg{parentWidget};
    pwDlg.setInputMode(QInputDialog::TextInput);
    pwDlg.setTextEchoMode(QLineEdit::Password);
    pwDlg.setLabelText(tr("Please enter your tournament password:"));
    int rc = pwDlg.exec();
    if (rc != QDialog::Accepted) return ERR::WrongState; // dummy
    pw = pwDlg.textValue().trimmed();

    if (pw.isEmpty())
    {
      QMessageBox::warning(parentWidget, tr("Enter password"),
                           tr("Please enter the password!"));
      continue;
    }

    QTournament::OnlineError oe = om->unlockKeystore(pw);
    if (oe != OnlineError::Okay)
    {
      QMessageBox::warning(parentWidget, tr("Enter password"),
                           tr("The password was not correct!"));
      pw.clear();
      continue;
    }

    return ERR::OK;
  }
}

