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

#ifndef PLAYERPROFILE_H
#define PLAYERPROFILE_H

#include <memory>

#include <QList>

#include "TournamentDB.h"
#include "Match.h"

namespace QTournament
{
  class Player;

  class PlayerProfile
  {
  public:
    PlayerProfile(const Player& _p);

    unique_ptr<Match> getLastPlayedMatch() const;
    unique_ptr<Match> getCurrentMatch() const;
    unique_ptr<Match> getNextMatch() const;

    unique_ptr<Match> getLastUmpireMatch() const;
    unique_ptr<Match> getCurrentUmpireMatch() const;
    unique_ptr<Match> getNextUmpireMatch() const;

    QList<Match> getMatchesAsPlayer() const { return matchesAsPlayer; }
    QList<Match> getMatchesAsUmpire() const { return matchesAsUmpire; }

    int getWalkoverCount() const { return walkoverCount; }
    int getFinishCount() const { return finishCount; }  // includes walkovers
    int getActuallyPlayedCount() const { return (finishCount - walkoverCount); }
    int getScheduledMatchesCount() const { return scheduledCount; }
    int getYetToBePlayedCount() const { return (matchesAsPlayer.length() - finishCount); }
    int getScheduledAndNotFinishedCount() const { return (scheduledCount - finishCount); }
    int getOthersCount() const { return (matchesAsPlayer.length() - scheduledCount); }
    int getUmpireFinishedCount() const { return umpireFinishedCount; }
    int getUmpireScheduledAndNotFinishedCount() const { return (matchesAsUmpire.length() - umpireFinishedCount); }

  protected:
    TournamentDB* db;
    const Player p;

    int lastPlayedMatchId;
    int currentMatchId;
    int nextMatchId;
    int lastUmpireMatchId;
    int currentUmpireMatchId;
    int nextUmpireMatchId;
    int finishCount;
    int walkoverCount;
    int scheduledCount;
    int umpireFinishedCount;

    QList<Match> matchesAsPlayer;
    QList<Match> matchesAsUmpire;

    void initMatchIds();
    void initMatchLists();

    unique_ptr<Match> returnMatchOrNullptr(int maId) const;
  };

}
#endif // PLAYERPROFILE_H
