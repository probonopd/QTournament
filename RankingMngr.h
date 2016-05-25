/*
 *    This is QTournament, a badminton tournament management program.
 *    Copyright (C) 2014 - 2016  Volker Knollmann
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

#include <QList>
#include <QObject>

#include "TournamentDB.h"
#include "TournamentDataDefs.h"
#include "TournamentErrorCodes.h"
#include "DbTab.h"
#include "TournamentDatabaseObjectManager.h"
#include "Category.h"
#include "PlayerPair.h"


using namespace SqliteOverlay;

namespace QTournament
{
  class RankingEntry;

  typedef vector<RankingEntry> RankingEntryList;
  typedef vector<RankingEntryList> RankingEntryListList;

  class RankingMngr : public QObject, TournamentDatabaseObjectManager
  {
    Q_OBJECT
    
  public:
    RankingMngr (TournamentDB* _db);
    RankingEntryList createUnsortedRankingEntriesForLastRound(const Category &cat, ERR *err=nullptr, PlayerPairList _ppList=PlayerPairList(), bool reset=false);
    RankingEntryListList sortRankingEntriesForLastRound(const Category &cat, ERR *err=nullptr) const;
    ERR forceRank(const RankingEntry& re, int rank) const;
    void fillRankGaps(const Category& cat, int round, int maxRank);

    unique_ptr<RankingEntry> getRankingEntry(const PlayerPair &pp, int round) const;
    unique_ptr<RankingEntry> getRankingEntry(const Category &cat, int round, int grpNum, int rank) const;
    RankingEntryListList getSortedRanking(const Category &cat, int round) const;

    int getHighestRoundWithRankingEntryForPlayerPair(const Category &cat, const PlayerPair &pp) const;

  private:

  signals:
  };
}

#endif	/* RANKINGMNGR_H */

