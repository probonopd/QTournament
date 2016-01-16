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

#include <stdexcept>

#include "RankingEntry.h"
#include "TournamentDataDefs.h"
#include "TournamentDB.h"
#include "TournamentErrorCodes.h"
#include "Tournament.h"

namespace QTournament
{

  RankingEntry::RankingEntry(TournamentDB* db, int rowId)
  :TournamentDatabaseObject(db, TAB_RANKING, rowId)
  {
  }

//----------------------------------------------------------------------------

  RankingEntry::RankingEntry(TournamentDB* db, SqliteOverlay::TabRow row)
  :TournamentDatabaseObject(db, row)
  {
  }

//----------------------------------------------------------------------------

  int RankingEntry::getRound() const
  {
    return row[RA_ROUND].toInt();
  }

//----------------------------------------------------------------------------

  Category RankingEntry::getCategory() const
  {
    int catId = row[RA_CAT_REF].toInt();
    auto tnmt = Tournament::getActiveTournament();
    return tnmt->getCatMngr()->getCategoryById(catId);
  }

//----------------------------------------------------------------------------

  unique_ptr<PlayerPair> RankingEntry::getPlayerPair() const
  {
    QVariant _ppId = row[RA_PAIR_REF];
    if (_ppId.isNull()) return nullptr;

    int ppId = _ppId.toInt();
    return unique_ptr<PlayerPair>(new PlayerPair(db, ppId));
  }

//----------------------------------------------------------------------------

  int RankingEntry::getRank() const
  {
    int rank = QVariant2Int_WithDefault(row[RA_RANK], -1);
    if (rank <= 0) return NO_RANK_ASSIGNED;

    return rank;
  }

//----------------------------------------------------------------------------

  int RankingEntry::getGroupNumber() const
  {
    return row[RA_GRP_NUM].toInt();
  }

//----------------------------------------------------------------------------

  tuple<int, int, int, int> RankingEntry::getMatchStats() const
  {
    int won = QVariant2Int_WithDefault(row[RA_MATCHES_WON]);
    int draw = QVariant2Int_WithDefault(row[RA_MATCHES_DRAW]);
    int lost = QVariant2Int_WithDefault(row[RA_MATCHES_LOST]);

    return make_tuple(won, draw, lost, won+draw+lost);
  }

//----------------------------------------------------------------------------

  int RankingEntry::QVariant2Int_WithDefault(const QVariant &v, int defaultVal) const
  {
    if ((v.isNull()) || (!(v.isValid())))
    {
      return defaultVal;
    }

    return v.toInt();
  }

//----------------------------------------------------------------------------

  tuple<int, int, int> RankingEntry::getGameStats() const
  {
    int won = QVariant2Int_WithDefault(row[RA_GAMES_WON]);
    int lost = QVariant2Int_WithDefault(row[RA_GAMES_LOST]);

    return make_tuple(won, lost, won+lost);
  }

//----------------------------------------------------------------------------

  tuple<int, int> RankingEntry::getPointStats() const
  {
    int won = QVariant2Int_WithDefault(row[RA_POINTS_WON]);
    int lost = QVariant2Int_WithDefault(row[RA_POINTS_LOST]);

    return make_tuple(won, lost);
  }

//----------------------------------------------------------------------------

//----------------------------------------------------------------------------


//----------------------------------------------------------------------------

}
