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

#include "../../Match.h"
#include "../../MatchMngr.h"

#include "../Procedures.h"

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

}
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


