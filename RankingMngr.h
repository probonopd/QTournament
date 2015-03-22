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
  typedef QList<RankingEntryList> RankingEntryListList;

  class RankingMngr : public QObject, GenericObjectManager
  {
    Q_OBJECT
    
  public:
    RankingMngr (TournamentDB* _db);
    RankingEntryList createUnsortedRankingEntriesForLastRound(const Category &cat, ERR *err=nullptr, PlayerPairList _ppList=PlayerPairList(), bool reset=false);
    RankingEntryListList sortRankingEntriesForLastRound(const Category &cat, ERR *err=nullptr) const;

    unique_ptr<RankingEntry> getRankingEntry(const PlayerPair &pp, int round) const;
    unique_ptr<RankingEntry> getRankingEntry(const Category &cat, int round, int grpNum, int rank) const;
    RankingEntryListList getSortedRanking(const Category &cat, int round) const;

  private:
    DbTab rankTab;

  signals:
  };
}

#endif	/* RANKINGMNGR_H */

