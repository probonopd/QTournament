/*
 *    This is QTournament, a badminton tournament management program.
 *    Copyright (C) 2014 - 2016  Volker Knollmann
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

#include "SimpleReportViewer.h"

#include "cmdCallMatch.h"
#include "PlayerMngr.h"
#include "MatchMngr.h"
#include "CourtMngr.h"
#include "ui/commonCommands/cmdAssignRefereeToMatch.h"
#include "ui/GuiHelpers.h"

cmdCallMatch::cmdCallMatch(QWidget* p, const QTournament::Match &_ma, const Court& _co)
  :AbstractCommand(_ma.getDatabaseHandle(), p), ma(_ma), co(_co)
{

}

//----------------------------------------------------------------------------

ERR cmdCallMatch::exec()
{
  MatchMngr mm{db};

  // this is a flag that tells us to remove the
  // umpire assignment in case we cancel the call
  bool callStartedWithUnassignedReferee = false;

  // check if we need to ask the user for a referee
  ERR err = mm.canAssignMatchToCourt(ma, co);
  if (err == MATCH_NEEDS_REFEREE)
  {
    callStartedWithUnassignedReferee = true;
    cmdAssignRefereeToMatch cmd{parentWidget, ma, REFEREE_ACTION::MATCH_CALL};
    err = cmd.exec();
    if (err != OK) return err;

    // if the match still needs a referee, the user
    // has canceled the selection dialog
    err = mm.canAssignMatchToCourt(ma, co);
    if (err == MATCH_NEEDS_REFEREE) return OK;
  }

  // all necessary pre-checks should have been performed before
  // so that the following call should always yield "ok"
  err = mm.canAssignMatchToCourt(ma, co);
  if (err != OK)
  {
    QString msg = tr("An unexpected error occured.\n");
    msg += tr("Sorry, this shouldn't happen.\n");
    msg += tr("The match cannot be started.");
    QMessageBox::critical(parentWidget, tr("Assign match to court"), msg);

    // restore the initial referee-state, if necessary
    if (callStartedWithUnassignedReferee)
    {
      mm.removeReferee(ma);
    }

    return err;
  }

  // prep the call
  QString call = GuiHelpers::prepCall(ma, co);
  int result = QMessageBox::question(parentWidget, tr("Assign match to court"), call);

  if (result == QMessageBox::Yes)
  {
    // after all the checks before, the following call
    // should always yield "ok"
    err = mm.assignMatchToCourt(ma, co);
    if (err != OK)
    {
      QString msg = tr("An unexpected error occured.\n");
      msg += tr("Sorry, this shouldn't happen.\n");
      msg += tr("The match cannot be started.");
      QMessageBox::critical(parentWidget, tr("Assign match to court"), msg);

      // restore the initial referee-state, if necessary
      if (callStartedWithUnassignedReferee)
      {
        mm.removeReferee(ma);
      }

      return err;
    }

    return OK;
  }

  // the user hit cancel.
  // restore the initial referee-state, if necessary
  if (callStartedWithUnassignedReferee)
  {
    mm.removeReferee(ma);
  }

  QMessageBox::information(parentWidget, tr("Assign match to court"), tr("Call cancled, match not started"));
  return OK;
}

