/* 
 * File:   Team.cpp
 * Author: volker
 * 
 * Created on March 2, 2014, 6:13 PM
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
  :GenericDatabaseObject(db, TAB_RANKING, rowId)
  {
  }

//----------------------------------------------------------------------------

  RankingEntry::RankingEntry(TournamentDB* db, dbOverlay::TabRow row)
  :GenericDatabaseObject(db, row)
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
    return Tournament::getCatMngr()->getCategoryById(catId);
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
