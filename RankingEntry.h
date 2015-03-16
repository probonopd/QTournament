/* 
 * File:   Team.h
 * Author: volker
 *
 * Created on March 2, 2014, 6:13 PM
 */

#ifndef RANKINGENTRY_H
#define	RANKINGENTRY_H

#include <memory>
#include <tuple>

#include <QList>

#include "GenericDatabaseObject.h"
#include "TournamentDB.h"
#include "TabRow.h"
#include "TournamentErrorCodes.h"
#include "Category.h"
#include "PlayerPair.h"

namespace QTournament
{
  class RankingEntry : public GenericDatabaseObject
  {

    friend class RankingMngr;
    friend class GenericObjectManager;

  public:
    static constexpr int NO_RANK_ASSIGNED = -1;
    static constexpr int NO_GROUP_ASSIGNED = -999;

    int getRound() const;
    Category getCategory() const;
    PlayerPair getPlayerPair() const;
    int getRank() const;
    int getGroupNumber() const;

    // sequence: won, draw, lost, total
    tuple<int, int, int, int> getMatchStats() const;

    // sequence: won, lost, total
    tuple<int, int, int> getGameStats() const;

    // sequence: won, lost
    tuple<int, int> getPointStats() const;

  private:
    RankingEntry (TournamentDB* db, int rowId);
    RankingEntry (TournamentDB* db, dbOverlay::TabRow row);
    int QVariant2Int_WithDefault(const QVariant& v, int defaultVal=0) const;
  };

}
#endif	/* RANKINGENTRY_H */

