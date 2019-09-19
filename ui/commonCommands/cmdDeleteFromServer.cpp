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

#include "cmdDeleteFromServer.h"
#include "cmdStartOnlineSession.h"
#include "cmdUnlockKeystore.h"
#include "OnlineMngr.h"
#include "ui/DlgRegisterTournament.h"
#include "ui/DlgPassword.h"

using namespace QTournament;

cmdDeleteFromServer::cmdDeleteFromServer(QWidget* p, const TournamentDB& _db)
  :AbstractCommand(_db, p)
{

}

//----------------------------------------------------------------------------

Error cmdDeleteFromServer::exec()
{
  OnlineMngr* om = db.getOnlineManager();

  // if the user hasn't registered yet,
  // it's pointless to proceed.
  // This should be checked by the caller
  // (e.g., by menu locking/disabling), so
  // we quit without error message
  if (!(om->hasRegistrationSubmitted()))
  {
    return Error::WrongState;  // dummy value
  }

  // ask for confirmation
  QString msg = tr("<p>Do you really want to delete the tournament from the server?</p>");
  msg += tr("<p>The deletion <b>cannot be undone</b> and is effective immediately.</p>");
  int rc = QMessageBox::question(parentWidget, tr("Delete tournament from server"), msg);
  if (rc != QMessageBox::Yes) return Error::OK;

  // check whether the server is online and whether we
  // have a network connection
  //
  // this can last up to five seconds (--> timeout) and thus
  // we better enable the hourglass cursor
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  int tripTime = om->ping();
  QApplication::restoreOverrideCursor();

  // if tripTime is less than zero, an error occurred
  if (tripTime < 0)
  {
    QString msg = tr("The tournament server is currently not available or there is no working internet connection.\n\nPlease try again later.");
    QMessageBox::information(parentWidget, tr("Remove tournament from server"), msg);
    return Error::WrongState;  // dummy error code; will not be evaluated by caller
  }

  // if the secret signing key has not yet been unlocked, ask the
  // user for the password and unlock the keystore
  if (!(om->isUnlocked()))
  {
    cmdUnlockKeystore cmd{parentWidget, db};
    Error err = cmd.exec();
    if (err != Error::OK) return err;
  }

  QString errTxt;
  OnlineError err = om->deleteFromServer(errTxt);

  // handle connection / transport errors
  if ((err != OnlineError::Okay) && (err != OnlineError::TransportOkay_AppError))
  {
    QString msg;
    switch (err)
    {
    case OnlineError::Timeout:
      msg = tr("The server is currently not available.\n\n");
      msg += tr("Maybe the server is temporarily down or you are offline.");
      break;

    case OnlineError::BadRequest:
      msg = tr("The server did not accept our connection request (400, BadRequest).");
      break;

    case OnlineError::IncompatibleVersions:
      msg = tr("<p>This version of QTournament is incompatible with the server.</p>");
      msg += tr("<p>Please download a more recent version from:</p>");
      msg += "<p><a href='http://qtournament.de'>QTournament.de</a></p>";
      break;

    case OnlineError::LocalDatabaseError:
      msg = tr("<p>A local database error has occurred.</p>");
      msg += tr("<p>The tournament has been deleted on the server but the local database file is now inconsistent.</p>");
      msg += tr("<p>You can safely continue with the tournament but online functions will not work as expected anymore.</p>");
      break;

    default:
      msg = tr("Deletion failed due to an unspecified network or server error!");
    }

    QMessageBox::warning(parentWidget, tr("Connection failed"), msg);
    return Error::WrongState; // dummy value
  }

  // at this point, the data exchange with the server was successful (HTTP and Signatures).
  // We only have to check if the request on application level was successful as well.

  if (err == OnlineError::Okay)
  {
    QMessageBox::information(parentWidget, tr("Deletion successful"), tr("The tournament has been successfully deleted from the server!"));

    return Error::OK;
  }

  if (errTxt == "DatabaseError")
  {
    msg = tr("You cannot connect because of a server-side database error.\n");
    msg += tr("Please try again later!");
  }
  if (msg.isEmpty())
  {
    msg = tr("You cannot delete the tournament because of an unexpected server error.\n");
    msg += tr("Please try again later!");
  }

  QMessageBox::warning(parentWidget, tr("Connection failed"), msg);
  return Error::WrongState; // dummy value

}

