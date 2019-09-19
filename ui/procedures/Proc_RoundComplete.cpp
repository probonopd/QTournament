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

#include <QMessageBox>
#include <QString>

#include "../Procedures.h"

#include "../DlgRoundFinished.h"

using namespace QTournament;

namespace Procedures {

  void roundCompleted_OfferPrinting(QWidget* parentWidget, const QTournament::Category& cat, int round)
  {
    // present an info dialog along with the option to directly
    // print some useful reports
    DlgRoundFinished dlg{parentWidget, cat, round};
    dlg.exec();
  }

  //----------------------------------------------------------------------------

  void roundCompleted_SwissLadderDeadlock(QWidget* parentWidget, const QTournament::Category& cat)
  {
    int nPairs = cat.getPlayerPairs().size();

    int nRoundsTheory = ((nPairs % 2) == 0) ? nPairs - 1 : nPairs;
    int nRoundsActual = cat.convertToSpecializedObject()->calcTotalRoundsCount();

    QString msg = QObject::tr("<br><center><b><font color=\"red\">SWISS LADDER DEADLOCK</font></b></center><br><br>");
    msg += QObject::tr("Unfortuantely, the sequence of matches in past rounds in the category %3 has lead ");
    msg += QObject::tr("to a deadlock. We can't play any more rounds in this category without repeating already ");
    msg += QObject::tr("played matches.<br><br>");
    msg += QObject::tr("Thus, the category has been reduced from %1 to %2 rounds and ");
    msg += QObject::tr("is now finished.<br><br>");
    msg += QObject::tr("Normally, such a deadlock should not happen... sincere apologies for this!<br><br>");
    msg = msg.arg(nRoundsTheory).arg(nRoundsActual).arg(cat.getName());

    QMessageBox::warning(parentWidget, QObject::tr("Swiss Ladder Deadlock"), msg);
  }
}
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


