/* 
 * File:   Team.h
 * Author: volker
 *
 * Created on March 2, 2014, 6:13 PM
 */

#ifndef MATCH_H
#define	MATCH_H

#include <memory>

#include "GenericDatabaseObject.h"
#include "TournamentDB.h"
#include "TabRow.h"
#include "TournamentErrorCodes.h"
#include "Category.h"
#include "Court.h"
//#include "MatchGroup.h"
#include "Score.h"

#include <QList>

namespace QTournament
{
  class MatchGroup;

  class Match : public GenericDatabaseObject
  {
    friend class MatchMngr;
    friend class MatchGroup;
    
  public:
    Category getCategory() const;
    MatchGroup getMatchGroup() const;
    bool hasPlayerPair1() const;
    bool hasPlayerPair2() const;
    bool hasBothPlayerPairs() const;
    PlayerPair getPlayerPair1() const;
    PlayerPair getPlayerPair2() const;
    int getMatchNumber() const;
    bool hasAllPlayersIdle() const;
    QString getDisplayName() const;

    unique_ptr<MatchScore> getScore(ERR *err=nullptr);
    ERR setScore(const MatchScore& score);

    unique_ptr<Court> getCourt(ERR *err=nullptr) const;

  private:
    Match (TournamentDB* db, int rowId);
    Match (TournamentDB* db, dbOverlay::TabRow row);

  };

}
#endif	/* MATCH_H */

