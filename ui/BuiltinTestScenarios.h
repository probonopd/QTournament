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

#ifndef BUILTINTESTSCENARIOS_H
#define BUILTINTESTSCENARIOS_H

#include "../TournamentDB.h"
#include "../TeamMngr.h"
#include "../PlayerMngr.h"
#include "../CatMngr.h"
#include "../MatchMngr.h"
#include "../CourtMngr.h"

namespace QTournament
{
  namespace BuiltinScenarios
  {
    // the dispatcher for calling the right
    // setup method based on its ID
    void setupTestScenario(const TournamentDB& db, int scenarioID);

    //----------------------------------------------------------------------------

    void setupScenario01(const TournamentDB& db);

    //----------------------------------------------------------------------------

    void setupScenario02(const TournamentDB& db);

    //----------------------------------------------------------------------------

    // a scenario with a lot of participants, including a group of
    // forty players in one category
    void setupScenario03(const TournamentDB& db);

    //----------------------------------------------------------------------------

    // extend scenario 3 to already start category "LS"
    // and add a few players to LD and start this category, too
    void setupScenario04(const TournamentDB& db);

    //----------------------------------------------------------------------------

    // extend scenario 4 to already stage and schedule a few match groups
    // in category "LS" and "LD"
    // Additionally, we add 4 courts to the tournament
    void setupScenario05(const TournamentDB& db);

    //----------------------------------------------------------------------------

    // extend scenario 5 to already play all matches in the round-robin rounds
    // of category "LS" and "LD"
    void setupScenario06(const TournamentDB& db);

    //----------------------------------------------------------------------------

    // extend scenario 3, set the LS match system to "single elimination",
    // run the category, stage the first three rounds, play the first
    // round and start the second
    void setupScenario07(const TournamentDB& db);

    //----------------------------------------------------------------------------

    // a scenario with up to 10 players in a ranking1-bracket
    void setupScenario08(const TournamentDB& db);

    //----------------------------------------------------------------------------

  }
}

#endif // BUILTINTESTSCENARIOS_H
