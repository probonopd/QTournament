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

#include <QMessageBox>
#include <QString>

#include <SimpleReportGeneratorLib/SimpleReportViewer.h>
#include "../../Match.h"
#include "../../MatchMngr.h"

#include "../Procedures.h"

#include "../DlgSelectReferee.h"
#include "reports/ResultSheets.h"
#include "../GuiHelpers.h"

using namespace QTournament;

namespace Procedures {

  void walkover(QWidget* parentWidget, const QTournament::Match& ma, int playerNum)
  {
    if ((playerNum != 1) && (playerNum != 2)) return; // shouldn't happen

    if (!(ma.isWalkoverPossible())) return;

    // get a user confirmation
    QString msg = QObject::tr("This will be a walkover for\n\n\t");
    if (playerNum == 1)
    {
      msg += ma.getPlayerPair1().getDisplayName();
    } else {
      msg += ma.getPlayerPair2().getDisplayName();
    }
    msg += "\n\n";
    msg += QObject::tr("All games will be 21:0.") + "\n\n";
    msg += QObject::tr("WARNING: this step is irrevocable!") + "\n\n";
    msg += QObject::tr("Proceed?");
    int result = QMessageBox::question(parentWidget, QObject::tr("Confirm walkover"), msg);
    if (result != QMessageBox::Yes)
    {
      return;
    }

    QTournament::MatchMngr mm{ma.getDatabaseHandle()};
    auto finalizationResult = mm.walkover(ma, playerNum);

    afterMatch(parentWidget, ma, finalizationResult);
  }

  //----------------------------------------------------------------------------

  void afterMatch(QWidget* parentWidget, const QTournament::Match& ma, const QTournament::MatchFinalizationResult& mfr)
  {
    if (mfr.err != Error::OK)
    {
      QString msg = QObject::tr("A dabase error occurred. The match result has not been stored.");
      QMessageBox::critical(parentWidget, QObject::tr("Store match result"), msg);
      return;
    }

    // if we messed up and caused a Swiss Ladder deadlock,
    // confess to the user
    if (mfr.hasSwissLadderDeadlock)
    {
      Procedures::roundCompleted_SwissLadderDeadlock(parentWidget, ma.getCategory());
    }

    // if we have just finished a round, show an info dialog
    // along with printing option
    if (mfr.completedRound)
    {
      Procedures::roundCompleted_OfferPrinting(parentWidget, ma.getCategory(), *mfr.completedRound);
    }
  }

  //----------------------------------------------------------------------------

  void preassignUmpireToMatch(QWidget* parentWidget, const QTournament::Match& ma)
  {
    RefereeMode refMode = ma.get_EFFECTIVE_RefereeMode();
    if ((refMode == RefereeMode::None) || (refMode == RefereeMode::HandWritten) || (refMode == RefereeMode::UseDefault))
    {
      return;   // nothing to do for us
    }

    // make sure we can assign a referee
    Error err = ma.canAssignReferee(RefereeAction::PreAssign);
    if (err != Error::OK)
    {
      QString msg;
      msg = QObject::tr("It is currently not possible to assign\n");
      msg += QObject::tr("an umpire to this match. Maybe the match has\n");
      msg += QObject::tr("already been started or finished?\n");
      QMessageBox::warning(parentWidget, QObject::tr("Umpire assignment failed"), msg);

      return;
    }

    // let the user pick the referee
    DlgSelectReferee dlg{ma.getDatabaseHandle(), ma, RefereeAction::PreAssign, parentWidget};
    int result = dlg.exec();
    if (result != QDialog::Accepted)
    {
      return;
    }
    auto selPlayer = dlg.getFinalPlayerSelection();
    if (!selPlayer) return;  // user cancled

    // actually do the assignment
    MatchMngr mm{ma.getDatabaseHandle()};
    err = mm.assignReferee(ma, *selPlayer, RefereeAction::PreAssign);
    if (err != Error::OK)
    {
      QString msg = QObject::tr("Could not assign umpire to match.\n");
      msg += QObject::tr("Maybe you tried to assign one of the players as umpire?");
      QMessageBox::warning(parentWidget, QObject::tr("Umpire assignment failed"), msg);
    }
  }

  //----------------------------------------------------------------------------

