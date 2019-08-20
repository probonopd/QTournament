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

#ifndef CATEGORY_H
#define	CATEGORY_H

#include <memory>
#include <functional>
#include <vector>
#include <type_traits>

#include <QVariant>

#include "TournamentDatabaseObject.h"
#include "TournamentDB.h"
#include <SqliteOverlay/TabRow.h>
#include "Player.h"
#include "PlayerPair.h"
#include "TournamentErrorCodes.h"
#include "KO_Config.h"

namespace QTournament
{
  class CatRoundStatus;
  class RankingEntry;
  class Match;
  class MatchScore;

  enum class ModMatchResult
  {
    NotImplemented,
    NotPossible,
    ScoreOnly,
    WinnerLoser,
    ModDone
  };

  class Category : public TournamentDatabaseObject
  {
    friend class CatMngr;
    friend class RoundRobinCategory;
    friend class EliminationCategory;
    friend class PureRoundRobinCategory;
    friend class SwissLadderCategory;
    friend class TournamentDatabaseObjectManager;
    friend class SqliteOverlay::GenericObjectManager<TournamentDB>;

  public:
    // getters
    QString getName() const;
    MatchType getMatchType() const;
    MatchSystem getMatchSystem() const;
    Sex getSex() const;
    CatAddState getAddState(const Sex s) const;
    CatAddState getAddState(const Player& p) const;
    QVariant getParameter(CatParameter) const;
    int getParameter_int(CatParameter) const;
    bool getParameter_bool(CatParameter) const;
    QString getParameter_string(CatParameter) const;
    PlayerPairList getPlayerPairs(int grp = GroupNum_NotAssigned) const;
    int getDatabasePlayerPairCount(int grp = GroupNum_NotAssigned) const;
    PlayerList getAllPlayersInCategory() const;
    Player getPartner(const Player& p) const;
    std::unique_ptr<Category> convertToSpecializedObject() const;
    int getGroupNumForPredecessorRound(const int grpNum) const;
    CatRoundStatus getRoundStatus() const;
    PlayerPairList getEliminatedPlayersAfterRound(int round, ERR *err) const;
    int getMaxNumGamesInRound(int round) const;
    QString getBracketVisDataString() const;

    // setters
    ERR setMatchType(MatchType t);
    ERR setMatchSystem(MatchSystem s);
    ERR setSex(Sex s);
    bool setParameter(CatParameter p, const QVariant& v);

    // modifications
    ERR rename(const QString& newName);
    ERR addPlayer(const Player& p);
    ERR removePlayer(const Player& p);

    // boolean and other queries
    bool canAddPlayers() const;
    bool hasPlayer(const Player& p) const;
    bool canRemovePlayer(const Player& p) const;
    bool isPaired(const Player& p) const;
    ERR canPairPlayers(const Player& p1, const Player& p2) const;
    ERR canSplitPlayers(const Player& p1, const Player& p2) const;
    bool hasUnpairedPlayers() const;
    ERR canApplyGroupAssignment(const std::vector<PlayerPairList>& grpCfg);
    ERR canApplyInitialRanking(PlayerPairList seed);
    bool hasMatchesInState(ObjState stat, int round=-1) const;
    bool isDrawAllowedInRound(int round) const;

    
    virtual ~Category() {}
    
    //
    // The following methods MUST be overwritten by derived classes for a specific match system
    //
    virtual ERR canFreezeConfig();
    virtual bool needsInitialRanking();
    virtual bool needsGroupInitialization();
    virtual ERR prepareFirstRound();
    virtual int calcTotalRoundsCount() const;
    virtual ERR onRoundCompleted(int round);
    virtual std::function<bool (RankingEntry&, RankingEntry&)> getLessThanFunction();
    virtual PlayerPairList getRemainingPlayersAfterRound(int round, ERR *err) const;
    virtual PlayerPairList getPlayerPairsForIntermediateSeeding() const;
    virtual ERR resolveIntermediateSeeding(const PlayerPairList& seed) const;

    //
    // The following methods CAN be overwritten to extend the
    // categories functionality
    //
    virtual ModMatchResult canModifyMatchResult(const Match& ma) const;
    virtual ModMatchResult modifyMatchResult(const Match& ma, const MatchScore& newScore) const;

  private:
    Category (const TournamentDB& _db, int rowId);
    Category (const TournamentDB& _db, const SqliteOverlay::TabRow& _row);
    ERR applyGroupAssignment(const std::vector<PlayerPairList>& grpCfg);
    ERR applyInitialRanking(const PlayerPairList& seed);
    ERR generateGroupMatches(const PlayerPairList &grpMembers, int grpNum, int firstRoundNum=1) const;
    ERR generateBracketMatches(int bracketMode, const PlayerPairList& seeding, int firstRoundNum) const;
  };

  // we need this to have a category object in a QHash
  inline uint qHash(const Category& c)
  {
    return static_cast<uint>(c.getId()); // bad style: conversion from int to uint...
  }

  using CategoryList = std::vector<Category>;

}
#endif	/* CATEGORY_H */

