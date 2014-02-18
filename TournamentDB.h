/* 
 * File:   TournamentDB.h
 * Author: nyoknvk1
 *
 * Created on 18. Februar 2014, 12:33
 */

#ifndef TOURNAMENTDB_H
#define	TOURNAMENTDB_H

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

    private:
    };
}

#endif	/* TOURNAMENTDB_H */

