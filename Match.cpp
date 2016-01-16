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

#include "Match.h"
#include "TournamentDataDefs.h"
#include "TournamentDB.h"
#include "TournamentErrorCodes.h"
#include "MatchMngr.h"
#include "Tournament.h"

namespace QTournament
{

  Match::Match(TournamentDB* db, int rowId)
  :TournamentDatabaseObject(db, TAB_MATCH, rowId)
  {
  }

//----------------------------------------------------------------------------

  Match::Match(TournamentDB* db, SqliteOverlay::TabRow row)
  :TournamentDatabaseObject(db, row)
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
    int grpId = row.getInt(MA_GRP_REF);
    return MatchGroup{db, grpId};
  }

//----------------------------------------------------------------------------

  bool Match::hasPlayerPair1() const
  {
    auto ppId = row.getInt2(MA_PAIR1_REF);
    if (ppId->isNull()) return false;
    return true;
  }

//----------------------------------------------------------------------------

  bool Match::hasPlayerPair2() const
  {
    auto ppId = row.getInt2(MA_PAIR2_REF);
    if (ppId->isNull()) return false;
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

    int ppId = row.getInt(MA_PAIR1_REF);
    auto tnmt = Tournament::getActiveTournament();
    return tnmt->getPlayerMngr()->getPlayerPair(ppId);
  }

//----------------------------------------------------------------------------

  PlayerPair Match::getPlayerPair2() const
  {
    if (!(hasPlayerPair2()))
    {
      throw runtime_error("Invalid request for PlayerPair2 of a match");
    }

    int ppId = row.getInt(MA_PAIR2_REF);
    auto tnmt = Tournament::getActiveTournament();
    return tnmt->getPlayerMngr()->getPlayerPair(ppId);
  }

//----------------------------------------------------------------------------

  int Match::getMatchNumber() const
  {
    auto num = row.getInt2(MA_NUM);
    if (num->isNull()) return MATCH_NUM_NOT_ASSIGNED;
    return num->get();
  }

//----------------------------------------------------------------------------

  QString Match::getDisplayName(const QString& localWinnerName, const QString& localLoserName) const
  {
    QString name1 = "??";
    if (hasPlayerPair1())
    {
      name1 = getPlayerPair1().getDisplayName();
    }

    // maybe, the name of player pair 1 is only symbolic
    // as of now
    int symName1 = getSymbolicPlayerPair1Name();
    if (symName1 != 0)
    {
      name1 = (symName1 > 0) ? localWinnerName : localLoserName;
      name1 += " #" + QString::number(abs(symName1));
    }

    QString name2 = "??";
    if (hasPlayerPair2())
    {
      name2 = getPlayerPair2().getDisplayName();
    }

    // maybe, the name of player pair 1 is only symbolic
    // as of now
    int symName2 = getSymbolicPlayerPair2Name();
    if (symName2 != 0)
    {
      name2 = (symName2 > 0) ? localWinnerName : localLoserName;
      name2 += " #" + QString::number(abs(symName2));
    }

    return name1 + " : " + name2;
  }

//----------------------------------------------------------------------------

  unique_ptr<MatchScore> Match::getScore(ERR *err) const
  {
    auto scoreEntry = row.getString2(MA_RESULT);

    if (scoreEntry->isNull())
    {
      if (err != nullptr) *err = NO_MATCH_RESULT_SET;
      return nullptr;
    }

    // we assume that any score that has been written to the database
    // is valid. So we simply parse it from the database string
    // without further validating it against the category settings
    QString scoreString = QString::fromUtf8(scoreEntry->get().data());
    auto result = MatchScore::fromStringWithoutValidation(scoreString);
    if (result == nullptr)
    {
      // this should never happen
      //
      // but if it does, we clear the invalid database entry
      // and return an error
      row.updateToNull(MA_RESULT);
      if (err != nullptr) *err = INCONSISTENT_MATCH_RESULT_STRING;
      return nullptr;
    }

    if (err != nullptr) *err = OK;
    return result;
  }

//----------------------------------------------------------------------------


//----------------------------------------------------------------------------

  unique_ptr<Court> Match::getCourt(ERR *err) const
  {
    auto courtEntry = row.getInt2(MA_COURT_REF);
    if (courtEntry->isNull())
    {
      if (err != nullptr) *err = NO_COURT_ASSIGNED;
      return nullptr;
    }

    int courtId = courtEntry->get();
    auto tnmt = Tournament::getActiveTournament();
    auto result = tnmt->getCourtMngr()->getCourtById(courtId);
    if (err != nullptr) *err = OK;
    return result;
  }

//----------------------------------------------------------------------------

  QList<Player> Match::determineActualPlayers() const
  {
    auto tnmt = Tournament::getActiveTournament();
    return tnmt->getPlayerMngr()->determineActualPlayersForMatch(*this);
  }

//----------------------------------------------------------------------------

  int Match::getSymbolicPlayerPair1Name() const
  {
    return getSymbolicPlayerPairName(1);
  }

  int Match::getSymbolicPlayerPair2Name() const
  {
    return getSymbolicPlayerPairName(2);
  }

//----------------------------------------------------------------------------

  int Match::getWinnerRank() const
  {
    auto _wr = row.getInt2(MA_WINNER_RANK);
    if (_wr->isNull()) return -1;

    int wr = _wr->get();
    return (wr < 1) ? -1 : wr;
  }

