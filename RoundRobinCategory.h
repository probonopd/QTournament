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

#ifndef ROUNDROBINCATEGORY_H
#define	ROUNDROBINCATEGORY_H

#include "Category.h"
#include "RankingEntry.h"
#include "PlayerPair.h"

namespace QTournament
{

  class RoundRobinCategory : public Category
  {
    friend class Category;

  public:
    virtual ERR canFreezeConfig() override;
    virtual bool needsInitialRanking() override;
    virtual bool needsGroupInitialization() override;
    virtual ERR prepareFirstRound() override;
    virtual int calcTotalRoundsCount() const override;
    virtual std::function<bool(RankingEntry& a, RankingEntry& b)> getLessThanFunction() override;
    virtual ERR onRoundCompleted(int round) override;
    virtual PlayerPairList getRemainingPlayersAfterRound(int round, ERR *err) const override;
    virtual PlayerPairList getPlayerPairsForIntermediateSeeding() const override;
    virtual ERR resolveIntermediateSeeding(const PlayerPairList& seed) const override;

    PlayerPairList getQualifiedPlayersAfterRoundRobin_sorted() const;

    
  private:
    RoundRobinCategory (const TournamentDB& _db, int rowId);
    RoundRobinCategory (const TournamentDB& _db, const SqliteOverlay::TabRow& _row);

  } ;
}

#endif	/* ROUNDROBINCATEGORY_H */

