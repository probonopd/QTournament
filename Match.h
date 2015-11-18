/* 
 * File:   Team.h
 * Author: volker
 *
 * Created on March 2, 2014, 6:13 PM
 */

#ifndef MATCH_H
#define	MATCH_H

#include <memory>

#include <QDateTime>

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
    friend class GenericObjectManager;
    
  public:
    Category getCategory() const;
    MatchGroup getMatchGroup() const;
    bool hasPlayerPair1() const;
    bool hasPlayerPair2() const;
    bool hasBothPlayerPairs() const;
    PlayerPair getPlayerPair1() const;
    PlayerPair getPlayerPair2() const;
    int getMatchNumber() const;
    QString getDisplayName(const QString& localWinnerName, const QString& localLoserName) const;

    unique_ptr<MatchScore> getScore(ERR *err=nullptr) const;
    unique_ptr<PlayerPair> getWinner() const;
    unique_ptr<PlayerPair> getLoser() const;

    unique_ptr<Court> getCourt(ERR *err=nullptr) const;
    QList<Player> determineActualPlayers() const;

    int getSymbolicPlayerPair1Name() const;
    int getSymbolicPlayerPair2Name() const;

    int getWinnerRank() const;
    int getLoserRank() const;

    bool isWalkoverPossible() const;

    QDateTime getStartTime() const;
    bool addAddtionalCallTime() const;
    QList<QDateTime> getAdditionalCallTimes() const;

  private:
    Match (TournamentDB* db, int rowId);
    Match (TournamentDB* db, dbOverlay::TabRow row);
    int getSymbolicPlayerPairName(int playerPos) const;

  };

  typedef unique_ptr<Match> upMatch;

}
#endif	/* MATCH_H */

