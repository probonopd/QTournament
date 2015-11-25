/*
 *    This is QTournament, a badminton tournament management program.
 *    Copyright (C) 2014 - 2015  Volker Knollmann
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
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