  void selectUmpireOnMatchCall(QWidget* parentWidget, const QTournament::Match& ma)
  {
    RefereeMode refMode = ma.get_EFFECTIVE_RefereeMode();
    if ((refMode == RefereeMode::None) || (refMode == RefereeMode::HandWritten) || (refMode == RefereeMode::UseDefault))
    {
      return;   // nothing to do for us
    }

    // make sure we can assign a referee
    Error err = ma.canAssignReferee(RefereeAction::MatchCall);
    if (err != Error::OK)
    {
      QString msg;
      msg = QObject::tr("An unexpected error occured: the match needs an umpire,\n");
      msg += QObject::tr("but an umpire can't be assigned right now.\n\n");
      msg += QObject::tr("The match cannot be started.");
      QMessageBox::warning(parentWidget, QObject::tr("Umpire assignment failed"), msg);

      return;
    }

    // let the user pick the referee
    DlgSelectReferee dlg{ma.getDatabaseHandle(), ma, RefereeAction::MatchCall, parentWidget};
    int result = dlg.exec();
    if (result != QDialog::Accepted)
    {
      return;
    }
    auto selPlayer = dlg.getFinalPlayerSelection();

    // if selPlayer is null and we are in a match call, the user decided to
    // continue without referee
    MatchMngr mm{ma.getDatabaseHandle()};
    if (!selPlayer)
    {
      err = mm.setRefereeMode(ma, RefereeMode::None);
      if (err != Error::OK)
      {
        QString msg = QObject::tr("Cannot continue without umpire!");
        QMessageBox::warning(parentWidget, QObject::tr("Umpire assignment failed"), msg);
        return;
      }
      err = mm.removeReferee(ma);
      if (err != Error::OK)
      {
        QString msg = QObject::tr("Cannot continue without umpire!");
        QMessageBox::warning(parentWidget, QObject::tr("Umpire assignment failed"), msg);
      }

      // there's nothing more to do for us
      return;
    }

    // actually do the assignment
    err = mm.assignReferee(ma, *selPlayer, RefereeAction::MatchCall);
    if (err != Error::OK)
    {
      QString msg = QObject::tr("Could not assign umpire to match.\n");
      msg += QObject::tr("Maybe you tried to assign one of the players as umpire?");
      QMessageBox::warning(parentWidget, QObject::tr("Umpire assignment failed"), msg);
      return;
    }

    // since we are calling a match, the user might want to print a
    // new result sheet with the freshly umpire name
    QString msg = QObject::tr("Do you want to print a new result sheet with the\n");
    msg += QObject::tr("updated umpire name for this match?");
    result = QMessageBox::question(parentWidget, QObject::tr("Print result sheet?"), msg);
    if (result == QMessageBox::Yes)
    {
      // create a report instance that is locked to show only one match
      ResultSheets sheet{ma.getDatabaseHandle(), ma};

      // let the report object create the actual output
      upSimpleReport rep = sheet.regenerateReport();

      // create an invisible report viewer and directly trigger
      // the print reaction
      SimpleReportLib::SimpleReportViewer viewer{parentWidget};
      viewer.setReport(rep.get());
      viewer.onBtnPrintClicked();
    }
  }

  //----------------------------------------------------------------------------

  void swapUmpire(QWidget* parentWidget, const QTournament::Match& ma)
  {
    RefereeMode refMode = ma.get_RAW_RefereeMode();
    if ((refMode == RefereeMode::None) || (refMode == RefereeMode::HandWritten) || (refMode == RefereeMode::UseDefault))
    {
      return;   // nothing to do for us
    }

    // make sure we can assign a referee
    Error err = ma.canAssignReferee(RefereeAction::Swap);
    if (err != Error::OK)
    {
      QString msg;
      msg = QObject::tr("It is currently not possible to swap\n");
      msg += QObject::tr("the assigned umpire for this match.");
      QMessageBox::warning(parentWidget, QObject::tr("Umpire assignment failed"), msg);

      return;
    }

    // let the user pick the referee
    DlgSelectReferee dlg{ma.getDatabaseHandle(), ma, RefereeAction::Swap, parentWidget};
    int result = dlg.exec();
    if (result != QDialog::Accepted)
    {
      return;
    }
    auto selPlayer = dlg.getFinalPlayerSelection();
    if (!selPlayer) return;  // user cancled

    // actually do the assignment
    MatchMngr mm{ma.getDatabaseHandle()};
    err = mm.assignReferee(ma, *selPlayer, RefereeAction::Swap);
    if (err != Error::OK)
    {
      QString msg = QObject::tr("Could not assign umpire to match.\n");
      msg += QObject::tr("Maybe you tried to assign one of the players as umpire?");
      QMessageBox::warning(parentWidget, QObject::tr("Umpire assignment failed"), msg);
    }
  }

  //----------------------------------------------------------------------------

  void callMatch(QWidget* parentWidget, const QTournament::Match& ma, const QTournament::Court& co)
  {
    MatchMngr mm{ma.getDatabaseHandle()};

    // this is a flag that tells us to remove the
    // umpire assignment in case we cancel the call
    bool callStartedWithUnassignedReferee = false;

    // check if we need to ask the user for a referee
    Error err = mm.canAssignMatchToCourt(ma, co);
    if (err == Error::MatchNeedsReferee)
    {
      callStartedWithUnassignedReferee = true;
      Procedures::selectUmpireOnMatchCall(parentWidget, ma);

      // if the match still needs a referee, the user
      // has canceled the selection dialog
      err = mm.canAssignMatchToCourt(ma, co);
      if (err == Error::MatchNeedsReferee) return;
    }

    // all necessary pre-checks should have been performed before
    // so that the following call should always yield "ok"
    err = mm.canAssignMatchToCourt(ma, co);
    if (err != Error::OK)
    {
      QString msg = QObject::tr("An unexpected error occured.\n");
      msg += QObject::tr("Sorry, this shouldn't happen.\n");
      msg += QObject::tr("The match cannot be started.");
      QMessageBox::critical(parentWidget, QObject::tr("Assign match to court"), msg);

      // restore the initial referee-state, if necessary
      if (callStartedWithUnassignedReferee)
      {
        mm.removeReferee(ma);
      }

      return;
    }

    // prep the call
    QString call = GuiHelpers::prepCall(ma, co);
    int result = QMessageBox::question(parentWidget, QObject::tr("Assign match to court"), call);

    if (result == QMessageBox::Yes)
    {
      // after all the checks before, the following call
      // should always yield "ok"
      err = mm.assignMatchToCourt(ma, co);
      if (err != Error::OK)
      {
        QString msg = QObject::tr("An unexpected error occured.\n");
        msg += QObject::tr("Sorry, this shouldn't happen.\n");
        msg += QObject::tr("The match cannot be started.");
        QMessageBox::critical(parentWidget, QObject::tr("Assign match to court"), msg);

        // restore the initial referee-state, if necessary
        if (callStartedWithUnassignedReferee)
        {
          mm.removeReferee(ma);
        }

        return;
      }

      return;
    }

    // the user hit cancel.
    // restore the initial referee-state, if necessary
    if (callStartedWithUnassignedReferee)
    {
      mm.removeReferee(ma);
    }
  }
}
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


