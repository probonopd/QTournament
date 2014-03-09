/* 
 * File:   Category.h
 * Author: volker
 *
 * Created on March 2, 2014, 6:13 PM
 */

#ifndef CATEGORY_H
#define	CATEGORY_H

#include "GenericDatabaseObject.h"
#include "TournamentDB.h"
#include "TabRow.h"
#include "TournamentErrorCodes.h"

namespace QTournament
{

  class Category : public GenericDatabaseObject
  {
    friend class CatMngr;
    
  public:
    QString getName();
    ERR rename(const QString& newName);
    MATCH_TYPE getMatchType();
    ERR setMatchType(MATCH_TYPE t);
    MATCH_SYSTEM getMatchSystem();
    ERR setMatchSystem(MATCH_SYSTEM s);
    SEX getSex();
    ERR setSex(SEX s);

  private:
    Category (TournamentDB* db, int rowId);
    Category (TournamentDB* db, dbOverlay::TabRow row);
  };

}
#endif	/* TEAM_H */

