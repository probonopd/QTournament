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

#ifndef RANKINGENTRY_H
#define	RANKINGENTRY_H

#include <memory>
#include <tuple>

#include <QList>

#include "GenericDatabaseObject.h"
#include "TournamentDB.h"
#include "TabRow.h"
#include "TournamentErrorCodes.h"
#include "Category.h"
#include "PlayerPair.h"

namespace QTournament
{
  class RankingEntry : public GenericDatabaseObject
  {

    friend class RankingMngr;
    friend class GenericObjectManager;

  public:
    static constexpr int NO_RANK_ASSIGNED = -1;
    static constexpr int NO_GROUP_ASSIGNED = -999;

    int getRound() const;
    Category getCategory() const;
    unique_ptr<PlayerPair> getPlayerPair() const;
    int getRank() const;
    int getGroupNumber() const;

    // sequence: won, draw, lost, total
    tuple<int, int, int, int> getMatchStats() const;

    // sequence: won, lost, total
    tuple<int, int, int> getGameStats() const;

    // sequence: won, lost
    tuple<int, int> getPointStats() const;

    // required for sort()
    friend void swap(RankingEntry& a, RankingEntry& b) noexcept
    {
      // only swap the row member; no need to swap the DB-pointer
      // because it is identical for both instances
      swap(a.row, b.row);
    }

  private:
    RankingEntry (TournamentDB* db, int rowId);
    RankingEntry (TournamentDB* db, dbOverlay::TabRow row);
    int QVariant2Int_WithDefault(const QVariant& v, int defaultVal=0) const;
  };

}
#endif	/* RANKINGENTRY_H */

