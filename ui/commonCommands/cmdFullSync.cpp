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
#include <QResource>

#include <Sloppy/Crypto/Crypto.h>
#include <SqliteOverlay/KeyValueTab.h>

#include "OnlineMngr.h"
#include "cmdFullSync.h"

using namespace QTournament;

cmdFullSync::cmdFullSync(QWidget* p, const TournamentDB& _db)
  :AbstractCommand(_db, p)
{

}

//----------------------------------------------------------------------------

Error cmdFullSync::exec()
{
  OnlineMngr* om = db.getOnlineManager();

  QString errTxt;
  OnlineError err = om->doFullSync(errTxt);

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

    default:
      msg = tr("Session setup failed due to an unspecified network or server error!");
    }

    QMessageBox::warning(parentWidget, tr("Full sync failed"), msg);
    return Error::WrongState; // dummy value
  }

  // at this point, the data exchange with the server was successful (HTTP and Signatures).
  // We only have to check if the request on application level was successful as well.

  QString msg;
  if (errTxt == "DatabaseError")
  {
    msg = tr("Syncing failed because of a server-side database error.\n");
    msg += tr("Please try again later!");
  }
  if (errTxt == "CSVError")
  {
    msg = tr("Syncing failed because the server couldn't digest our CSV data!\n");
    msg += tr("Strange, this shouldn't happen...");
  }
  if (msg.isEmpty() && (err != OnlineError::Okay))
  {
    msg = tr("You cannot connect because of an unexpected server error.\n");
    msg += tr("Please try again later!");
  }
  if (!(msg.isEmpty()))
  {
    QMessageBox::warning(parentWidget, tr("Full sync failed"), msg);
    return Error::WrongState; // dummy value
  }

  QMessageBox::information(parentWidget, tr("Full sync successful"),
                           tr("The server is now in sync with your local tournament file!"));
  return Error::OK;
}

