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

using namespace QTournament;

cmdOnlineRegistration::cmdOnlineRegistration(QWidget* p, const TournamentDB& _db)
  :AbstractCommand(_db, p)
{

}

//----------------------------------------------------------------------------

Error cmdOnlineRegistration::exec()
{
  OnlineMngr* om = db.getOnlineManager();

  // if the user hasn't supplied a password yet,
  // do that now
  if (!(om->hasSecretInDatabase()))
  {
    cmdSetOrChangePassword cmd{parentWidget, db};
    Error e = cmd.exec();
    if (e != Error::OK) return Error::WrongState;  // dummy value
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

  // show the registration form
  SqliteOverlay::KeyValueTab cfg{db, TabCfg};
  std::string tName = cfg[CfgKey_TnmtName];
  std::string club = cfg[CfgKey_TnmtOrga];
  DlgRegisterTournament dlg{parentWidget, QString::fromUtf8(tName.c_str()), QString::fromUtf8(club.c_str())};
  int rc = dlg.exec();
  if (rc != QDialog::Accepted) return Error::WrongState;  // dummy error code; will not be evaluated by caller

  // show a consent form, either in English or German
  bool isGerman = QLocale().name().startsWith("de", Qt::CaseInsensitive);
  QFile binData{(isGerman ? ":/ui/consent_de.html" : ":/ui/consent.html")};
  QString consentTxt = binData.readAll();
  QMessageBox dlgConsent{parentWidget};
  dlgConsent.setText(consentTxt);
  dlgConsent.setStandardButtons(QMessageBox::No | QMessageBox::Yes);
  dlgConsent.setDefaultButton(QMessageBox::Yes);
  dlgConsent.setWindowTitle(tr("Please read carefully"));
  rc = dlgConsent.exec();
  if (rc != QMessageBox::Yes) return Error::WrongState;  // dummy error code; will not be evaluated by caller

  // do the actual registration
  //
  // this can last up to five seconds (--> timeout) and thus
  // we better enable the hourglass cursor
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  QString errTxt;
  OnlineError err = om->registerTournament(dlg.getValidatedRegistrationData(), errTxt);
  QApplication::restoreOverrideCursor();

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

    case OnlineError::IncompatibleVersions:
      msg = tr("<p>This version of QTournament is incompatible with the server.</p>");
      msg += tr("<p>Please download a more recent version from:</p>");
      msg += "<p><a href='http://qtournament.de'>QTournament.de</a></p>";
      break;

    default:
      msg = tr("Registration failed due to an unspecified network or server error!");
    }

    QMessageBox::warning(parentWidget, tr("Online Registration FAILED"), msg);
    return Error::WrongState; // dummy value
  }

  // at this point, the data exchange with the server was successful (HTTP and Signatures).
  // We only have to check if the request on application level was successful as well.
  QString msg;
  if (err == OnlineError::Okay)
  {
    msg = tr("Online registration successful!\n\n");
    msg += tr("Within the next minutes you should receive a confirmation email. ");
    msg += tr("Please click on the link in the email to complete your registration.");
    QMessageBox::information(parentWidget, tr("Online registration"), msg);

    return Error::OK;
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
    msg += tr("\tinfo@qtournament.de");
  }
  if (errTxt == "InvalidParameter")
  {
    msg = tr("The server refused one or more registration parameters.\n\n");
    msg += tr("Please modify your request and try again.");
  }
  if (errTxt == "MailError")
  {
    msg = tr("The server could not send a confirmation email to the address you've provided.\n\n");
    msg += tr("Please try again and send an email to info@qtournament.de if the problem persists.");
  }
  if (errTxt.isEmpty())
  {
    msg = tr("Registration failed due to an unspecified network or server error!\n\n");
    msg += tr("You may try to register again but if the problems persist please send an email to\n\n");
    msg += tr("\tinfo@qtournament.de");
  }

  QMessageBox::warning(parentWidget, tr("Online Registration FAILED"), msg);

  return Error::WrongState; // dummy value
}

