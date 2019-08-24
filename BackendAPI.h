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

#ifndef BACKENDAPI_H
#define	BACKENDAPI_H

#include <memory>
#include <optional>

//#include "TournamentDB.h"
#include "TournamentErrorCodes.h"


namespace QTournament
{
  class TournamentDB;
  class Match;

  /** \brief Only queries that are guaranteed to not modify the database
   */
  namespace API::Qry
  {
    /** \returns the next callable match with the lowest match number
     */
    std::optional<Match> nextCallableMatch(
        const TournamentDB& db
        );
  }
}
#endif	/* COURT_H */

