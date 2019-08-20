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

#ifndef TOURNAMENTERRORCODES_H
#define	TOURNAMENTERRORCODES_H

#include <optional>

#include <Sloppy/BasicException.h>

namespace QTournament
{
    enum class Error {
        OK = 0,
        InvalidName = -10000,
        NameExists,
        NotUsingTeams,
        InvalidSex,
        InvalidTeam,
        CategoryClosedForMorePlayers,
        PlayerNotSuitable,
        PlayerAlreadyInCategory,
        PlayerNotInCategory,
        PlayerNotRemovableFromCategory,
        PlayerAlreadyPaired,
        NoCategoryForPairing,
        CategoryNotConfiguraleAnymore,
        PlayersNotAPair,
        PlayersIdentical,
        InvalidId,
        InvalidReconfig,
        InvalidRound,
        InvalidGroupNum,
        MatchGroupExists,
        NoSuchMatchGroup,
        ConfigAlreadyFrozen,
        UnpairedPlayers,
        InvalidKoConfig,
        CategoryNotYetFrozen,
        CategoryNotUnfreezeable,
        CategoryNeedsNoGroupAssignments,
        InvalidPlayerCount,
        CategoryNeedsNoSeeding,
        MatchGroupNotConfiguraleAnymore,
        MatchNotConfiguraleAnymore,
        PlayerAlreadyAssignedToOtherMatchInTheSameRoundAndCategory,
        CategoryStillConfigurable,
        GroupNumberMismatch,
        WrongState,
        MatchGroupAlreadyClosed,
        MatchGroupEmpty,
        MatchGroupNotUnstageable,
        CourtNumberExists,
        NoMatchAvail,
        NoCourtAvail,
        OnlyManualCourtAvail,
        MatchNotRunnable,
        CourtBusy,
        CourtDisabled,
        NoMatchResultSet,
        InvalidMatchResultForCategorySettings,
        InconsistentMatchResultString,
        MatchNotRunning,
        NoCourtAssigned,
        PlayerNotIdle,
        PlayerNotPlaying,
        RoundNotFinished,
        RankingAlreadyExists,
        MissingRankingEntries,
        InvalidSeedingList,
        InvalidPlayerPair,
        InvalidMatchLink,
        InvalidRank,
        PlayerAlreadyInMatches,
        NotAllPlayersRegistered,
        EPD_NotFound,
        EPD_NotOpened,
        EPD_NotConfigured,
        EPD_CreationFailed,
        EPD_NoSexForPlayerDefined,
        EPD_InvalidDatabaseName,
        FileAlreadyExists,
        FileNotExisting,
        IncompatibleFileFormat,
        DatabaseError,
        MatchNeedsNoReferee,
        MatchNeedsReferee,
        RefereeNotIdle,
        CourtNotDisabled,
        CourtAlreadyUsed,
    };

    //----------------------------------------------------------------------------

    class TournamentException : public Sloppy::BasicException
    {
    public:
      TournamentException(const std::string& sender, const std::string& context, Error errorCode)
        :Sloppy::BasicException{
           "QTournament logic exception", sender, context,
           "Error code = " + std::to_string(static_cast<int>(errorCode))},
         err{errorCode} {}

      Error error() const { return err; }

    private:
      Error err;
    };

    //----------------------------------------------------------------------------

    /** \brief A combination of error code and object for function return values;
     * enforces the invariant that "containing an object" always means "OK" and
     * that "containing an error code other than OK" always means "no object".
     */
    template<typename T>
    class ObjectOrError : public std::optional<T>
    {
    public:
      explicit ObjectOrError(const T& obj)
        :std::optional<T>{obj}, e{Error::OK} {}

      explicit ObjectOrError(const std::optional<T>& obj)
        :std::optional<T>{obj}, e{Error::OK} {}

      explicit ObjectOrError(std::optional<T>&& obj)
        :std::optional<T>(std::forward<std::optional<T>>(obj)), e{Error::OK} {}

      explicit ObjectOrError(T&& obj)
        :std::optional<T>(std::forward<T>(obj)), e{Error::OK} {}

      explicit ObjectOrError(Error errorCode)
        :std::optional<T>{}, e{errorCode}
      {
        if (errorCode == Error::OK)
        {
          throw std::invalid_argument("ErrorOrObject ctor: initialized with OK but without object");
        }
      }

      template<class... Args>
      ObjectOrError(Args&&... args)
        :std::optional<T>{std::in_place, std::forward<Args>(args)...}, e{Error::OK} {}

      constexpr Error err() const noexcept { return e; }

    private:
      Error e;
    };
}

#endif	/* TOURNAMENTERRORCODES_H */

