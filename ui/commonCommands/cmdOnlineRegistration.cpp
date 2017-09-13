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

#include "cmdOnlineRegistration.h"
#include "cmdSetOrChangePassword.h"
#include "cmdUnlockKeystore.h"
#include "OnlineMngr.h"
#include "ui/DlgRegisterTournament.h"
#include "ui/DlgPassword.h"

cmdOnlineRegistration::cmdOnlineRegistration(QWidget* p, TournamentDB* _db)
  :AbstractCommand(_db, p)
{

}

//----------------------------------------------------------------------------

ERR cmdOnlineRegistration::exec()
{
  OnlineMngr* om = db->getOnlineManager();

  // if the user hasn't supplied a password yet,
  // do that now
  if (!(om->hasSecretInDatabase()))
  {
    cmdSetOrChangePassword cmd{parentWidget, db};
    ERR e = cmd.exec();
    if (e != ERR::OK) return ERR::WRONG_STATE;  // dummy value
  }

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
    QMessageBox::information(parentWidget, tr("Online registration"), msg);
    return ERR::WRONG_STATE;  // dummy error code; will not be evaluated by caller
  }

  // if the secret signing key has not yet been unlocked, ask the
  // user for the password and unlock the keystore
  if (!(om->isUnlocked()))
  {
    cmdUnlockKeystore cmd{parentWidget, db};
    ERR err = cmd.exec();
    if (err != ERR::OK) return err;
  }

  // show the registration form
  auto cfg = SqliteOverlay::KeyValueTab::getTab(db, TAB_CFG, false);
  string tName = cfg->operator [](CFG_KEY_TNMT_NAME);
  string club = cfg->operator [](CFG_KEY_TNMT_ORGA);
  DlgRegisterTournament dlg{parentWidget, QString::fromUtf8(tName.c_str()), QString::fromUtf8(club.c_str())};
  int rc = dlg.exec();
  if (rc != QDialog::Accepted) return ERR::WRONG_STATE;  // dummy error code; will not be evaluated by caller

  // show a consent form
  QResource binData(":/ui/consent.html");
  QString consentTxt = QString::fromUtf8((const char*)binData.data());
  QMessageBox dlgConsent{parentWidget};
  dlgConsent.setText(consentTxt);
  dlgConsent.setStandardButtons(QMessageBox::No | QMessageBox::Yes);
  dlgConsent.setDefaultButton(QMessageBox::Yes);
  dlgConsent.setWindowTitle(tr("Please read carefully"));
  rc = dlgConsent.exec();
  if (rc != QMessageBox::Yes) return ERR::WRONG_STATE;  // dummy error code; will not be evaluated by caller

  QString errTxt;
  OnlineError err = om->registerTournament(dlg.getValidatedRegistrationData(), errTxt);

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
      msg = tr("The server did not accept our registration request (400, BadRequest).");
      break;

    default:
      msg = tr("Registration failed due to an unspecified network or server error!");
    }

    QMessageBox::warning(parentWidget, tr("Online Registration FAILED"), msg);
    return ERR::WRONG_STATE; // dummy value
  }

  // at this point, the data exchange with the server was successful (HTTP and Signatures).
  // We only have to check if the request on application level was successful as well.
  QString msg;
  if (err == OnlineError::Okay)
  {
    msg = tr("Online registration successful!\n\n");
    msg += tr("Within the next minutes you should receive a confirmation email. ");
    msg += tr("Please click on the link the in email to complete your registration.");
    QMessageBox::information(parentWidget, tr("Online registration"), msg);

    return ERR::OK;
  }

  //
  // an application level error occurred
  //

  if (errTxt == "NameExists")
  {
    msg = tr("A tournament with this name already exists.\n\n");
    msg += tr("Maybe you have already successfully registered your tournament before or ");
    msg += tr("someone else is already using your tournament name.\n\n");
    msg += tr("Please note that you can only register ONCE with a tournament file, ");
    msg += tr("regardless of the tournament name. Attempting to register again after ");
    msg += tr("an already successful prior registration will always fail.");
  }
  if (errTxt == "DatabaseError")
  {
    msg = tr("Your registration failed due to a database error on the server.\n\n");
    msg += tr("You may try to register again but if the problems persist please send an email to\n\n");
    msg += tr("\tqtournament@vkserv.de");
  }
  if (errTxt == "InvalidParameter")
  {
    msg = tr("The server refused one or more registration parameters.\n\n");
    msg += tr("Please modify your request and try again.");
  }
  if (errTxt.isEmpty())
  {
    msg = tr("Registration failed due to an unspecified network or server error!\n\n");
    msg += tr("You may try to register again but if the problems persist please send an email to\n\n");
    msg += tr("\tqtournament@vkserv.de");
  }

  QMessageBox::warning(parentWidget, tr("Online Registration FAILED"), msg);

  return ERR::WRONG_STATE; // dummy value
}

