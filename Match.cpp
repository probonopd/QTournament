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

#include <stdexcept>

#include "Match.h"
#include "TournamentDataDefs.h"
#include "TournamentDB.h"
#include "TournamentErrorCodes.h"
#include "MatchMngr.h"
#include "PlayerMngr.h"
#include "CourtMngr.h"
#include "KeyValueTab.h"

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
    PlayerMngr pm{db};
    return pm.getPlayerPair(ppId);
  }

//----------------------------------------------------------------------------

  PlayerPair Match::getPlayerPair2() const
  {
    if (!(hasPlayerPair2()))
    {
      throw runtime_error("Invalid request for PlayerPair2 of a match");
    }

    int ppId = row.getInt(MA_PAIR2_REF);
    PlayerMngr pm{db};
    return pm.getPlayerPair(ppId);
  }

//----------------------------------------------------------------------------

  int Match::getMatchNumber() const
  {
    auto num = row.getInt2(MA_NUM);
    if (num->isNull()) return MATCH_NUM_NOT_ASSIGNED;
    return num->get();
  }

  //----------------------------------------------------------------------------

  void Match::getDisplayNameTextItems(const QString& localWinnerName, const QString& localLoserName,
                                      QString& row1Left_out, QString& row2Left_out,
                                      QString& row1Right_out, QString& row2Right_out,
                                      bool& isDoubles) const
  {
    //
    // get the text items to be drawn for a match. The overall layout is as follows
    //
    //               row1Left : row1Right
    //               row2Left   row2Right
    //
    row1Left_out = "??";
    row2Left_out = "";
    if (hasPlayerPair1())
    {
      QTournament::PlayerPair pp = getPlayerPair1();
      row1Left_out = pp.getPlayer1().getDisplayName();
      if (pp.hasPlayer2())
      {
        row2Left_out = pp.getPlayer2().getDisplayName();
      }
    }

    // maybe, the name of player pair 1 is only symbolic
    // as of now
    int symName1 = getSymbolicPlayerPair1Name();
    if (symName1 != 0)
    {
      row1Left_out = (symName1 > 0) ? localWinnerName : localLoserName;
      row1Left_out += " #" + QString::number(abs(symName1));
      row2Left_out.clear();
    }

    //
    // do the same for player pair 2
    //
    row1Right_out = "??";
    row2Right_out = "";
    if (hasPlayerPair2())
    {
      QTournament::PlayerPair pp = getPlayerPair2();
      row1Right_out = pp.getPlayer1().getDisplayName();
      if (pp.hasPlayer2())
      {
        row2Right_out = pp.getPlayer2().getDisplayName();
      }
    }
    int symName2 = getSymbolicPlayerPair2Name();
    if (symName2 != 0)
    {
      row1Right_out = (symName1 > 0) ? localWinnerName : localLoserName;
      row1Right_out += " #" + QString::number(abs(symName1));
      row2Right_out.clear();
    }
  }

//----------------------------------------------------------------------------

  QString Match::getDisplayName(const QString& localWinnerName, const QString& localLoserName) const
  {
    QString row1Left;
    QString row2Left;
    QString row1Right;
    QString row2Right;
    bool isDoubles;
    getDisplayNameTextItems(localWinnerName, localLoserName, row1Left, row2Left, row1Right, row2Right, isDoubles);

    QString name1 = row2Left.isEmpty() ? row1Left : row1Left + " / " + row2Left;
    QString name2 = row2Right.isEmpty() ? row1Right : row1Right + " / " + row2Right;

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
    CourtMngr cm{db};
    auto result = cm.getCourtById(courtId);
    if (err != nullptr) *err = OK;
    return result;
  }

