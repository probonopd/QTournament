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

#ifndef RANKINGMNGR_H
#define	RANKINGMNGR_H

#include <memory>
#include <vector>

#include <QList>
#include <QObject>

#include <SqliteOverlay/DbTab.h>

#include "TournamentDB.h"
#include "TournamentDataDefs.h"
#include "TournamentErrorCodes.h"
#include "TournamentDatabaseObjectManager.h"
#include "Category.h"
#include "PlayerPair.h"


namespace QTournament
{
  class RankingEntry;

  using RankingEntryList = std::vector<RankingEntry>;
  using RankingEntryListList = std::vector<RankingEntryList>;

  class RankingMngr : public QObject, public TournamentDatabaseObjectManager
  {
    Q_OBJECT
    
  public:
    RankingMngr (const TournamentDB& _db);
    RankingEntryList createUnsortedRankingEntriesForLastRound(const Category &cat, ERR *err=nullptr, const PlayerPairList& _ppList={}, bool reset=false);
    RankingEntryListList sortRankingEntriesForLastRound(const Category &cat, ERR *err=nullptr) const;
    ERR forceRank(const RankingEntry& re, int rank) const;
    ERR clearRank(const RankingEntry& re) const;
    void fillRankGaps(const Category& cat, int round, int maxRank);

    std::optional<RankingEntry> getRankingEntry(const PlayerPair &pp, int round) const;
    std::optional<RankingEntry> getRankingEntry(const Category &cat, int round, int grpNum, int rank) const;
    RankingEntryListList getSortedRanking(const Category &cat, int round) const;

    int getHighestRoundWithRankingEntryForPlayerPair(const Category &cat, const PlayerPair &pp) const;

    ERR updateRankingsAfterMatchResultChange(const Match& ma, const MatchScore& oldScore, bool skipSorting=false) const;

    std::string getSyncString(const std::vector<int>& rows) const override;

  private:

  signals:
  };
}

#endif	/* RANKINGMNGR_H */

