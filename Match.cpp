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

#include "Match.h"
#include "TournamentDataDefs.h"
#include "TournamentDB.h"
#include "TournamentErrorCodes.h"
#include "MatchMngr.h"
#include "PlayerMngr.h"
#include "CourtMngr.h"
#include "HelperFunc.h"
#include <SqliteOverlay/KeyValueTab.h>

namespace QTournament
{

  Match::Match(const TournamentDB& _db, int rowId)
  :TournamentDatabaseObject(_db, TabMatch, rowId)
  {
  }

//----------------------------------------------------------------------------

  Match::Match(const TournamentDB& _db, const SqliteOverlay::TabRow& _row)
  :TournamentDatabaseObject(_db, _row)
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
    int grpId = row.getInt(MA_GrpRef);
    return MatchGroup{db, grpId};
  }

//----------------------------------------------------------------------------

  bool Match::hasPlayerPair1() const
  {
    return row.getInt2(MA_Pair1Ref).has_value();
  }

//----------------------------------------------------------------------------

  bool Match::hasPlayerPair2() const
  {
    return row.getInt2(MA_Pair2Ref).has_value();
  }

//----------------------------------------------------------------------------

  bool Match::hasBothPlayerPairs() const
  {
    return (hasPlayerPair1() && hasPlayerPair2());
  }

//----------------------------------------------------------------------------

  PlayerPair Match::getPlayerPair1() const
  {
    auto ppId = row.getInt2(MA_Pair1Ref);

    if (!ppId)
    {
      throw std::runtime_error("Invalid request for PlayerPair1 of a match");
    }

    PlayerMngr pm{db};
    return pm.getPlayerPair(*ppId);
  }

//----------------------------------------------------------------------------

  PlayerPair Match::getPlayerPair2() const
  {
    auto ppId = row.getInt2(MA_Pair2Ref);

    if (!ppId)
    {
      throw std::runtime_error("Invalid request for PlayerPair2 of a match");
    }

    PlayerMngr pm{db};
    return pm.getPlayerPair(*ppId);
  }

//----------------------------------------------------------------------------

  int Match::getMatchNumber() const
  {
    return row.getInt2(MA_Num).value_or(MatchNumNotAssigned);
  }

  //----------------------------------------------------------------------------

  void Match::getDisplayNameTextItems(const QString& localWinnerName, const QString& localLoserName,
                                      QString& row1Left_out, QString& row2Left_out,
                                      QString& row1Right_out, QString& row2Right_out) const
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
      row1Right_out = (symName2 > 0) ? localWinnerName : localLoserName;
      row1Right_out += " #" + QString::number(abs(symName2));
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
    getDisplayNameTextItems(localWinnerName, localLoserName, row1Left, row2Left, row1Right, row2Right);

    QString name1 = row2Left.isEmpty() ? row1Left : row1Left + " / " + row2Left;
    QString name2 = row2Right.isEmpty() ? row1Right : row1Right + " / " + row2Right;

    return name1 + " : " + name2;
  }

//----------------------------------------------------------------------------

  std::optional<MatchScore> Match::getScore(Error *err) const
  {
    auto scoreEntry = row.getString2(MA_Result);

    if (!scoreEntry)
    {
      Sloppy::assignIfNotNull<Error>(err, Error::NoMatchResultSet);
      return {};
    }

    // we assume that any score that has been written to the database
    // is valid. So we simply parse it from the database string
    // without further validating it against the category settings
    QString scoreString = stdString2QString(*scoreEntry);
    auto result = MatchScore::fromStringWithoutValidation(scoreString);
    if (!result)
    {
      // this should never happen
      //
      // but if it does, we clear the invalid database entry
      // and return an error
      row.updateToNull(MA_Result);
      Sloppy::assignIfNotNull<Error>(err, Error::InconsistentMatchResultString);
      return {};
    }

    Sloppy::assignIfNotNull<Error>(err, Error::OK);
    return result;
  }

//----------------------------------------------------------------------------


