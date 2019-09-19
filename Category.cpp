/*
 *    This is QTournament, a badminton tournament management program.
 *    Copyright (C) 2014 - 2019  Volker Knollmann
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
#include <algorithm>

#include "Category.h"
#include "TournamentDataDefs.h"
#include "TournamentDB.h"
#include "TournamentErrorCodes.h"
#include "CatMngr.h"
#include "MatchMngr.h"
#include "RoundRobinCategory.h"
#include "RoundRobinGenerator.h"
#include "Match.h"
#include "CatRoundStatus.h"
#include "ElimCategory.h"
#include "PureRoundRobinCategory.h"
#include "SwissLadderCategory.h"
#include "HelperFunc.h"
#include "PlayerMngr.h"
#include "SvgBracketCategory.h"

using namespace SqliteOverlay;

namespace QTournament
{

  Category::Category(const TournamentDB& _db, int rowId)
  :TournamentDatabaseObject(_db, TabCategory, rowId)
  {
  }

  //----------------------------------------------------------------------------

  Category::Category(const TournamentDB& _db, const SqliteOverlay::TabRow& _row)
  :TournamentDatabaseObject(_db, _row)
  {
  }

  //----------------------------------------------------------------------------

  QString Category::getName() const
  {
    return stdString2QString(row[GenericNameFieldName]);
  }

  //----------------------------------------------------------------------------

  Error Category::rename(const QString& nn)
  {
    CatMngr cm{db};
    return cm.renameCategory(*this, nn);
  }

  //----------------------------------------------------------------------------

  MatchSystem Category::getMatchSystem() const
  {
    int sysInt = row.getInt(CAT_Sys);

    return static_cast<MatchSystem>(sysInt);
  }

  //----------------------------------------------------------------------------

  MatchType Category::getMatchType() const
  {
    int typeInt = row.getInt(CAT_MatchType);

    return static_cast<MatchType>(typeInt);
  }

  //----------------------------------------------------------------------------

  Sex Category::getSex() const
  {
    int sexInt = row.getInt(CAT_Sex);

    return static_cast<Sex>(sexInt);
  }

  //----------------------------------------------------------------------------

  Error Category::setMatchSystem(MatchSystem s)
  {
    CatMngr cm{db};
    return cm.setMatchSystem(*this, s);
  }

  //----------------------------------------------------------------------------

  Error Category::setMatchType(MatchType t)
  {
    CatMngr cm{db};
    return cm.setMatchType(*this, t);
  }

  //----------------------------------------------------------------------------

  Error Category::setSex(Sex s)
  {
    CatMngr cm{db};
    return cm.setSex(*this, s);
  }

  //----------------------------------------------------------------------------

  bool Category::canAddPlayers() const
  {
    // For now, you can only add players to a category
    // when it's still in configuration mode
    return isInState(ObjState::CAT_Config);

    // TODO: make more sophisticated tests depending e. g. on
    // the match system. For instance, if we have random
    // matches, players should be addable after every round
  }

  //----------------------------------------------------------------------------

  CatAddState Category::getAddState(const Sex s) const
  {
    if (!(canAddPlayers()))
    {
      return CatAddState::CatClosed;
    }

    // a "mixed" category can take any player
    if (getMatchType() == MatchType::Mixed)
    {
      return CatAddState::CanJoin;
    }

    // ok, so we're either in singles or doubles. if the sex
    // is set to Sex::DontCare, then also any player will fit
    if (getSex() == Sex::DontCare)
    {
      return CatAddState::CanJoin;
    }

    // in all other cases, the category's sex has to
    // match the player's sex
    return (s == getSex()) ? CatAddState::CanJoin : CatAddState::WrongSex;
  }

  //----------------------------------------------------------------------------

  CatAddState Category::getAddState(const Player& p) const
  {
    if (hasPlayer(p))
    {
      return CatAddState::AlreadyMember;
    }

    return getAddState(p.getSex());
  }

  //----------------------------------------------------------------------------

  Error Category::addPlayer(const Player& p)
  {
    CatMngr cm{db};
    return cm.addPlayerToCategory(p, *this);
  }

  //----------------------------------------------------------------------------

  bool Category::hasPlayer(const Player& p) const
  {
    SqliteOverlay::WhereClause wc;
    wc.addCol(P2C_PlayerRef, p.getId());
    wc.addCol(P2C_CatRef, getId());

    return (DbTab{db, TabP2C, false}.getMatchCountForWhereClause(wc) > 0);
  }

  //----------------------------------------------------------------------------

  bool Category::canRemovePlayer(const Player& p) const
  {
    // For now, you can only delete players from a category
    // when it's still in configuration mode
    if (is_NOT_InState(ObjState::CAT_Config)) return false;

    // check whether the player is paired with another player
    if (isPaired(p))
    {
      Player partner = getPartner(p);
      if (canSplitPlayers(p, partner) != Error::OK)
      {
        return false;
      }
    }

    // TODO: make more sophisticated tests depending e. g. on
    // the match system. For instance, if we have random
    // matches, players should be removable after every round
    //
    // Also, we should be able to remove only players that were not
    // yet involved/scheduled for any matches.

    return true;
  }

  //----------------------------------------------------------------------------

  Error Category::removePlayer(const Player& p)
  {
    CatMngr cm{db};
    return cm.removePlayerFromCategory(p, *this);
  }


  //----------------------------------------------------------------------------

  QVariant Category::getParameter(CatParameter p) const
  {
    switch (p) {

    case CatParameter::AllowDraw:
      return row.getInt(CAT_AcceptDraw);

    case CatParameter::WinScore:
      return row.getInt(CAT_WinScore);

    case CatParameter::DrawScore:
      return row.getInt(CAT_DrawScore);

    case CatParameter::GroupConfig:
      return QString::fromUtf8(row[CAT_GroupConfig].data());

    case CatParameter::RoundRobinIterations:
      return row.getInt(CAT_RoundRobinIterations);

    case CatParameter::BracketMatchSystem:
      return row.getInt(CAT_BracketMatchSys);

    case CatParameter::FirstRoundOffset:
      return row.getInt(CAT_RoundOffset);

      /*
      case :
	return row[];
      
      case :
	return row[];
      
      case :
	return row[];
       */
    default:
      return QVariant();
    }
  }

  //----------------------------------------------------------------------------

  bool Category::setParameter(CatParameter p, const QVariant& v)
  {
    CatMngr cm{db};
    return cm.setCatParameter(*this, p, v);
  }

  //----------------------------------------------------------------------------

  int Category::getParameter_int(CatParameter p) const
  {
    return getParameter(p).toInt();
  }

  //----------------------------------------------------------------------------

  bool Category::getParameter_bool(CatParameter p) const
  {
    return getParameter(p).toBool();
  }

  //----------------------------------------------------------------------------

  QString Category::getParameter_string(CatParameter p) const
  {
    return getParameter(p).toString();
  }

  //----------------------------------------------------------------------------

  /**
    Retrieves a list of PlayerPairs in this category. This method checks for
    both, PlayerPairs already created in the database and unpaired players
    that are not yet in the database.

    Additionally, the retrieved PlayerPairs can be limited to one
    match group only. If this filter is used there will be no check
    for unpaired, single players that are not yet in the datase.

    \param grp the group number to limit the results to (negative if not used)

    \return a QList of PlayerPairs
    */
  PlayerPairList Category::getPlayerPairs(int grp) const
  {
    PlayerPairList result;
    PlayerMngr pmngr{db};

    // get all players assigned to this category
    PlayerList singlePlayers = getAllPlayersInCategory();

    // filter out the players that are paired and have already
    // entries in the database
    SqliteOverlay::WhereClause wc;
    wc.addCol(Pairs_CatRef, getId());
    if (grp > 0) wc.addCol(Pairs_GrpNum, grp);

    for (TabRowIterator it{db, TabPairs, wc}; it.hasData(); ++it)
    {
      const auto& pairRow = *it;

      int id1 = pairRow.getInt(Pairs_Player1Ref);
      Player p1 = pmngr.getPlayer(id1);
      Sloppy::eraseAllOccurencesFromVector<Player>(singlePlayers, p1);

      // id2 is sometimes empty, e.g. in singles categories
      auto id2 = pairRow.getInt2(Pairs_Player2Ref);
      if (!id2) {
        result.push_back(PlayerPair(p1, pairRow.id()));
      } else {
        Player p2 = pmngr.getPlayer(*id2);
        result.push_back(PlayerPair(p1, p2, pairRow.id()));
        Sloppy::eraseAllOccurencesFromVector<Player>(singlePlayers, p2);
      }
    }

    // if we have a valid group number, we are not interested in
    // the unpaired player that are not yet in the database
    //
    // Reason:
    // If we have group numbers assigned, unpaired, non-databased
    // players can't exist anymore
    if (grp > 0) return result;

    // create special entries for un-paired players that do
    // not yet have an entry in the database
    for (const Player& p : singlePlayers)
    {
      result.push_back(PlayerPair(p));
    }

    return result;
  }

  //----------------------------------------------------------------------------

  int Category::getDatabasePlayerPairCount(int grp) const
  {
    // since we want to count only player pairs in the database,
    // we must be beyond CONFIG to be sure that valid database
    // entries exist
    if (isInState(ObjState::CAT_Config)) return -1;

    DbTab pairTab{db, TabPairs, false};
    SqliteOverlay::WhereClause wc;
    wc.addCol(Pairs_CatRef, getId());
    if (grp > 0) wc.addCol(Pairs_GrpNum, grp);
    return pairTab.getMatchCountForWhereClause(wc);
  }

  //----------------------------------------------------------------------------

  PlayerList Category::getAllPlayersInCategory() const
  {
    PlayerList result;
    PlayerMngr pmngr{db};

    auto rows = DbTab{db, TabP2C, false}.getRowsByColumnValue(P2C_CatRef, getId());
    for (const auto& row : rows)
    {
      result.push_back(pmngr.getPlayer(row.getInt(P2C_PlayerRef)));
    }

    return result;
  }

  //----------------------------------------------------------------------------

  bool Category::isPaired(const Player& p) const
  {
    if (!(hasPlayer(p)))
    {
      return false;
    }

    // manually construct a where-clause for an OR-query
    Sloppy::estring where{"(%1 = %2 OR %3 = %2) AND (%4 = %5)"};
    where.arg(Pairs_Player1Ref);
    where.arg(p.getId());
    where.arg(Pairs_Player2Ref);
    where.arg(Pairs_CatRef);
    where.arg(getId());

    // see if we have a row that matches the query
    return (DbTab{db, TabPairs, false}.getMatchCountForWhereClause(where) != 0);
  }

  //----------------------------------------------------------------------------

  Error Category::canPairPlayers(const Player& p1, const Player& p2) const
  {
    // we can only create pairs while being in config mode
    if (is_NOT_InState(ObjState::CAT_Config))
    {
      return Error::CategoryNotConfiguraleAnymore;
    }

    // in singles, we don't need pairs. The same is true if we're using
    // the match system "random matches with random partners".
    MatchType mt = getMatchType();
    if (mt == MatchType::Singles)
    {
      return Error::NoCategoryForPairing;
    }
    // TODO: check for "random" with "random partners"


    // make sure that both players are actually listed in this category
    if ((!(hasPlayer(p1))) || (!(hasPlayer(p2))))
    {
      return Error::PlayerNotInCategory;
    }

    // make sure that both players are not yet paired in this category
    if ((isPaired(p1)) || (isPaired(p2)))
    {
      return Error::PlayerAlreadyPaired;
    }

    // make sure that the players are not identical
    if (p1 == p2)
    {
      return Error::PlayersIdentical;
    }

    // if this is a mixed category, make sure the sex is right
    if ((mt == MatchType::Mixed) && (getSex() != Sex::DontCare))
    {
      if (p1.getSex() == p2.getSex())
      {
        return Error::InvalidSex;
      }
    }

    // if this is a doubles category, make sure the sex is right
    if ((mt == MatchType::Doubles) && (getSex() != Sex::DontCare))
    {
      Sex catSex = getSex();
      if ((p1.getSex() != catSex) || (p2.getSex() != catSex))
      {
        return Error::InvalidSex;
      }
    }

    return Error::OK;
  }

  //----------------------------------------------------------------------------

  Error Category::canSplitPlayers(const Player& p1, const Player& p2) const
  {
    // we can only split pairs while being in config mode
    if (is_NOT_InState(ObjState::CAT_Config))
    {
      return Error::CategoryNotConfiguraleAnymore;
    }

    // check if the two players are paired for this category
    SqliteOverlay::WhereClause wc;
    wc.addCol(Pairs_CatRef, getId());
    wc.addCol(Pairs_Player1Ref, p1.getId());
    wc.addCol(Pairs_Player2Ref, p2.getId());
    DbTab pairsTab{db, TabPairs, false};
    if (pairsTab.getMatchCountForWhereClause(wc) != 0)
    {
      return Error::OK;
    }

    // swap player 1 and player 2 and make a new query
    wc.clear();
    wc.addCol(Pairs_CatRef, getId());
    wc.addCol(Pairs_Player1Ref, p2.getId());
    wc.addCol(Pairs_Player2Ref, p1.getId());
    if (pairsTab.getMatchCountForWhereClause(wc) != 0)
    {
      return Error::OK;
    }

    return Error::PlayersNotAPair;
  }

  //----------------------------------------------------------------------------

  Player Category::getPartner(const Player& p) const
  {
    if (!(hasPlayer(p)))
    {
      throw std::invalid_argument("Player not in category");
    }

    // manually construct a where-clause for an OR-query
    Sloppy::estring where{"(%1 = %2 OR %3 = %2) AND (%4 = %5)"};
    where.arg(Pairs_Player1Ref);
    where.arg(p.getId());
    where.arg(Pairs_Player2Ref);
    where.arg(Pairs_CatRef);
    where.arg(getId());

    // see if we have a row that matches the query
    int partnerId = -1;
    DbTab pairTab{db, TabPairs, false};
    auto row = pairTab.getSingleRowByWhereClause2(where);
    if (!row)
    {
      throw std::invalid_argument("Player doesn't have a partner");
    }

    // check if we have a partner
    auto p2Id = row->getInt2(Pairs_Player2Ref);
    if (!p2Id)
    {
      throw std::invalid_argument("Player doesn't have a partner");
    }
    int p1Id = row->getInt(Pairs_Player1Ref);
    partnerId = (p1Id == p.getId()) ? *p2Id : p1Id;

    PlayerMngr pmngr{db};
    return pmngr.getPlayer(partnerId);
  }

  //----------------------------------------------------------------------------

  bool Category::hasUnpairedPlayers() const
  {
    for (const auto& pp : getPlayerPairs())
    {
      if (!(pp.hasPlayer2())) return true;
    }

    return false;
  }

  //----------------------------------------------------------------------------

  std::unique_ptr<Category> Category::convertToSpecializedObject() const
  {
    // return an instance of a suitable, specialized category-child
    MatchSystem sys = getMatchSystem();

    if (sys == MatchSystem::GroupsWithKO) {
      return std::unique_ptr<Category>(new RoundRobinCategory(db, row));
    }

    if (sys == MatchSystem::RoundRobin)
    {
      return std::unique_ptr<Category>(new PureRoundRobinCategory(db, row));
    }

    if (sys == MatchSystem::SwissLadder)
    {
      return std::unique_ptr<Category>(new SwissLadderCategory(db, row));
    }

    if (sys == MatchSystem::Bracket)
    {
      int brackSys = getParameter_int(CatParameter::BracketMatchSystem);
      return std::unique_ptr<Category>(new SvgBracketCategory(db, row, static_cast<SvgBracketMatchSys>(brackSys)));
    }

    // THIS IS JUST A HOT FIX UNTIL WE HAVE
    // SPECIALIZED CLASSED FOR ALL MATCH SYSTEMS!!!
    //
    // Returning an object of the base class instead of the derived class
    // will end up in exceptions and abnormal program termination
    return std::unique_ptr<Category>(new Category(db, row));
  }

  //----------------------------------------------------------------------------

  Error Category::canApplyGroupAssignment(const std::vector<PlayerPairList>& grpCfg)
  {
    if (is_NOT_InState(ObjState::CAT_Frozen)) return Error::CategoryNotYetFrozen;

    std::unique_ptr<Category> specializedCat = convertToSpecializedObject();
    if (!(specializedCat->needsGroupInitialization()))
    {
      return Error::CategoryNeedsNoGroupAssignments;
    }

    KO_Config cfg = KO_Config(getParameter(CatParameter::GroupConfig).toString());
    if (!(cfg.isValid())) return Error::InvalidKoConfig;

    // check if the grpCfg matches the KO_Config
    if (grpCfg.size() != cfg.getNumGroups()) return Error::InvalidGroupNum;
    int pairsInGrpCfg = 0;
    for (int i=0; i<grpCfg.size(); i++)
    {
      pairsInGrpCfg += grpCfg.at(i).size();
    }
    int pairsInCategory = getPlayerPairs().size();
    if (pairsInCategory != pairsInGrpCfg) return Error::InvalidPlayerCount;

    // make sure we only have player pairs that actually belong to this category
    PlayerPairList allPairs = getPlayerPairs();
    for (int i=0; i<grpCfg.size(); i++)
    {
      PlayerPairList thisGroup = grpCfg.at(i);

      for (int cnt=0; cnt < thisGroup.size(); cnt++)
      {
        int ppId = thisGroup.at(cnt).getPairId();

        bool isValid = false;
        for (int n=0; n < allPairs.size(); n++)
        {
          if (allPairs.at(n).getPairId() == ppId)
          {
            isValid = true;
            allPairs.erase(allPairs.begin() + n);  // delete the n'th element to reduce the number of checks for the next loop
            break;
          }
        }

        if (!isValid) return Error::PlayerNotInCategory;
      }
    }

    // okay, we're good to go!
    return Error::OK;
  }

  //----------------------------------------------------------------------------

  Error Category::canApplyInitialRanking(PlayerPairList seed)
  {
    if (is_NOT_InState(ObjState::CAT_Frozen)) return Error::CategoryNotYetFrozen;

    std::unique_ptr<Category> specializedCat = convertToSpecializedObject();
    if (!(specializedCat->needsInitialRanking()))
    {
      return Error::CategoryNeedsNoSeeding;
    }

    int pairsInCategory = getPlayerPairs().size();
    if (pairsInCategory != seed.size()) return Error::InvalidPlayerCount;

    // make sure we only have player pairs that actually belong to this category
    PlayerPairList allPairs = getPlayerPairs();
    for (int i=0; i<seed.size(); i++)
    {
      int ppId = seed.at(i).getPairId();

      bool isValid = false;
      for (int n=0; n < allPairs.size(); n++)
      {
        if (allPairs.at(n).getPairId() == ppId)
        {
          isValid = true;
          allPairs.erase(allPairs.begin() + n);  // delete the n'th element to reduce the number of checks for the next loop
          break;
        }
      }

      if (!isValid) return Error::PlayerNotInCategory;
    }

    // okay, we're good to go!
    return Error::OK;
  }

  //----------------------------------------------------------------------------

  Error Category::applyGroupAssignment(const std::vector<PlayerPairList>& grpCfg)
  {
    Error e = canApplyGroupAssignment(grpCfg);
    if (e != Error::OK) return e;
    
    // The previous call checked for all possible errors or
    // misconfigurations. So we can safely write directly to the database
    DbTab pairTab{db, TabPairs, false};
    int grpNum = 0;
    for (const PlayerPairList& ppl : grpCfg)
    {
      ++grpNum;   // we start counting groups with "1"
      for (const PlayerPair& pp : ppl)
      {
        int ppId = pp.getPairId();
        pairTab[ppId].update(Pairs_GrpNum, grpNum);
      }
    }
    
    return Error::OK;
  }

  //----------------------------------------------------------------------------

  Error Category::applyInitialRanking(const PlayerPairList& seed)
  {
    Error e = canApplyInitialRanking(seed);
    if (e != Error::OK) return e;
    
    // The previous call checked for all possible errors or
    // misconfigurations. So we can safely write directly to the database
    DbTab pairTab{db, TabPairs, false};
    int rank{1}; // we start counting ranks at "1"
    for (const auto& pp : seed)
    {
      int ppId = pp.getPairId();
      pairTab[ppId].update(Pairs_InitialRank, rank);
      ++rank;
    }
    
    return Error::OK;
  }

  //----------------------------------------------------------------------------

  /**
    Convenience function that generates a set of group matches for
    a set of PlayerPairs. This function does not do any error checking
    whether the PlayerPairs or other arguments are valid. It assumes
    that those checks have been performed before and that it's generally
    safe to generate the matches here and now.

    \param grpMembers list of PlayerPairs for that the group matches will be generated
    \param grpNum the group number that will be applied to the matches / match groups
    \param firstRoundNum the number of the first round of group matches, usually 1
    \param progressNotificationQueue is an optional pointer to a FIFO that communicates progress back to the GUI

    \return error code
    */
  Error Category::generateGroupMatches(const PlayerPairList& grpMembers, int grpNum, int firstRoundNum) const
  {
    if ((grpNum < 1) && (grpNum != GroupNum_Iteration)) return Error::InvalidGroupNum;

    RoundRobinGenerator rrg;
    MatchMngr mm{db};
    int numPlayers = static_cast<int>(grpMembers.size());
    int internalRoundNum = 0;

    try
    {
      // create all or nothing
      auto trans = db.get().startTransaction();

      while (true)
      {
        // create matches for the next round
        auto matches = rrg(numPlayers, internalRoundNum);

        // if no new matches were created, we have
        // covered all necessary rounds and can return
        if (matches.size() == 0)
        {
          trans.commit();
          return Error::OK;
        }

        // create a match group for the new round
        auto mg = mm.createMatchGroup(*this, firstRoundNum + internalRoundNum, grpNum);
        if (!mg) return mg.err();

        // assign matches to this group
        for (auto [pairIndex1, pairIndex2] : matches)
        {
          const PlayerPair& pp1 = grpMembers.at(pairIndex1);
          const PlayerPair& pp2 = grpMembers.at(pairIndex2);

          auto newMatch = mm.createMatch(*mg);
          if (!newMatch) return newMatch.err();

          Error e = mm.setPlayerPairsForMatch(*newMatch, pp1, pp2);
          if (e != Error::OK) return e;
        }

        // close this group (transition to FROZEN) and potentially promote it further to IDLE
        mm.closeMatchGroup(*mg);

        ++internalRoundNum;
      }
    }
    catch (...)
    {
      return Error::DatabaseError;
    }
  }

  //----------------------------------------------------------------------------

  CatRoundStatus Category::getRoundStatus() const
  {
    return CatRoundStatus(db, *this);
  }

  //----------------------------------------------------------------------------

  bool Category::hasMatchesInState(ObjState stat, int round) const
  {
    MatchMngr mm{db};

    MatchGroupList mgl = mm.getMatchGroupsForCat(*this, round);
    for (MatchGroup mg : mgl)
    {
      if (mg.hasMatchesInState(stat)) return true;
    }

    return false;
  }

  //----------------------------------------------------------------------------

  PlayerPairList Category::getEliminatedPlayersAfterRound(int round, Error* err) const
  {
    //
    // Approach: determine the list of eliminated players by
    // subtracting the remaining players from the full list of players
    //

    auto specialCat = convertToSpecializedObject();

    Error e;
    PlayerPairList remainingPlayers = specialCat->getRemainingPlayersAfterRound(round, &e);
    if (e != Error::OK)
    {
      if (err != nullptr) *err = e;
      return PlayerPairList();
    }

    PlayerPairList allPlayers = getPlayerPairs();
    PlayerPairList eliminatedPlayers;
    for (PlayerPair pp : allPlayers)
    {
      if (std::find(remainingPlayers.begin(), remainingPlayers.end(), pp) != remainingPlayers.end()) continue;
      eliminatedPlayers.push_back(pp);
    }

    if (err != nullptr) *err = Error::OK;
    return eliminatedPlayers;
  }

  //----------------------------------------------------------------------------

  int Category::getMaxNumGamesInRound(int round) const
  {
    //
    // TODO: the basic number of winGames needs to become
    // a category parameter
    //
    int numWinGames = 2;

    // calculate the results for both cases, draw or no draw
    int resultWithDraw = 2 * (numWinGames - 1);
    int resultNoDraw = 2 * numWinGames - 1;


    // return the number of games dependent on the
    // draw status
    return (isDrawAllowedInRound(round) ? resultWithDraw : resultNoDraw);
  }

  //----------------------------------------------------------------------------

  bool Category::isDrawAllowedInRound(int round) const
  {
    // is a draw basically allowed?
    bool isDrawAllowed = getParameter_bool(CatParameter::AllowDraw);
    if (!isDrawAllowed)
    {
      return false;
    }


    //
    // everything below this point can only be reached if the
    // "CatParameter::AllowDraw" parameter is true
    //


    // in any kind of "bracket match", draws are not possible.
    // So we need to have a "decision game", if necessary
    MatchSystem ms = getMatchSystem();
    if (ms == MatchSystem::Bracket)
    {
      return false;
    }

    // in swiss ladder or random matches, draws are okay
    if ((ms == MatchSystem::SwissLadder) || (ms == MatchSystem::Randomize))
    {
      return true;
    }

    // in round-robins with subsequent KO matches, it depends on the round
    // we're in
    if (ms == MatchSystem::GroupsWithKO)
    {
      // invalid parameter
      if (round < 1) return false;

      KO_Config cfg = KO_Config(getParameter_string(CatParameter::GroupConfig));
      if (round <= cfg.getNumRounds())
      {
        // if draw is allowed and we're still in the round-robin phase,
        // a draw is possible
        return true;
      }

      // in the KO-phase, draw is not possible, regardless of the
      // category setting
      return false;
    }

    // default value, should never be reached
    return false;
  }

  //----------------------------------------------------------------------------

  Error Category::canFreezeConfig()
  {
    throw std::runtime_error("Unimplemented Method: canFreezeConfig");
  }

  //----------------------------------------------------------------------------

  bool Category::needsInitialRanking()
  {
    throw std::runtime_error("Unimplemented Method: needsInitialRanking");
  }

  //----------------------------------------------------------------------------

  bool Category::needsGroupInitialization()
  {
    throw std::runtime_error("Unimplemented Method: needsGroupInitialization");
  }

  //----------------------------------------------------------------------------

  Error Category::prepareFirstRound()
  {
    throw std::runtime_error("Unimplemented Method: prepareFirstRound");
  }

  //----------------------------------------------------------------------------

  int Category::calcTotalRoundsCount() const
  {
    throw std::runtime_error("Unimplemented Method: calcTotalRoundsCount");
  }

  //----------------------------------------------------------------------------

  Error Category::onRoundCompleted(int round)
  {
    throw std::runtime_error("Unimplemented Method: onRoundCompleted");
  }

  //----------------------------------------------------------------------------

  std::function<bool (RankingEntry&, RankingEntry&)> Category::getLessThanFunction()
  {
    throw std::runtime_error("Unimplemented Method: getLessThanFunction");
  }

  //----------------------------------------------------------------------------

  PlayerPairList Category::getRemainingPlayersAfterRound(int round, Error* err) const
  {
    throw std::runtime_error("Unimplemented Method: getRemainingPlayersAfterRound");
  }

  //----------------------------------------------------------------------------

  PlayerPairList Category::getPlayerPairsForIntermediateSeeding() const
  {
    throw std::runtime_error("Unimplemented Method: getPlayerPairsForIntermediateSeeding");
  }

  //----------------------------------------------------------------------------

  Error Category::resolveIntermediateSeeding(const PlayerPairList& seed) const
  {
    throw std::runtime_error("Unimplemented Method: resolveIntermediateSeeding");
  }

  //----------------------------------------------------------------------------

  ModMatchResult Category::canModifyMatchResult(const Match& ma) const
  {
    return ModMatchResult::NotImplemented;
  }

  //----------------------------------------------------------------------------

  ModMatchResult Category::modifyMatchResult(const Match& ma, const MatchScore& newScore) const
  {
    return ModMatchResult::NotImplemented;
  }

  //----------------------------------------------------------------------------

  //----------------------------------------------------------------------------


  //----------------------------------------------------------------------------


  //----------------------------------------------------------------------------


  //----------------------------------------------------------------------------


  //----------------------------------------------------------------------------


  //----------------------------------------------------------------------------


  //----------------------------------------------------------------------------


  //----------------------------------------------------------------------------


  //----------------------------------------------------------------------------

}
