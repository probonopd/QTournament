/*
 *    This is QTournament, a badminton tournament management program.
 *    Copyright (C) 2014 - 2015  Volker Knollmann
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

#ifndef TOURNAMENTDB_H
#define	TOURNAMENTDB_H

#include <tuple>

#include "GenericDatabase.h"

#include "TournamentDataDefs.h"

namespace QTournament
{

    class TournamentDB : public dbOverlay::GenericDatabase
    {
    public:

        TournamentDB(QString fName, bool createNew);
        virtual void populateTables();
        virtual void populateViews();
        void createIndices();

        tuple<int, int> getVersion();

        bool isCompatibleDatabaseVersion();

        bool needsConversion();
        bool convertToLatestDatabaseVersion();

    private:
    };
}

#endif	/* TOURNAMENTDB_H */

