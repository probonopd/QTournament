/* 
 * File:   Team.cpp
 * Author: volker
 * 
 * Created on March 2, 2014, 6:13 PM
 */

#include <stdexcept>

#include "Match.h"
#include "TournamentDataDefs.h"
#include "TournamentDB.h"
#include "TournamentErrorCodes.h"
#include "MatchMngr.h"
#include "Tournament.h"

namespace QTournament
{

  Match::Match(TournamentDB* db, int rowId)
  :GenericDatabaseObject(db, TAB_MATCH, rowId)
  {
  }

//----------------------------------------------------------------------------

  Match::Match(TournamentDB* db, dbOverlay::TabRow row)
  :GenericDatabaseObject(db, row)
  {
  }

//----------------------------------------------------------------------------

  Category Match::getCategory() const
  {
    return getMatchGroup().getCategory();
  }

//----------------------------------------------------------------------------

  MatchGroup Match::getMatchGroup() const
  {
    int grpId = row[MA_GRP_REF].toInt();
    return MatchGroup{db, grpId};
  }

//----------------------------------------------------------------------------

  bool Match::hasPlayerPair1() const
  {
    QVariant ppId = row[MA_PAIR1_REF];
    if (ppId.isNull()) return false;
    return true;
  }

//----------------------------------------------------------------------------

  bool Match::hasPlayerPair2() const
  {
    QVariant ppId = row[MA_PAIR2_REF];
    if (ppId.isNull()) return false;
    return true;
  }

//----------------------------------------------------------------------------

  bool Match::hasBothPlayerPairs() const
  {
    return (hasPlayerPair1() && hasPlayerPair2());
  }

//----------------------------------------------------------------------------

  PlayerPair Match::getPlayerPair1() const
  {
    if (!(hasPlayerPair1()))
    {
      throw runtime_error("Invalid request for PlayerPair1 of a match");
    }

    int ppId = row[MA_PAIR1_REF].toInt();
    return Tournament::getPlayerMngr()->getPlayerPair(ppId);
  }

//----------------------------------------------------------------------------

  PlayerPair Match::getPlayerPair2() const
  {
    if (!(hasPlayerPair2()))
    {
      throw runtime_error("Invalid request for PlayerPair2 of a match");
    }

    int ppId = row[MA_PAIR2_REF].toInt();
    return Tournament::getPlayerMngr()->getPlayerPair(ppId);
  }

//----------------------------------------------------------------------------

  int Match::getMatchNumber() const
  {
    QVariant num = row[MA_NUM];
    if (num.isNull()) return MATCH_NUM_NOT_ASSIGNED;
    return num.toInt();
  }

//----------------------------------------------------------------------------

  bool Match::hasAllPlayersIdle() const
  {
    if (!(hasBothPlayerPairs())) return false;

    auto pp = getPlayerPair1();
    if (!(pp.areAllPlayersIdle())) return false;
    pp = getPlayerPair2();
    if (!(pp.areAllPlayersIdle())) return false;
    return true;
  }

//----------------------------------------------------------------------------

  QString Match::getDisplayName() const
  {
    QString name1 = "??";
    if (hasPlayerPair1())
    {
      name1 = getPlayerPair1().getDisplayName();
    }

    QString name2 = "??";
    if (hasPlayerPair2())
    {
      name2 = getPlayerPair2().getDisplayName();
    }

    return name1 + " : " + name2;
  }

//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------

}
