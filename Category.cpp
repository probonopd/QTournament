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
#include <algorithm>

#include "Category.h"
#include "TournamentDataDefs.h"
#include "TournamentDB.h"
#include "TournamentErrorCodes.h"
#include "CatMngr.h"
#include "RoundRobinCategory.h"
#include "RoundRobinGenerator.h"
#include "Match.h"
#include "CatRoundStatus.h"
#include "BracketGenerator.h"
#include "ElimCategory.h"
#include "PureRoundRobinCategory.h"
#include "SwissLadderCategory.h"
#include "HelperFunc.h"
#include "PlayerMngr.h"

namespace QTournament
{

  Category::Category(TournamentDB* db, int rowId)
  :TournamentDatabaseObject(db, TAB_CATEGORY, rowId)
  {
  }

  //----------------------------------------------------------------------------

  Category::Category(TournamentDB* db, SqliteOverlay::TabRow row)
  :TournamentDatabaseObject(db, row)
  {
  }

  //----------------------------------------------------------------------------

  QString Category::getName() const
  {
    return QString::fromUtf8(row[GENERIC_NAME_FIELD_NAME].data());
  }

  //----------------------------------------------------------------------------

  ERR Category::rename(const QString& nn)
  {
    CatMngr cm{db};
    return cm.renameCategory(*this, nn);
  }

  //----------------------------------------------------------------------------

  MATCH_SYSTEM Category::getMatchSystem() const
  {
    int sysInt = row.getInt(CAT_SYS);

    return static_cast<MATCH_SYSTEM>(sysInt);
  }

  //----------------------------------------------------------------------------

  MATCH_TYPE Category::getMatchType() const
  {
    int typeInt = row.getInt(CAT_MATCH_TYPE);

    return static_cast<MATCH_TYPE>(typeInt);
  }

  //----------------------------------------------------------------------------

  SEX Category::getSex() const
  {
    int sexInt = row.getInt(CAT_SEX);

    return static_cast<SEX>(sexInt);
  }

  //----------------------------------------------------------------------------

  ERR Category::setMatchSystem(MATCH_SYSTEM s)
  {
    CatMngr cm{db};
    return cm.setMatchSystem(*this, s);
  }

  //----------------------------------------------------------------------------

  ERR Category::setMatchType(MATCH_TYPE t)
  {
    CatMngr cm{db};
    return cm.setMatchType(*this, t);
  }

  //----------------------------------------------------------------------------

  ERR Category::setSex(SEX s)
  {
    CatMngr cm{db};
    return cm.setSex(*this, s);
  }

  //----------------------------------------------------------------------------

  bool Category::canAddPlayers() const
  {
    // For now, you can only add players to a category
    // when it's still in configuration mode
    return (getState() == STAT_CAT_CONFIG);

    // TODO: make more sophisticated tests depending e. g. on
    // the match system. For instance, if we have random
    // matches, players should be addable after every round
  }

  //----------------------------------------------------------------------------

  CAT_ADD_STATE Category::getAddState(const SEX s) const
  {
    if (!(canAddPlayers()))
    {
      return CAT_CLOSED;
    }

    // a "mixed" category can take any player
    if (getMatchType() == MIXED)
    {
      return CAN_JOIN;
    }

    // ok, so we're either in singles or doubles. if the sex
    // is set to DONT_CARE, then also any player will fit
    if (getSex() == DONT_CARE)
    {
      return CAN_JOIN;
    }

    // in all other cases, the category's sex has to
    // match the player's sex
    return (s == getSex()) ? CAN_JOIN : WRONG_SEX;
  }

  //----------------------------------------------------------------------------

  CAT_ADD_STATE Category::getAddState(const Player& p) const
  {
    if (hasPlayer(p))
    {
      return ALREADY_MEMBER;
    }

    return getAddState(p.getSex());
  }

  //----------------------------------------------------------------------------

  ERR Category::addPlayer(const Player& p)
  {
    CatMngr cm{db};
    return cm.addPlayerToCategory(p, *this);
  }

  //----------------------------------------------------------------------------

  bool Category::hasPlayer(const Player& p) const
  {
    SqliteOverlay::WhereClause wc;
    wc.addIntCol(P2C_PLAYER_REF, p.getId());
    wc.addIntCol(P2C_CAT_REF, getId());

    return (db->getTab(TAB_P2C)->getMatchCountForWhereClause(wc) > 0);
  }

  //----------------------------------------------------------------------------

