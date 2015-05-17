/* 
 * File:   RoundRobinCategory.h
 * Author: volker
 *
 * Created on August 25, 2014, 8:34 PM
 */

#ifndef PUREROUNDROBINCATEGORY_H
#define	PUREROUNDROBINCATEGORY_H

#include "Category.h"
#include "ThreadSafeQueue.h"
#include "RankingEntry.h"


using namespace dbOverlay;

namespace QTournament
{

  class PureRoundRobinCategory : public Category
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
    PureRoundRobinCategory (TournamentDB* db, int rowId);
    PureRoundRobinCategory (TournamentDB* db, dbOverlay::TabRow row);

  } ;
}

#endif	/* PUREROUNDROBINCATEGORY_H */