//----------------------------------------------------------------------------

  int Match::getLoserRank() const
  {
    auto _lr = row.getInt2(MA_LOSER_RANK);
    if (_lr->isNull()) return -1;

    int lr = _lr->get();
    return (lr < 1) ? -1 : lr;
  }

//----------------------------------------------------------------------------

  bool Match::isWalkoverPossible() const
  {
    OBJ_STATE stat = getState();
    return ((stat == STAT_MA_READY) || (stat == STAT_MA_RUNNING) || (stat == STAT_MA_BUSY) || (stat == STAT_MA_WAITING));
  }

//----------------------------------------------------------------------------

  QDateTime Match::getStartTime() const
  {
    auto startTime = row.getInt2(MA_START_TIME);
    if (startTime->isNull()) return QDateTime();   // return null-time as error indicator
    uint epochSecs = startTime->get();   // Hmmm... conversion from int to unit... should work until 2035 or something

    return QDateTime::fromTime_t(epochSecs);
  }

//----------------------------------------------------------------------------

  bool Match::addAddtionalCallTime() const
  {
    QDateTime curDateTime = QDateTime::currentDateTimeUtc();
    uint epochSecs = curDateTime.toTime_t();
    QString sEpochSecs = QString::number(epochSecs);

    QString callTimes = "";
    auto _callTimes = row.getString2(MA_ADDITIONAL_CALL_TIMES);
    if (!(_callTimes->isNull()))
    {
      callTimes = QString::fromUtf8(_callTimes->get().data()) + ",";
    }
    callTimes += sEpochSecs;

    // we have a limit of 50 chars for this CSV-string
    if (callTimes.length() <= 50)
    {
      row.update(MA_ADDITIONAL_CALL_TIMES, callTimes.toUtf8().constData());
      return true;
    }

    return false;
  }

//----------------------------------------------------------------------------

  QList<QDateTime> Match::getAdditionalCallTimes() const
  {
    QList<QDateTime> result;

    auto _callTimes = row.getString2(MA_ADDITIONAL_CALL_TIMES);
    if (_callTimes->isNull())
    {
      return result;
    }
    QString allTimes = QString::fromUtf8(_callTimes->get().data());
    QStringList sCallTimes = allTimes.split(",");

    for (QString sCallTime : sCallTimes)
    {
      uint epochSecs = sCallTime.toUInt();
      result.append(QDateTime::fromTime_t(epochSecs));
    }

    return result;
  }

  //----------------------------------------------------------------------------

  int Match::getMatchDuration() const
  {
    if (getState() != STAT_MA_FINISHED) return -1;

    auto _startTime = row.getInt2(MA_START_TIME);
    if (_startTime->isNull()) return -1;
    int startTime = _startTime->get();

    auto _finishTime = row.getInt2(MA_FINISH_TIME);
    if (_finishTime->isNull()) return -1;
    int finishedTime = _finishTime->get();

    return finishedTime - startTime;
  }

//----------------------------------------------------------------------------

  int Match::getSymbolicPlayerPairName(int playerPos) const
  {
    // if we have a regular PP, don't return a symbolic name
    if ((playerPos == 1) && hasPlayerPair1()) return 0;
    if ((playerPos == 2) && hasPlayerPair2()) return 0;

    // check if we have a symbolic name
    auto symName = (playerPos == 1) ? row.getInt2(MA_PAIR1_SYMBOLIC_VAL) : row.getInt2(MA_PAIR2_SYMBOLIC_VAL);
    if (symName->isNull()) return 0;

    // okay, there is a symbolic name
    int matchRef = symName->get();
    if (matchRef == 0) return 0;

    bool isWinner = matchRef > 0;
    if (matchRef < 0) matchRef = -matchRef;

    auto tnmt = Tournament::getActiveTournament();
    auto ma = tnmt->getMatchMngr()->getMatch(matchRef);
    int matchNumber = ma->getMatchNumber();
    if (matchNumber == MATCH_NUM_NOT_ASSIGNED) return 0;

    return isWinner ? matchNumber : -matchNumber;
  }

//----------------------------------------------------------------------------

  unique_ptr<PlayerPair> Match::getWinner() const
  {
    unique_ptr<MatchScore> score = getScore();
    if (score == nullptr)
    {
      return nullptr;   // score is not yet set
    }

    int winner = score->getWinner();
    if (winner == 0)
    {
      return nullptr;   // draw; no winner
    }

    PlayerPair w = (winner == 1) ? getPlayerPair1() : getPlayerPair2();

    return unique_ptr<PlayerPair>(new PlayerPair(w));
  }

//----------------------------------------------------------------------------

  unique_ptr<PlayerPair> Match::getLoser() const
  {
    unique_ptr<MatchScore> score = getScore();
    if (score == nullptr)
    {
      return nullptr;   // score is not yet set
    }

    int loser = score->getLoser();
    if (loser == 0)
    {
      return nullptr;   // draw; no loser
    }

    PlayerPair l = (loser == 1) ? getPlayerPair1() : getPlayerPair2();

    return unique_ptr<PlayerPair>(new PlayerPair(l));
  }

//----------------------------------------------------------------------------

}