  bool Category::canRemovePlayer(const Player& p) const
  {
    // For now, you can only delete players from a category
    // when it's still in configuration mode
    if (getState() != STAT_CAT_CONFIG) return false;

    // check whether the player is paired with another player
    if (isPaired(p))
    {
      Player partner = getPartner(p);
      if (canSplitPlayers(p, partner) != OK)
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

  ERR Category::removePlayer(const Player& p)
  {
    CatMngr cm{db};
    return cm.removePlayerFromCategory(p, *this);
  }


  //----------------------------------------------------------------------------

  QVariant Category::getParameter(CAT_PARAMETER p) const
  {
    switch (p) {

    case ALLOW_DRAW:
      return row.getInt(CAT_ACCEPT_DRAW);

    case WIN_SCORE:
      return row.getInt(CAT_WIN_SCORE);

    case DRAW_SCORE:
      return row.getInt(CAT_DRAW_SCORE);

    case GROUP_CONFIG:
      return QString::fromUtf8(row[CAT_GROUP_CONFIG].data());

    case ROUND_ROBIN_ITERATIONS:
      return row.getInt(CAT_ROUND_ROBIN_ITERATIONS);
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

  bool Category::setParameter(CAT_PARAMETER p, const QVariant& v)
  {
    CatMngr cm{db};
    return cm.setCatParameter(*this, p, v);
  }

  //----------------------------------------------------------------------------

  int Category::getParameter_int(CAT_PARAMETER p) const
  {
    return getParameter(p).toInt();
  }

  //----------------------------------------------------------------------------

  bool Category::getParameter_bool(CAT_PARAMETER p) const
  {
    return getParameter(p).toBool();
  }

  //----------------------------------------------------------------------------

  QString Category::getParameter_string(CAT_PARAMETER p) const
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
    wc.addIntCol(PAIRS_CAT_REF, getId());
    if (grp > 0) wc.addIntCol(PAIRS_GRP_NUM, grp);
    auto it = db->getTab(TAB_PAIRS)->getRowsByWhereClause(wc);
    while (!(it.isEnd()))
    {
      int id1 = (*it).getInt(PAIRS_PLAYER1_REF);
      Player p1 = pmngr.getPlayer(id1);
      eraseAllValuesFromVector<Player>(singlePlayers, p1);

      // id2 is sometimes empty, e.g. in singles categories
      auto _id2 = (*it).getInt2(PAIRS_PLAYER2_REF);
      if (_id2->isNull()) {
        result.push_back(PlayerPair(p1, (*it).getId()));
      } else {
        int id2 = _id2->get();
        Player p2 = pmngr.getPlayer(id2);
        result.push_back(PlayerPair(p1, p2, (*it).getId()));
        eraseAllValuesFromVector<Player>(singlePlayers, p2);
      }

      ++it;
    }

    // if we have a valid group number, we are not interested in
    // the unpaired player that are not yet in the database
    //
    // Reason:
    // If we have group numbers assigned, unpaired, non-databased
    // players can't exists anymore
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
    if (getState() == STAT_CAT_CONFIG) return -1;

    DbTab* pairTab = db->getTab(TAB_PAIRS);
    SqliteOverlay::WhereClause wc;
    wc.addIntCol(PAIRS_CAT_REF, getId());
    if (grp > 0) wc.addIntCol(PAIRS_GRP_NUM, grp);
    return pairTab->getMatchCountForWhereClause(wc);
  }

  //----------------------------------------------------------------------------

  PlayerList Category::getAllPlayersInCategory() const
  {
    PlayerList result;
    PlayerMngr pmngr{db};

    auto it = db->getTab(TAB_P2C)->getRowsByColumnValue(P2C_CAT_REF, getId());
    while (!(it.isEnd()))
    {
      result.push_back(pmngr.getPlayer((*it).getInt(P2C_PLAYER_REF)));
      ++it;
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
    QString whereClause = "(%1 = %2 OR %3 = %2) AND (%4 = %5)";
    whereClause = whereClause.arg(PAIRS_PLAYER1_REF).arg(p.getId());
    whereClause = whereClause.arg(PAIRS_PLAYER2_REF);
    whereClause = whereClause.arg(PAIRS_CAT_REF).arg(getId());

    // see if we have a row that matches the query
    SqliteOverlay::DbTab* pairTab = db->getTab(TAB_PAIRS);
    return (pairTab->getMatchCountForWhereClause(whereClause.toUtf8().constData()) != 0);
  }

  //----------------------------------------------------------------------------

  ERR Category::canPairPlayers(const Player& p1, const Player& p2) const
  {
    // we can only create pairs while being in config mode
    if (getState() != STAT_CAT_CONFIG)
    {
      return CATEGORY_NOT_CONFIGURALE_ANYMORE;
    }

    // in singles, we don't need pairs. The same is true if we're using
    // the match system "random matches with random partners".
    MATCH_TYPE mt = getMatchType();
    if (mt == SINGLES)
    {
      return NO_CATEGORY_FOR_PAIRING;
    }
    // TODO: check for "random" with "random partners"


    // make sure that both players are actually listed in this category
    if ((!(hasPlayer(p1))) || (!(hasPlayer(p2))))
    {
      return PLAYER_NOT_IN_CATEGORY;
    }

    // make sure that both players are not yet paired in this category
    if ((isPaired(p1)) || (isPaired(p2)))
    {
      return PLAYER_ALREADY_PAIRED;
    }

    // make sure that the players are not identical
    if (p1 == p2)
    {
      return PLAYERS_IDENTICAL;
    }

    // if this is a mixed category, make sure the sex is right
    if ((mt == MIXED) && (getSex() != DONT_CARE))
    {
      if (p1.getSex() == p2.getSex())
      {
	return INVALID_SEX;
      }
    }

    // if this is a doubles category, make sure the sex is right
    if ((mt == DOUBLES) && (getSex() != DONT_CARE))
    {
      SEX catSex = getSex();
      if ((p1.getSex() != catSex) || (p2.getSex() != catSex))
      {
	return INVALID_SEX;
      }
    }

    return OK;
  }

  //----------------------------------------------------------------------------

  ERR Category::canSplitPlayers(const Player& p1, const Player& p2) const
  {
    // we can only split pairs while being in config mode
    if (getState() != STAT_CAT_CONFIG)
    {
      return CATEGORY_NOT_CONFIGURALE_ANYMORE;
    }

    // check if the two players are paired for this category
    SqliteOverlay::WhereClause wc;
    wc.addIntCol(PAIRS_CAT_REF, getId());
    wc.addIntCol(PAIRS_PLAYER1_REF, p1.getId());
    wc.addIntCol(PAIRS_PLAYER2_REF, p2.getId());
    SqliteOverlay::DbTab* pairsTab = db->getTab(TAB_PAIRS);
    if (pairsTab->getMatchCountForWhereClause(wc) != 0)
    {
      return OK;
    }

    // swap player 1 and player 2 and make a new query
    wc.clear();
    wc.addIntCol(PAIRS_CAT_REF, getId());
    wc.addIntCol(PAIRS_PLAYER1_REF, p2.getId());
    wc.addIntCol(PAIRS_PLAYER2_REF, p1.getId());
    if (pairsTab->getMatchCountForWhereClause(wc) != 0)
    {
      return OK;
    }

    return PLAYERS_NOT_A_PAIR;
  }

  //----------------------------------------------------------------------------

  Player Category::getPartner(const Player& p) const
  {
    if (!(hasPlayer(p)))
    {
      throw std::invalid_argument("Player not in category");
    }

    // manually construct a where-clause for an OR-query
    QString whereClause = "(%1 = %2 OR %3 = %2) AND (%4 = %5)";
    whereClause = whereClause.arg(PAIRS_PLAYER1_REF).arg(p.getId());
    whereClause = whereClause.arg(PAIRS_PLAYER2_REF);
    whereClause = whereClause.arg(PAIRS_CAT_REF).arg(getId());

    // see if we have a row that matches the query
    int partnerId = -1;
    SqliteOverlay::DbTab* pairTab = db->getTab(TAB_PAIRS);
    if (pairTab->getMatchCountForWhereClause(whereClause.toUtf8().constData()) == 0)
    {
      throw std::invalid_argument("Player doesn't have a partner");
    }

    // check if we have a partner
    SqliteOverlay::TabRow r = pairTab->getSingleRowByWhereClause(whereClause.toUtf8().constData());
    auto p2Id = r.getInt2(PAIRS_PLAYER2_REF);
    if (p2Id->isNull())
    {
      throw std::invalid_argument("Player doesn't have a partner");
    }
    int p1Id = r.getInt(PAIRS_PLAYER1_REF);
    partnerId = (p1Id == p.getId()) ? p2Id->get() : p1Id;

    PlayerMngr pmngr{db};
    return pmngr.getPlayer(partnerId);
  }

  //----------------------------------------------------------------------------

  bool Category::hasUnpairedPlayers() const
  {
    PlayerPairList pp = getPlayerPairs();
    for (int i=0; i < pp.size(); i++)
    {
      if (!(pp.at(i).hasPlayer2())) return true;
    }

    return false;
  }

  //----------------------------------------------------------------------------

  unique_ptr<Category> Category::convertToSpecializedObject() const
  {
    // return an instance of a suitable, specialized category-child
    MATCH_SYSTEM sys = getMatchSystem();

    if (sys == GROUPS_WITH_KO) {
      return unique_ptr<Category>(new RoundRobinCategory(db, row));
    }

    if (sys == SINGLE_ELIM) {
      return unique_ptr<Category>(new EliminationCategory(db, row, BracketGenerator::BRACKET_SINGLE_ELIM));
    }

    if (sys == RANKING) {
      return unique_ptr<Category>(new EliminationCategory(db, row, BracketGenerator::BRACKET_RANKING1));
    }

    if (sys == ROUND_ROBIN)
    {
      return unique_ptr<Category>(new PureRoundRobinCategory(db, row));
    }

    if (sys == SWISS_LADDER)
    {
      return unique_ptr<Category>(new SwissLadderCategory(db, row));
    }

    // THIS IS JUST A HOT FIX UNTIL WE HAVE
    // SPECIALIZED CLASSED FOR ALL MATCH SYSTEMS!!!
    //
    // Returning an object of the base class instead of the derived class
    // will end up in exceptions and abnormal program termination
    return unique_ptr<Category>(new Category(db, row));
  }

  //----------------------------------------------------------------------------

  ERR Category::canApplyGroupAssignment(vector<PlayerPairList> grpCfg)
  {
    if (getState() != STAT_CAT_FROZEN) return CATEGORY_NOT_YET_FROZEN;

    unique_ptr<Category> specializedCat = convertToSpecializedObject();
    if (!(specializedCat->needsGroupInitialization()))
    {
      return CATEGORY_NEEDS_NO_GROUP_ASSIGNMENTS;
    }

    KO_Config cfg = KO_Config(getParameter(GROUP_CONFIG).toString());
    if (!(cfg.isValid())) return INVALID_KO_CONFIG;

    // check if the grpCfg matches the KO_Config
    if (grpCfg.size() != cfg.getNumGroups()) return INVALID_GROUP_NUM;
    int pairsInGrpCfg = 0;
    for (int i=0; i<grpCfg.size(); i++)
    {
      pairsInGrpCfg += grpCfg.at(i).size();
    }
    int pairsInCategory = getPlayerPairs().size();
    if (pairsInCategory != pairsInGrpCfg) return INVALID_PLAYER_COUNT;

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

        if (!isValid) return PLAYER_NOT_IN_CATEGORY;
      }
    }

    // okay, we're good to go!
    return OK;
  }

  //----------------------------------------------------------------------------

  ERR Category::canApplyInitialRanking(PlayerPairList seed)
  {
    if (getState() != STAT_CAT_FROZEN) return CATEGORY_NOT_YET_FROZEN;

    unique_ptr<Category> specializedCat = convertToSpecializedObject();
    if (!(specializedCat->needsInitialRanking()))
    {
      return CATEGORY_NEEDS_NO_SEEDING;
    }

    int pairsInCategory = getPlayerPairs().size();
    if (pairsInCategory != seed.size()) return INVALID_PLAYER_COUNT;

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

      if (!isValid) return PLAYER_NOT_IN_CATEGORY;
    }

    // okay, we're good to go!
    return OK;
  }

  //----------------------------------------------------------------------------

  ERR Category::applyGroupAssignment(vector<PlayerPairList> grpCfg)
  {
    ERR e = canApplyGroupAssignment(grpCfg);
    if (e != OK) return e;
    
    // The previous call checked for all possible errors or
    // misconfigurations. So we can safely write directly to the database
    DbTab* pairTab = db->getTab(TAB_PAIRS);
    int grpNum = 0;
    for (PlayerPairList ppl : grpCfg)
    {
      ++grpNum;   // we start counting groups with "1"
      for (PlayerPair pp : ppl)
      {
        int ppId = pp.getPairId();
        TabRow r = pairTab->operator [](ppId);
        r.update(PAIRS_GRP_NUM, grpNum);
      }
    }
    
    return OK;
  }

  //----------------------------------------------------------------------------

  ERR Category::applyInitialRanking(PlayerPairList seed)
  {
    ERR e = canApplyInitialRanking(seed);
    if (e != OK) return e;
    
    // The previous call checked for all possible errors or
    // misconfigurations. So we can safely write directly to the database
    DbTab* pairTab = db->getTab(TAB_PAIRS);
    for (int rank = 0; rank < seed.size(); ++rank)
    {
      int ppId = seed.at(rank).getPairId();
      TabRow r = pairTab->operator [](ppId);
      r.update(PAIRS_INITIAL_RANK, rank+1);   // we start counting ranks at "1"
    }
    
    return OK;
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
  ERR Category::generateGroupMatches(const PlayerPairList& grpMembers, int grpNum, int firstRoundNum, ProgressQueue *progressNotificationQueue) const
  {
    if ((grpNum < 1) && (grpNum != GROUP_NUM__ITERATION)) return INVALID_GROUP_NUM;

    RoundRobinGenerator rrg;
    MatchMngr mm{db};
    int numPlayers = grpMembers.size();
    int internalRoundNum = 0;
    while (true)
    {
      // create matches for the next round
      auto matches = rrg(numPlayers, internalRoundNum);

      // if no new matches were created, we have
      // covered all necessary rounds and can return
      if (matches.size() == 0) return OK;

      // create a match group for the new round
      ERR e;
      auto mg = mm.createMatchGroup(*this, firstRoundNum + internalRoundNum, grpNum, &e);
      if (e != OK) return e;

      // assign matches to this group
      for (auto match : matches)
      {
        int pairIndex1 = get<0>(match);
        int pairIndex2 = get<1>(match);

        PlayerPair pp1 = grpMembers.at(pairIndex1);
        PlayerPair pp2 = grpMembers.at(pairIndex2);

        auto newMatch = mm.createMatch(*mg, &e);
        if (e != OK) return e;

        e = mm.setPlayerPairsForMatch(*newMatch, pp1, pp2);
        if (e != OK) return e;

        if (progressNotificationQueue != nullptr) progressNotificationQueue->step();
      }

      // close this group (transition to FROZEN) and potentially promote it further to IDLE
      mm.closeMatchGroup(*mg);

      ++internalRoundNum;
    }

    return OK;  // should never be reached, but anyway...
  }

  //----------------------------------------------------------------------------

  /**
    Convenience function that generates a set of bracket matches for
    a list of PlayerPairs. This function does not do any error checking
    whether the PlayerPairs or other arguments are valid. It assumes
    that those checks have been performed before and that it's generally
    safe to generate the matches here and now.

    \param bracketMode is the type of bracket to use (e.g., single elimination)
    \param seeding is the initial list of player pairs, sorted from best player (index 0) to weakest player
    \param firstRoundNum the number of the first round of bracket matches
    \param progressNotificationQueue is an optional pointer to a FIFO that communicates progress back to the GUI

    \return error code
    */
  ERR Category::generateBracketMatches(int bracketMode, const PlayerPairList& seeding, int firstRoundNum, ProgressQueue* progressNotificationQueue) const
  {
    CatRoundStatus crs = getRoundStatus();
    if (firstRoundNum <= crs.getHighestGeneratedMatchRound()) return INVALID_ROUND;

    // generate the bracket data for the player list
    BracketGenerator gen{bracketMode};
    BracketMatchDataList bmdl;
    RawBracketVisDataDef visDataDef;
    gen.getBracketMatches(seeding.size(), bmdl, visDataDef);

    //
    // handle a special corner case here:
    //
    // If we are
    //   * in KO matches after round robins; and
    //   * we are configured to start directly with the finals; and
    //   * the second of each group qualifies
    //
    // then we have four initial players that DO NOT need a
    // semi final. The normal behavior of the BracketGenerator for
    // four players is to generate semi-finals and finals. But in this
    // special case, we only need the finals ("real" final and the
    // match for third place).
    //
    // So if the conditions above are fulfilled, we regenerate a
    // new set of matches. This is not very elegant (since it should
    // be solved in the BracketGenerator) but efficient...
    //
    if (getMatchSystem() == GROUPS_WITH_KO)
    {
      KO_Config cfg = KO_Config(getParameter_string(GROUP_CONFIG));
      if ((cfg.getStartLevel() == FINAL) && (cfg.getSecondSurvives()))
      {
        // we start with finals, which is simply "first vs. second"
        BracketMatchData final = BracketMatchData::getNew();
        final.setInitialRanks(1, 2);
        final.nextMatchForLoser = -2;
        final.nextMatchForWinner = -1;
        final.depthInBracket = 0;

        RawBracketVisElement visFinal;
        visFinal.page = 0;
        visFinal.gridX0 = 2;
        visFinal.gridY0 = 0;
        visFinal.ySpan = 2;
        visFinal.yPageBreakSpan = 0;
        visFinal.orientation = BRACKET_ORIENTATION::RIGHT;
        visFinal.terminator = BRACKET_TERMINATOR::OUTWARDS;
        visFinal.terminatorOffsetY = 0;

        // match for third place
        BracketMatchData thirdPlaceMatch = BracketMatchData::getNew();
        thirdPlaceMatch.setInitialRanks(3, 4);
        thirdPlaceMatch.nextMatchForLoser = -4;
        thirdPlaceMatch.nextMatchForWinner = -3;
        thirdPlaceMatch.depthInBracket = 0;

        RawBracketVisElement visThird;
        visThird.page = 0;
        visThird.gridX0 = 2;
        visThird.gridY0 = 1;
        visThird.ySpan = 2;
        visThird.yPageBreakSpan = 0;
        visThird.orientation = BRACKET_ORIENTATION::LEFT;
        visThird.terminator = BRACKET_TERMINATOR::OUTWARDS;
        visThird.terminatorOffsetY = 0;

        // replace all previously generated matches with these two
        bmdl.clear();
        bmdl.push_back(final);
        bmdl.push_back(thirdPlaceMatch);

        visDataDef.clear();
        visDataDef.addPage(BRACKET_PAGE_ORIENTATION::LANDSCAPE, BRACKET_LABEL_POS::TOP_LEFT);
        visDataDef.addElement(visFinal);
        visDataDef.addElement(visThird);
      }
    }

    // prepare the notification queue, if any
    if (progressNotificationQueue != nullptr)
    {
      progressNotificationQueue->reset(bmdl.size() * 2);
    }

    // sort the bracket data so that we have early rounds first
    //
    // std::sort constantly produces memory leaks by reading / writing beyond the end of the list. So I've
    // finally decided to use my own primitive sorting algorithm that is optimized on simplicity, not efficiency
    //
    //std::sort(bmdl.begin(), bmdl.end(), BracketGenerator::getBracketMatchSortFunction_earlyRoundsFirst());
    lazyAndInefficientVectorSortFunc<BracketMatchData>(bmdl, BracketGenerator::getBracketMatchSortFunction_earlyRoundsFirst());

    // create match groups and matches "from left to right"
    MatchMngr mm{db};
    int curRound = -1;
    int curDepth = -1;
    unique_ptr<MatchGroup> curGroup = nullptr;
    QHash<int, int> bracket2Match;
    for (BracketMatchData bmd : bmdl)
    {
      // skip unused matches
      if (bmd.matchDeleted)
      {
        continue;
      }

      // do we have to start a new round / group?
      if (bmd.depthInBracket != curDepth)
      {
        if (curGroup != nullptr)
        {
          mm.closeMatchGroup(*curGroup);
        }

        curDepth = bmd.depthInBracket;
        ++curRound;

        // determine the number for the new match group
        int grpNum = GROUP_NUM__ITERATION;
        switch (curDepth)
        {
        case 0:
          grpNum = GROUP_NUM__FINAL;
          break;
        case 1:
          grpNum = GROUP_NUM__SEMIFINAL;
          break;
        case 2:
          grpNum = GROUP_NUM__QUARTERFINAL;
          break;
        case 3:
          grpNum = GROUP_NUM__L16;
          break;
        }

        // create the match group
        ERR err;
        curGroup = mm.createMatchGroup(*this, firstRoundNum+curRound, grpNum, &err);
        assert(err == OK);
        assert(curGroup != nullptr);
      }

      // create a new, empty match in this group and map it to the bracket match id
      ERR err;
      auto ma = mm.createMatch(*curGroup, &err);
      assert(err == OK);
      assert(ma != nullptr);

      bracket2Match.insert(bmd.getBracketMatchId(), ma->getId());

      if (progressNotificationQueue != nullptr) progressNotificationQueue->step();
    }
    // close the last open match group (the finals)
    if (curGroup != nullptr) mm.closeMatchGroup(*curGroup);

    // a little helper function that returns an iterator to a match with
    // a given ID
    auto getMatchById = [&bmdl](int matchId) {
      BracketMatchDataList::iterator i = bmdl.begin();
      while (i != bmdl.end())
      {
        if ((*i).getBracketMatchId() == matchId) return i;
        ++i;
      }
      return i;
    };

    // fill the empty matches with the right values
    for (const BracketMatchData& bmd : bmdl)
    {
      // skip unused matches
      if (bmd.matchDeleted)
      {
        continue;
      }

      // "zero" is invalid for initialRank!
      assert(bmd.initialRank_Player1 != 0);
      assert(bmd.initialRank_Player2 != 0);

      auto ma = mm.getMatch(bracket2Match.value(bmd.getBracketMatchId()));
      assert(ma != nullptr);

      // case 1: we have "real" players that we can use
      if ((bmd.initialRank_Player1 > 0) && (bmd.initialRank_Player1 <= seeding.size()))
      {
        PlayerPair pp = seeding.at(bmd.initialRank_Player1 - 1);
        mm.setPlayerPairForMatch(*ma, pp, 1);
      }
      if ((bmd.initialRank_Player2 > 0) && (bmd.initialRank_Player2 <= seeding.size()))
      {
        PlayerPair pp = seeding.at(bmd.initialRank_Player2 - 1);
        mm.setPlayerPairForMatch(*ma, pp, 2);
      }

      // case 2: we have "symbolic" values like "winner of bracket match XYZ"
      if (bmd.initialRank_Player1 < 0)
      {
        int srcBracketMatchId = -(bmd.initialRank_Player1);
        BracketMatchData srcBracketMatch = *(getMatchById(srcBracketMatchId));

        int srcDatabaseMatchId = bracket2Match.value(srcBracketMatchId);
        auto srcDatabaseMatch = mm.getMatch(srcDatabaseMatchId);
        assert(srcDatabaseMatch != nullptr);

        if (srcBracketMatch.nextMatchForWinner == bmd.getBracketMatchId())  // player 1 of bmd is the winner of srcMatch
        {
          assert(srcBracketMatch.nextMatchPlayerPosForWinner == 1);
          mm.setSymbolicPlayerForMatch(*srcDatabaseMatch, *ma, true, 1);
        } else {
          // player 1 of bmd is the loser of srcMatch
          assert(srcBracketMatch.nextMatchPlayerPosForLoser == 1);
          mm.setSymbolicPlayerForMatch(*srcDatabaseMatch, *ma, false, 1);
        }
      }
      if (bmd.initialRank_Player2 < 0)
      {
        int srcBracketMatchId = -(bmd.initialRank_Player2);
        BracketMatchData srcBracketMatch = *(getMatchById(srcBracketMatchId));

        int srcDatabaseMatchId = bracket2Match.value(srcBracketMatchId);
        auto srcDatabaseMatch = mm.getMatch(srcDatabaseMatchId);

        if (srcBracketMatch.nextMatchForWinner == bmd.getBracketMatchId())  // player 2 of bmd is the winner of srcMatch
        {
          assert(srcBracketMatch.nextMatchPlayerPosForWinner == 2);
          mm.setSymbolicPlayerForMatch(*srcDatabaseMatch, *ma, true, 2);
        } else {
          // player 2 of bmd is the loser of srcMatch
          assert(srcBracketMatch.nextMatchPlayerPosForLoser == 2);
          mm.setSymbolicPlayerForMatch(*srcDatabaseMatch, *ma, false, 2);
        }
      }

      // case 3 (rare): only one player is used and the match does not need to be played,
      // BUT the match contains information about the final rank of the one player
      if ((bmd.initialRank_Player1 == BracketMatchData::UNUSED_PLAYER) && (bmd.nextMatchForWinner < 0))
      {
        mm.setPlayerToUnused(*ma, 1, -(bmd.nextMatchForWinner));
      }
      if ((bmd.initialRank_Player2 == BracketMatchData::UNUSED_PLAYER) && (bmd.nextMatchForWinner < 0))
      {
        mm.setPlayerToUnused(*ma, 2, -(bmd.nextMatchForWinner));
      }

      // last step: perhaps we have final ranks for winner and/or loser
      if (bmd.nextMatchForWinner < 0)
      {
        mm.setRankForWinnerOrLoser(*ma, true, -(bmd.nextMatchForWinner));
      }
      if (bmd.nextMatchForLoser < 0)
      {
        mm.setRankForWinnerOrLoser(*ma, false, -(bmd.nextMatchForLoser));
      }

      if (progressNotificationQueue != nullptr) progressNotificationQueue->step();
    }

    //
    // copy visualization info (if available) to the database
    //

    if ((visDataDef.getNumPages() > 0) && (visDataDef.getNumElements() > 0))
    {
      // create a new visualization entry
      upBracketVisData bvd;
      for (int i=0; i < visDataDef.getNumPages(); ++i)
      {
        BRACKET_PAGE_ORIENTATION orientation;
        BRACKET_LABEL_POS lp;
        tie(orientation, lp) = visDataDef.getPageInfo(i);

        if (i == 0)
        {
          bvd = BracketVisData::createNew(*this, orientation, lp);
        }

        assert(bvd != nullptr);

        if (i > 0)
        {
          bvd->addPage(orientation, lp);
        }
      }
      for (int i=0; i < visDataDef.getNumElements(); ++i)
      {
        RawBracketVisElement el = visDataDef.getElement(i);
        bvd->addElement(i + 1, el);    // bracket match IDs are 1-based, not 0-based!
      }

      // link actual matches to the bracket elements
      for (int i=0; i < visDataDef.getNumElements(); ++i)
      {
        if (bracket2Match.keys().contains(i+1))    // bracket match IDs are 1-based, not 0-based!
        {
          int maId = bracket2Match.value(i+1);     // bracket match IDs are 1-based, not 0-based!
          auto ma = mm.getMatch(maId);

          auto bracketElement = bvd->getVisElement(i+1);   // bracket match IDs are 1-based, not 0-based!
          assert(bracketElement != nullptr);

          assert(bracketElement->linkToMatch(*ma));
        }
      }

      // fill empty bracket matches with names as good as possible
      // for now
      bvd->fillMissingPlayerNames();
    }
    return OK;
  }

  //----------------------------------------------------------------------------

  int Category::getGroupNumForPredecessorRound(const int grpNum) const
  {
    // a regular players group
    if (grpNum > 0) return grpNum;

    // a regular round, e.g. in swiss ladder or random matches
    if (grpNum == GROUP_NUM__ITERATION) return GROUP_NUM__ITERATION;

    // in elimination categories, everything before "L16" is "Iteration"
    // and the rest follows the normal KO-logic
    MATCH_SYSTEM mSys = getMatchSystem();
    if (mSys == SINGLE_ELIM)
    {
      switch(grpNum)
      {
      case GROUP_NUM__FINAL:
        return GROUP_NUM__SEMIFINAL;
      case GROUP_NUM__SEMIFINAL:
        return GROUP_NUM__QUARTERFINAL;
      case GROUP_NUM__QUARTERFINAL:
        return GROUP_NUM__L16;
      }
      return GROUP_NUM__ITERATION;
    }

    // if we made it to this point, we are in KO rounds.
    // so we need the KO-config to decide if there is a previous
    // KO round or if we fall back to round robins
    KO_Config cfg = KO_Config(getParameter_string(GROUP_CONFIG));
    KO_START startLvl = cfg.getStartLevel();

    if (startLvl == FINAL) return ANY_PLAYERS_GROUP_NUMBER;

    if (startLvl == SEMI)
    {
      if (grpNum == GROUP_NUM__FINAL) return GROUP_NUM__SEMIFINAL;
    }

    if (startLvl == QUARTER)
    {
      switch (grpNum)
      {
      case GROUP_NUM__FINAL:
        return GROUP_NUM__SEMIFINAL;
      case GROUP_NUM__SEMIFINAL:
        return GROUP_NUM__QUARTERFINAL;
      }
    }

    if (startLvl == L16)
    {
      switch (grpNum)
      {
      case GROUP_NUM__FINAL:
        return GROUP_NUM__SEMIFINAL;
      case GROUP_NUM__SEMIFINAL:
        return GROUP_NUM__QUARTERFINAL;
      case GROUP_NUM__QUARTERFINAL:
        return GROUP_NUM__L16;
      }
    }

    return ANY_PLAYERS_GROUP_NUMBER;
  }



  //----------------------------------------------------------------------------

  CatRoundStatus Category::getRoundStatus() const
  {
    return CatRoundStatus(db, *this);
  }

  //----------------------------------------------------------------------------

  bool Category::hasMatchesInState(OBJ_STATE stat, int round) const
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

  PlayerPairList Category::getEliminatedPlayersAfterRound(int round, ERR* err) const
  {
    //
    // Approach: determine the list of eliminated players by
    // subtracting the remaining players from the full list of players
    //

    auto specialCat = convertToSpecializedObject();

    ERR e;
    PlayerPairList remainingPlayers = specialCat->getRemainingPlayersAfterRound(round, &e);
    if (e != OK)
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

    if (err != nullptr) *err = OK;
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
    bool isDrawAllowed = getParameter_bool(ALLOW_DRAW);
    if (!isDrawAllowed)
    {
      return false;
    }


    //
    // everything below this point can only be reached if the
    // "ALLOW_DRAW" parameter is true
    //


    // in any kind of "bracket match", draws are not possible.
    // So we need to have a "decision game", if necessary
    MATCH_SYSTEM ms = getMatchSystem();
    if ((ms == RANKING) || (ms == SINGLE_ELIM))
    {
      return false;
    }

    // in swiss ladder or random matches, draws are okay
    if ((ms == SWISS_LADDER) || (ms == RANDOMIZE))
    {
      return true;
    }

    // in round-robins with subsequent KO matches, it depends on the round
    // we're in
    if (ms == GROUPS_WITH_KO)
    {
      // invalid parameter
      if (round < 1) return false;

      KO_Config cfg = KO_Config(getParameter_string(GROUP_CONFIG));
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

  QString Category::getBracketVisDataString() const
  {
    return QString::fromUtf8(row[CAT_BRACKET_VIS_DATA].data());
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

}
