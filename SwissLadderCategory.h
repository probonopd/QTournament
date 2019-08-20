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

#ifndef SWISSLADDERCATEGORY_H
#define	SWISSLADDERCATEGORY_H

#include "Category.h"
#include "RankingEntry.h"


namespace QTournament
{

  class SwissLadderCategory : public Category
  {
    friend class Category;

  public:
    virtual Error canFreezeConfig() override;
    virtual bool needsInitialRanking() override;
    virtual bool needsGroupInitialization() override;
    virtual Error prepareFirstRound() override;
    virtual int calcTotalRoundsCount() const override;
    virtual std::function<bool(RankingEntry& a, RankingEntry& b)> getLessThanFunction() override;
    virtual Error onRoundCompleted(int round) override;
    virtual PlayerPairList getRemainingPlayersAfterRound(int round, Error *err) const override;
    
    ModMatchResult canModifyMatchResult(const Match& ma) const override;
    ModMatchResult modifyMatchResult(const Match& ma, const MatchScore& newScore) const override;

  private:
    SwissLadderCategory (const TournamentDB& _db, int rowId);
    SwissLadderCategory (const TournamentDB& _db, const SqliteOverlay::TabRow _row);
    bool genMatchesForNextRound() const;
    Error handleDeadlock() const;

  } ;
}

#endif	/* SWISSLADDERCATEGORY_H */

