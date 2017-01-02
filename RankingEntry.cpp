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

#include <stdexcept>

#include <SqliteOverlay/ClausesAndQueries.h>

#include "RankingEntry.h"
#include "TournamentDataDefs.h"
#include "TournamentDB.h"
#include "TournamentErrorCodes.h"
#include "CatMngr.h"

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
    return row.getInt(RA_ROUND);
  }

//----------------------------------------------------------------------------

  Category RankingEntry::getCategory() const
  {
    int catId = row.getInt(RA_CAT_REF);
    CatMngr cm{db};
    return cm.getCategoryById(catId);
  }

//----------------------------------------------------------------------------

  unique_ptr<PlayerPair> RankingEntry::getPlayerPair() const
  {
    auto _ppId = row.getInt2(RA_PAIR_REF);
    if (_ppId->isNull()) return nullptr;

    int ppId = _ppId->get();
    return unique_ptr<PlayerPair>(new PlayerPair(db, ppId));
  }

//----------------------------------------------------------------------------

  int RankingEntry::getRank() const
  {
    int rank = ScalarQueryResult2Int_WithDefault(row.getInt2(RA_RANK), -1);
    if (rank <= 0) return NO_RANK_ASSIGNED;

    return rank;
  }

//----------------------------------------------------------------------------

  int RankingEntry::getGroupNumber() const
  {
    return row.getInt(RA_GRP_NUM);
  }

//----------------------------------------------------------------------------

  tuple<int, int, int, int> RankingEntry::getMatchStats() const
  {
    int won = ScalarQueryResult2Int_WithDefault(row.getInt2(RA_MATCHES_WON));
    int draw = ScalarQueryResult2Int_WithDefault(row.getInt2(RA_MATCHES_DRAW));
    int lost = ScalarQueryResult2Int_WithDefault(row.getInt2(RA_MATCHES_LOST));

    return make_tuple(won, draw, lost, won+draw+lost);
  }

//----------------------------------------------------------------------------

  int RankingEntry::ScalarQueryResult2Int_WithDefault(const unique_ptr<SqliteOverlay::ScalarQueryResult<int>> &v, int defaultVal) const
  {
    if (v->isNull())
    {
      return defaultVal;
    }

    return v->get();
  }

//----------------------------------------------------------------------------

  tuple<int, int, int> RankingEntry::getGameStats() const
  {
    int won = ScalarQueryResult2Int_WithDefault(row.getInt2(RA_GAMES_WON));
    int lost = ScalarQueryResult2Int_WithDefault(row.getInt2(RA_GAMES_LOST));

    return make_tuple(won, lost, won+lost);
  }

//----------------------------------------------------------------------------

  tuple<int, int> RankingEntry::getPointStats() const
  {
    int won = ScalarQueryResult2Int_WithDefault(row.getInt2(RA_POINTS_WON));
    int lost = ScalarQueryResult2Int_WithDefault(row.getInt2(RA_POINTS_LOST));

    return make_tuple(won, lost);
  }

//----------------------------------------------------------------------------

//----------------------------------------------------------------------------


//----------------------------------------------------------------------------

}
