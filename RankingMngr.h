/* 
 * File:   TeamMngr.h
 * Author: nyoknvk1
 *
 * Created on 18. Februar 2014, 14:04
 */

#ifndef RANKINGMNGR_H
#define	RANKINGMNGR_H

#include <memory>

#include <QList>
#include <QObject>

#include "TournamentDB.h"
#include "TournamentDataDefs.h"
#include "TournamentErrorCodes.h"
#include "DbTab.h"
#include "GenericObjectManager.h"
#include "Category.h"
#include "PlayerPair.h"


using namespace dbOverlay;

namespace QTournament
{
  class RankingEntry;

  typedef QList<RankingEntry> RankingEntryList;

  class RankingMngr : public QObject, GenericObjectManager
  {
    Q_OBJECT
    
  public:
    RankingMngr (TournamentDB* _db);
    RankingEntryList createUnsortedRankingEntriesForLastRound(const Category &cat, ERR *err, PlayerPairList _ppList, bool reset);

    unique_ptr<RankingEntry> getRankingEntry(const Category &cat, const PlayerPair &pp, int round) const;
    RankingEntryList getSortedRanking(const Category &cat, int round) const;

  private:
    DbTab rankTab;

  signals:
  };
}

#endif	/* RANKINGMNGR_H */

