/* 
 * File:   RoundRobinCategory.h
 * Author: volker
 *
 * Created on August 25, 2014, 8:34 PM
 */

#ifndef ELIMCATEGORY_H
#define	ELIMCATEGORY_H

#include "Category.h"
#include "ThreadSafeQueue.h"
#include "RankingEntry.h"


using namespace dbOverlay;

namespace QTournament
{

  class EliminationCategory : public Category
  {
    friend class Category;

  public:
    virtual ERR canFreezeConfig() override;
    virtual bool needsInitialRanking() override;
    virtual bool needsGroupInitialization() override;
    virtual ERR prepareFirstRound(ProgressQueue* progressNotificationQueue=nullptr) override;
    virtual int calcTotalRoundsCount() override;
    virtual std::function<bool(RankingEntry& a, RankingEntry& b)> getLessThanFunction() override;
    virtual ERR onRoundCompleted(int round) override;
    virtual ERR prepareNextRound(PlayerPairList seeding, ProgressQueue* progressNotificationQueue=nullptr) override;
    
  private:
    EliminationCategory (TournamentDB* db, int rowId);
    EliminationCategory (TournamentDB* db, dbOverlay::TabRow row);

  } ;
}

#endif	/* ELIMCATEGORY_H */