//----------------------------------------------------------------------------

  PlayerList Match::determineActualPlayers() const
  {
    PlayerMngr pm{db};
    return pm.determineActualPlayersForMatch(*this);
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

  QDateTime Match::getFinishTime() const
  {
    auto finishTime = row.getInt2(MA_FINISH_TIME);
    if (finishTime->isNull()) return QDateTime();   // return null-time as error indicator
    uint epochSecs = finishTime->get();   // Hmmm... conversion from int to unit... should work until 2035 or something

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
    OBJ_STATE stat = getState();
    if ((stat != STAT_MA_FINISHED) && (stat != STAT_MA_RUNNING)) return -1;

    auto _startTime = row.getInt2(MA_START_TIME);
    if (_startTime->isNull()) return -1;
    int startTime = _startTime->get();

    int finishTime;
    if (stat == STAT_MA_FINISHED)
    {
      auto _finishTime = row.getInt2(MA_FINISH_TIME);
      if (_finishTime->isNull()) return -1;
      finishTime = _finishTime->get();
    } else {
      finishTime = QDateTime::currentDateTimeUtc().toTime_t();
    }

    return finishTime - startTime;
  }

  //----------------------------------------------------------------------------

  REFEREE_MODE Match::get_RAW_RefereeMode() const
  {
    int modeId = row.getInt(MA_REFEREE_MODE);
    return static_cast<REFEREE_MODE>(modeId);
  }

  //----------------------------------------------------------------------------

  REFEREE_MODE Match::get_EFFECTIVE_RefereeMode() const
  {
    REFEREE_MODE mode = get_RAW_RefereeMode();
    if (mode == REFEREE_MODE::USE_DEFAULT)
    {
      auto cfg = KeyValueTab::getTab(db, TAB_CFG, false);
      int tnmtDefaultRefereeModeId = cfg->getInt(CFG_KEY_DEFAULT_REFEREE_MODE);
      mode = static_cast<REFEREE_MODE>(tnmtDefaultRefereeModeId);
    }

    assert(mode != REFEREE_MODE::USE_DEFAULT);

    return mode;
  }

  //----------------------------------------------------------------------------

  upPlayer Match::getAssignedReferee() const
  {
    auto _refereeId = row.getInt2(MA_REFEREE_REF);
    if (_refereeId->isNull()) return nullptr;

    PlayerMngr pm{db};
    return pm.getPlayer_up(_refereeId->get());
  }

  //----------------------------------------------------------------------------

  bool Match::hasRefereeAssigned() const
  {
    auto _refereeId = row.getInt2(MA_REFEREE_REF);
    return (_refereeId->isNull() == false);
  }

  //----------------------------------------------------------------------------

  ERR Match::canAssignReferee(REFEREE_ACTION refAction) const
  {
    // only allow changes to the referee assignment
    // if the match is fully defined (all player names determined) and
    // if the match can be called now or later
    //
    // we also allow assignments if the match is running and has already
    // a referee assigned. This is for swapping a referee in mid-game
    //
    // ==> match must be (READY) or (BUSY) or (RUNNING and hasRefereeAssigned is true)
    //
    OBJ_STATE stat = getState();
    if ((refAction == REFEREE_ACTION::PRE_ASSIGN) || (refAction == REFEREE_ACTION::MATCH_CALL))
    {
      if (!((stat == STAT_MA_BUSY) || (stat == STAT_MA_READY)))
      {
        return MATCH_NOT_CONFIGURALE_ANYMORE;
      }
    }
    else if (refAction == REFEREE_ACTION::SWAP)
    {
      if (!((stat == STAT_MA_RUNNING) && (hasRefereeAssigned() == true)))
      {
        return MATCH_NOT_CONFIGURALE_ANYMORE;
      }
    } else {
      // default
      return MATCH_NOT_CONFIGURALE_ANYMORE;
    }

    // don't allow assignments if the mode is set to NONE
    // or to HANDWRITTEN
    REFEREE_MODE mod = get_EFFECTIVE_RefereeMode();
    if ((mod == REFEREE_MODE::NONE) || (mod == REFEREE_MODE::HANDWRITTEN))
    {
      return MATCH_NEEDS_NO_REFEREE;
    }

    return OK;
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

    MatchMngr mm{db};
    auto ma = mm.getMatch(matchRef);
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
