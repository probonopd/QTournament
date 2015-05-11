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
    virtual int calcTotalRoundsCount() const override;
    virtual std::function<bool(RankingEntry& a, RankingEntry& b)> getLessThanFunction() override;
    virtual ERR onRoundCompleted(int round) override;
    virtual PlayerPairList getRemainingPlayersAfterRound(int round, ERR *err) const override;
    
  private:
    EliminationCategory (TournamentDB* db, int rowId, int eliminationMode);
    EliminationCategory (TournamentDB* db, dbOverlay::TabRow row, int eliminationMode);

    int elimMode;

  } ;
}

#endif	/* ELIMCATEGORY_H */

