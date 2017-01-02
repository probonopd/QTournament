/*
 *    This is QTournament, a badminton tournament management program.
 *    Copyright (C) 2014 - 2017  Volker Knollmann
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

#ifndef PUREROUNDROBINCATEGORY_H
#define	PUREROUNDROBINCATEGORY_H

#include "Category.h"
#include "ThreadSafeQueue.h"
#include "RankingEntry.h"


using namespace SqliteOverlay;

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
    int getRoundCountPerIteration() const;
    int getIterationCount() const;
    static unique_ptr<PureRoundRobinCategory> getFromGenericCat(const Category& cat);

  private:
    PureRoundRobinCategory (TournamentDB* db, int rowId);
    PureRoundRobinCategory (TournamentDB* db, SqliteOverlay::TabRow row);

  } ;
}

#endif	/* PUREROUNDROBINCATEGORY_H */