//----------------------------------------------------------------------------

  std::optional<Court> Match::getCourt(Error *err) const
  {
    auto courtId = row.getInt2(MA_CourtRef);
    if (!courtId)
    {
      Sloppy::assignIfNotNull<Error>(err, Error::NoCourtAssigned);
      return {};
    }

    CourtMngr cm{db};
    auto result = cm.getCourtById(*courtId);
    Sloppy::assignIfNotNull<Error>(err, Error::OK);

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
    auto _wr = row.getInt2(MA_WinnerRank);
    if (!_wr) return -1;

    int wr = *_wr;
    return (wr < 1) ? -1 : wr;
  }

//----------------------------------------------------------------------------

  int Match::getLoserRank() const
  {
    auto _lr = row.getInt2(MA_LoserRank);
    if (!_lr) return -1;

    int lr = *_lr;
    return (lr < 1) ? -1 : lr;
  }

//----------------------------------------------------------------------------

  bool Match::isWalkoverPossible() const
  {
    ObjState stat = getState();
    return ((stat == ObjState::MA_Ready) || (stat == ObjState::MA_Running) || (stat == ObjState::MA_Busy) || (stat == ObjState::MA_Waiting));
  }

//----------------------------------------------------------------------------

  bool Match::isWonByWalkover() const
  {
    ObjState stat = getState();
    if (stat != ObjState::MA_Finished) return false;

    // if the match is finished but has no starting time, it
    // has been won by a walkover
    //
    // 2019-08-19: FIX, the comment above doesn't seem to fit
    // to the code below
    QDateTime fTime = getFinishTime();
    return (!(fTime.isValid()));
  }

//----------------------------------------------------------------------------

  QDateTime Match::getStartTime() const
  {
    auto startTime = row.getInt2(MA_StartTime);
    if (!startTime) return QDateTime();   // return null-time as error indicator

    return QDateTime::fromTime_t(*startTime); // Hmmm... conversion from int to uint... should work until 2035 or something
  }

//----------------------------------------------------------------------------

  QDateTime Match::getFinishTime() const
  {
    auto finishTime = row.getInt2(MA_FinishTime);
    if (!finishTime) return QDateTime();   // return null-time as error indicator

    return QDateTime::fromTime_t(*finishTime);  // Hmmm... conversion from int to uint... should work until 2035 or something
  }

//----------------------------------------------------------------------------

  void Match::addAddtionalCallTime() const
  {
    std::string callTimes{};
    auto _callTimes = row.getString2(MA_AdditionalCallTimes);
    if (_callTimes)
    {
      callTimes = *_callTimes + ",";
    }

    UTCTimestamp now;
    callTimes += std::to_string(now.getRawTime());

    row.update(MA_AdditionalCallTimes, callTimes);
  }

