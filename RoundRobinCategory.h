/* 
 * File:   RoundRobinCategory.h
 * Author: volker
 *
 * Created on August 25, 2014, 8:34 PM
 */

#ifndef ROUNDROBINCATEGORY_H
#define	ROUNDROBINCATEGORY_H

#include "Category.h"
#include "ThreadSafeQueue.h"


using namespace dbOverlay;

namespace QTournament
{

  class RoundRobinCategory : public Category
  {
    friend class Category;

  public:
    virtual ERR canFreezeConfig() override;
    virtual bool needsInitialRanking() override;
    virtual bool needsGroupInitialization() override;
    virtual ERR prepareFirstRound(ProgressQueue* progressNotificationQueue) override;
    virtual int calcTotalRoundsCount() override;

    
  private:
    RoundRobinCategory (TournamentDB* db, int rowId);
    RoundRobinCategory (TournamentDB* db, dbOverlay::TabRow row);

  } ;
}

#endif	/* ROUNDROBINCATEGORY_H */

