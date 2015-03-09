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

  unique_ptr<MatchScore> Match::getScore(ERR *err)
  {
    QVariant scoreEntry = row[MA_RESULT];

    if (scoreEntry.isNull())
    {
      if (err != nullptr) *err = NO_MATCH_RESULT_SET;
      return nullptr;
    }

    // we assume that any score that has been written to the database
    // is valid. So we simply parse it from the database string
    // without further validating it against the category settings
    QString scoreString = scoreEntry.toString();
    auto result = MatchScore::fromStringWithoutValidation(scoreString);
    if (result == nullptr)
    {
      // this should never happen
      //
      // but if it does, we clear the invalid database entry
      // and return an error
      row.update(MA_RESULT, QVariant());
      if (err != nullptr) *err = INCONSISTENT_MATCH_RESULT_STRING;
      return nullptr;
    }

    if (err != nullptr) *err = OK;
    return result;
  }

//----------------------------------------------------------------------------

  ERR Match::setScore(const MatchScore &score)
  {

  }

//----------------------------------------------------------------------------

  unique_ptr<Court> Match::getCourt(ERR *err) const
  {
    QVariant courtEntry = row[MA_COURT_REF];
    if (courtEntry.isNull())
    {
      if (err != nullptr) *err = NO_COURT_ASSIGNED;
      return nullptr;
    }

    int courtId = courtEntry.toInt();
    auto result = Tournament::getCourtMngr()->getCourtById(courtId);
    if (err != nullptr) *err = OK;
    return result;
  }

//----------------------------------------------------------------------------

  QList<Player> Match::determineActualPlayers() const
  {
    return Tournament::getPlayerMngr()->determineActualPlayersForMatch(*this);
  }

//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------

}
