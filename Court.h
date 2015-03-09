/* 
 * File:   Team.h
 * Author: volker
 *
 * Created on March 2, 2014, 6:13 PM
 */

#ifndef COURT_H
#define	COURT_H

#include <memory>

#include <QList>

#include "GenericDatabaseObject.h"
#include "TournamentDB.h"
#include "TabRow.h"
#include "TournamentErrorCodes.h"
//#include "Match.h"

namespace QTournament
{
  class Match;

  class Court : public GenericDatabaseObject
  {

    friend class CourtMngr;
    
  public:
    QString getName(int maxLen = 0) const;
    ERR rename(const QString& newName);
    int getNumber() const;
    bool isManualAssignmentOnly() const;
    void setManualAssignment(bool isManual);
    unique_ptr<Match> getMatch() const;

  private:
    Court (TournamentDB* db, int rowId);
    Court (TournamentDB* db, dbOverlay::TabRow row);
  };

}
#endif	/* COURT_H */