//----------------------------------------------------------------------------

  QList<QDateTime> Match::getAdditionalCallTimes() const
  {
    QList<QDateTime> result;

    auto _callTimes = row.getString2(MA_AdditionalCallTimes);
    if (!_callTimes)
    {
      return result;
    }
    QString allTimes = stdString2QString(*_callTimes);
    QStringList sCallTimes = allTimes.split(",");

    for (const QString& sCallTime : sCallTimes)
    {
      uint epochSecs = sCallTime.toUInt();
      result.append(QDateTime::fromTime_t(epochSecs));
    }

    return result;
  }

  //----------------------------------------------------------------------------

  int Match::getMatchDuration() const
  {
    ObjState stat = getState();
    if ((stat != ObjState::MA_Finished) && (stat != ObjState::MA_Running)) return -1;

    auto startTime = row.getInt2(MA_StartTime);
    if (!startTime) return -1;

    int finishTime;
    if (stat == ObjState::MA_Finished)
    {
      auto _finishTime = row.getInt2(MA_FinishTime);
      if (!_finishTime) return -1;
      finishTime = *_finishTime;
    } else {
      finishTime = QDateTime::currentDateTimeUtc().toTime_t();
    }

    return finishTime - *startTime;
  }

  //----------------------------------------------------------------------------

  RefereeMode Match::get_RAW_RefereeMode() const
  {
    int modeId = row.getInt(MA_RefereeMode);
    return static_cast<RefereeMode>(modeId);
  }

  //----------------------------------------------------------------------------

  RefereeMode Match::get_EFFECTIVE_RefereeMode() const
  {
    RefereeMode mode = get_RAW_RefereeMode();
    if (mode == RefereeMode::UseDefault)
    {
      auto cfg = SqliteOverlay::KeyValueTab{db.get(), TabCfg};
      int tnmtDefaultRefereeModeId = cfg.getInt(CfgKey_DefaultRefereemode);
      mode = static_cast<RefereeMode>(tnmtDefaultRefereeModeId);
    }

    assert(mode != RefereeMode::UseDefault);

    return mode;
  }

  //----------------------------------------------------------------------------

  std::optional<Player> Match::getAssignedReferee() const
  {
    auto refereeId = row.getInt2(MA_RefereeRef);
    if (!refereeId) return {};

    PlayerMngr pm{db};
    return pm.getPlayer2(*refereeId);
  }

  //----------------------------------------------------------------------------

  bool Match::hasRefereeAssigned() const
  {
    return row.getInt2(MA_RefereeRef).has_value();
  }

  //----------------------------------------------------------------------------

  Error Match::canAssignReferee(RefereeAction refAction) const
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
    ObjState stat = getState();
    if ((refAction == RefereeAction::PreAssign) || (refAction == RefereeAction::MatchCall))
    {
      if (!((stat == ObjState::MA_Busy) || (stat == ObjState::MA_Ready)))
      {
        return Error::MatchNotConfiguraleAnymore;
      }
    }
    else if (refAction == RefereeAction::Swap)
    {
      if (!((stat == ObjState::MA_Running) && (hasRefereeAssigned() == true)))
      {
        return Error::MatchNotConfiguraleAnymore;
      }
    } else {
      // default
      return Error::MatchNotConfiguraleAnymore;
    }

    // don't allow assignments if the mode is set to RefereeMode::None
    // or to RefereeMode::HandWritten
    RefereeMode mod = get_EFFECTIVE_RefereeMode();
    if ((mod == RefereeMode::None) || (mod == RefereeMode::HandWritten))
    {
      return Error::MatchNeedsNoReferee;
    }

    return Error::OK;
  }

  //----------------------------------------------------------------------------

  std::optional<BracketMatchNumber> Match::bracketMatchNum() const
  {
    auto brNum = row.getInt2(MA_BracketMatchNum);
    return (brNum.has_value()) ? BracketMatchNumber{brNum.value()} : std::optional<BracketMatchNumber>{};
  }

  //----------------------------------------------------------------------------

  int Match::getSymbolicPlayerPairName(int playerPos) const
  {
    // if we have a regular PP, don't return a symbolic name
    if ((playerPos == 1) && hasPlayerPair1()) return 0;
    if ((playerPos == 2) && hasPlayerPair2()) return 0;

    // check if we have a symbolic name
    auto symName = (playerPos == 1) ? row.getInt2(MA_Pair1SymbolicVal) : row.getInt2(MA_Pair2SymbolicVal);
    if (!symName) return 0;

    // okay, there is a symbolic name
    int matchRef = *symName;
    if (matchRef == 0) return 0;

    bool isWinner = matchRef > 0;
    if (matchRef < 0) matchRef = -matchRef;

    MatchMngr mm{db};
    auto ma = mm.getMatch(matchRef);
    int matchNumber = ma->getMatchNumber();
    if (matchNumber == MatchNumNotAssigned) return 0;

    return isWinner ? matchNumber : -matchNumber;
  }

//----------------------------------------------------------------------------

  std::optional<PlayerPair> Match::getWinner() const
  {
    auto score = getScore();
    if (!score)
    {
      return {};   // score is not yet set
    }

    int winner = score->getWinner();
    if (winner == 0)
    {
      return {};   // draw; no winner
    }

    PlayerPair w = (winner == 1) ? getPlayerPair1() : getPlayerPair2();

    return PlayerPair(w);
  }

//----------------------------------------------------------------------------

  std::optional<PlayerPair> Match::getLoser() const
  {
    auto score = getScore();
    if (!score)
    {
      return {};   // score is not yet set
    }

    int loser = score->getLoser();
    if (loser == 0)
    {
      return {};   // draw; no loser
    }

    PlayerPair l = (loser == 1) ? getPlayerPair1() : getPlayerPair2();

    return PlayerPair(l);
  }

//----------------------------------------------------------------------------

}
