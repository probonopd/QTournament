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

#ifndef UIPROCEDURES_H
#define GUIHELPERS_H

#include <QWidget>

#include "../Category.h"
#include "../MatchMngr.h"


/** \brief "Procedures" shall cover self-contained sequences of actions that involve
 * user interaction, dialog boxes, etc.; it shall be possible to call these procedures
 * from any widget.
 *
 * A "Procedure" shall include all error handling and preferably shall not return a
 * value.
 *
 * Procedures might be LONG LASTING or BLOCKING if they wait for user interaction. You
 * should not call them from time critical event handlers.
 */
namespace Procedures {

  /** \brief Displays the "Round completed, wanna print?" dialog after
   * a round is complete
   */
  void roundCompleted_OfferPrinting(
      QWidget* parentWidget,   ///< parent window for dialogs, etc.
      const QTournament::Category& cat,   ///< the category that has a freshly finished round
      int round   ///< the number of the round that is finished
      );

  //----------------------------------------------------------------------------

  /** \brief Display a warning message that we've just encountered a deadlock
   * situation in a Swiss Ladder category
   */
  void roundCompleted_SwissLadderDeadlock(
      QWidget* parentWidget,   ///< parent window for dialogs, etc.
      const QTournament::Category& cat   ///< the category that is affected
      );

  //----------------------------------------------------------------------------

  /** \brief Executes a walkover for a scheduled or a running match
   */
  void walkover(
      QWidget* parentWidget,   ///< parent window for dialogs, etc.
      const QTournament::Match& ma,   ///< the affected match
      int playerNum   ///< the player wins by walkover (either "1" or "2")
      );

  //----------------------------------------------------------------------------

  /** \brief Everything that has to be done after a match is finished,
   * be it normally or by walkover
   */
  void afterMatch(
      QWidget* parentWidget,   ///< parent window for dialogs, etc.
      const QTournament::Match& ma,   ///< the affected match
      const QTournament::MatchFinalizationResult& mfr   ///< some flags what has happened after the match was finished
      );
  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
}
#endif // GUIHELPERS_H
