/*
 *    This is QTournament, a badminton tournament management program.
 *    Copyright (C) 2014 - 2019  Volker Knollmann
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
#include <QResource>

#include <Sloppy/Crypto/Crypto.h>
#include <SqliteOverlay/KeyValueTab.h>

#include "OnlineMngr.h"
#include "cmdConnectionSettings.h"
#include "../DlgConnectionSettings.h"

using namespace QTournament;

cmdConnectionSetting::cmdConnectionSetting(QWidget* p, const TournamentDB& _db)
  :AbstractCommand(_db, p)
{

}

//----------------------------------------------------------------------------

Error cmdConnectionSetting::exec()
{
  OnlineMngr* om = db.getOnlineManager();
  SyncState sy = om->getSyncState();
  if (sy.hasSession())
  {
    QString msg = tr("Please disconnect before changing the server settings!");
    QMessageBox::information(parentWidget, tr("Edit Connection Settings"), msg);
    return Error::WrongState; // Dummy
  }

  QString customUrlBefore = om->getCustomUrl();
  QString customKeyBefore = om->getCustomServerKey();
  int customTimeoutBefore = om->getCustomTimeout_ms();
  DlgConnectionSettings dlg{parentWidget, customUrlBefore, customKeyBefore, customTimeoutBefore};
  int rc = dlg.exec();
  if (rc != QDialog::Accepted) return Error::WrongState;  // Dummy

  QString customUrlAfter = dlg.getCustomUrl();
  QString customKeyAfter = dlg.getCustomPubKey();
  int customTimeoutAfter = dlg.getCustomTimeout_ms();

  try
  {
    // start a transaction
    auto trans = db.startTransaction();

    // update the URL, if necessary
    if (customUrlAfter != customUrlBefore)
    {
      bool isOkay = om->setCustomUrl(customUrlAfter);

      if (!isOkay) {
        QString msg = tr("The server URL could not be updated.");
        msg += "\n\n";
        msg += tr("All connection settings remain untouched!");
        QMessageBox::warning(parentWidget, tr("Edit Connection Settings"), msg);
        return Error::WrongState; // Dummy
      }
    }

    // update the key, if necessary
    if (customKeyAfter != customKeyBefore)
    {
      bool isOkay = om->setCustomServerKey(customKeyAfter);

      if (!isOkay) {
        QString msg = tr("The server's public key could not be updated. Maybe your input was malformed?");
        msg += "\n\n";
        msg += tr("All connection settings remain untouched!");
        QMessageBox::warning(parentWidget, tr("Edit Connection Settings"), msg);
        return Error::WrongState; // Dummy
      }
    }

    // update the timeout, if necessary
    if (customTimeoutAfter != customTimeoutBefore)
    {
      bool isOkay = om->setCustomTimeout_ms(customTimeoutAfter);

      if (!isOkay) {
        QString msg = tr("The timeout setting could not be updated.");
        msg += "\n\n";
        msg += tr("All connection settings remain untouched!");
        QMessageBox::warning(parentWidget, tr("Edit Connection Settings"), msg);
        return Error::WrongState; // Dummy
      }
    }

    // commit all changes
    trans.commit();
  }
  catch (...)
  {
    QString msg = tr("A local database error occured and thus settings can't be applied!");
    QMessageBox::warning(parentWidget, tr("Edit Connection Settings"), msg);
    return Error::WrongState; // Dummy
  }

  // apply all potential changes
  om->applyCustomServerSettings();
  return Error::OK;
}

