/* 
 * File:   TournamentDataDefs.h
 * Author: nyoknvk1
 *
 * Created on 18. Februar 2014, 12:48
 */

#ifndef TOURNAMENTDATADEFS_H
#define	TOURNAMENTDATADEFS_H


namespace QTournament
{
#define DB_VERSION 1

#define TAB_CFG QString("Config")
#define CFG_KEY_DB_VERSION QString("DatabaseVersion")
#define CFG_KEY_TNMT_NAME QString("TournamentName")
#define CFG_KEY_TNMT_ORGA QString("OrganizingClub")
#define CFG_KEY_USE_TEAMS QString("UseTeams")
//#define CFG_KEY_ QString("")
//#define CFG_KEY_ QString("")
//#define CFG_KEY_ QString("")
//#define CFG_KEY_ QString("")
//#define CFG_KEY_ QString("")
//#define CFG_KEY_ QString("")
    
#define TAB_PLAYER QString("Player")
#define PL_FNAME QString("FirstName")
#define PL_LNAME QString("LastName")
//#define PL_ QString("")
//#define PL_ QString("")
//#define PL_ QString("")
//#define PL_ QString("")
//#define PL_ QString("")
//#define PL_ QString("")
//#define PL_ QString("")
//#define PL_ QString("")
//#define PL_ QString("")
//#define PL_ QString("")
//#define PL_ QString("")
//#define PL_ QString("")
//#define PL_ QString("")
//#define PL_ QString("")
//#define PL_ QString("")
//#define PL_ QString("")
//#define PL_ QString("")
//#define PL_ QString("")
//#define PL_ QString("")
//#define PL_ QString("")

    class TournamentSettings {
    public:
        QString tournamentName;
        QString organizingClub;
        bool useTeams;
    };


}

#endif	/* TOURNAMENTDATADEFS_H */

