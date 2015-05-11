/* 
 * File:   Category.cpp
 * Author: volker
 * 
 * Created on March 2, 2014, 6:13 PM
 */

#include "Category.h"
#include "TournamentDataDefs.h"
#include "TournamentDB.h"
#include "TournamentErrorCodes.h"
#include "CatMngr.h"
#include "Tournament.h"
#include "RoundRobinCategory.h"
#include "RoundRobinGenerator.h"
#include "Match.h"
#include "CatRoundStatus.h"
#include "BracketGenerator.h"
#include "ElimCategory.h"

#include <stdexcept>

namespace QTournament
{

  Category::Category(TournamentDB* db, int rowId)
  :GenericDatabaseObject(db, TAB_CATEGORY, rowId)
  {
  }

  //----------------------------------------------------------------------------

  Category::Category(TournamentDB* db, dbOverlay::TabRow row)
  :GenericDatabaseObject(db, row)
  {
  }

  //----------------------------------------------------------------------------

  QString Category::getName() const
  {
    return row[GENERIC_NAME_FIELD_NAME].toString();
  }

  //----------------------------------------------------------------------------

  ERR Category::rename(const QString& nn)
  {
    return Tournament::getCatMngr()->renameCategory(*this, nn);
  }

  //----------------------------------------------------------------------------

  MATCH_SYSTEM Category::getMatchSystem() const
  {
    int sysInt = row[CAT_SYS].toInt();

    return static_cast<MATCH_SYSTEM>(sysInt);
  }

  //----------------------------------------------------------------------------

  MATCH_TYPE Category::getMatchType() const
  {
    int typeInt = row[CAT_MATCH_TYPE].toInt();

    return static_cast<MATCH_TYPE>(typeInt);
  }

  //----------------------------------------------------------------------------

  SEX Category::getSex() const
  {
    int sexInt = row[CAT_SEX].toInt();

    return static_cast<SEX>(sexInt);
  }

  //----------------------------------------------------------------------------

  ERR Category::setMatchSystem(MATCH_SYSTEM s)
  {
    return Tournament::getCatMngr()->setMatchSystem(*this, s);
  }

  //----------------------------------------------------------------------------

  ERR Category::setMatchType(MATCH_TYPE t)
  {
    return Tournament::getCatMngr()->setMatchType(*this, t);
  }

  //----------------------------------------------------------------------------

  ERR Category::setSex(SEX s)
  {
    return Tournament::getCatMngr()->setSex(*this, s);
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
    return Tournament::getCatMngr()->addPlayerToCategory(p, *this);
  }

  //----------------------------------------------------------------------------

  bool Category::hasPlayer(const Player& p) const
  {
    QVariantList qvl;
    qvl << P2C_PLAYER_REF << p.getId();
    qvl << P2C_CAT_REF << getId();

    return (db->getTab(TAB_P2C).getMatchCountForColumnValue(qvl) > 0);
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
    return Tournament::getCatMngr()->removePlayerFromCategory(p, *this);
  }


  //----------------------------------------------------------------------------

  QVariant Category::getParameter(CAT_PARAMETER p) const
  {
    switch (p) {

    case ALLOW_DRAW:
      return row[CAT_ACCEPT_DRAW];

    case WIN_SCORE:
      return row[CAT_WIN_SCORE];

    case DRAW_SCORE:
      return row[CAT_DRAW_SCORE];

    case GROUP_CONFIG:
      return row[CAT_GROUP_CONFIG];
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
    return Tournament::getCatMngr()->setCatParameter(*this, p, v);
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
    QList<PlayerPair> result;
    PlayerMngr* pmngr = Tournament::getPlayerMngr();

    // get all players assigned to this category
    QList<Player> singlePlayers = getAllPlayersInCategory();

    // filter out the players that are paired and have already
    // entries in the database
    QVariantList qvl;
    qvl << PAIRS_CAT_REF << getId();
    if (grp > 0) qvl << PAIRS_GRP_NUM << grp;
    DbTab::CachingRowIterator it = db->getTab(TAB_PAIRS).getRowsByColumnValue(qvl);
    while (!(it.isEnd()))
    {
      int id1 = (*it)[PAIRS_PLAYER1_REF].toInt();
      Player p1 = pmngr->getPlayer(id1);
      singlePlayers.removeAll(p1);

      // id2 is sometimes empty, e.g. in singles categories
      QVariant qv = (*it)[PAIRS_PLAYER2_REF];
      if (qv.isNull()) {
        result.append(PlayerPair(p1, (*it).getId()));
      } else {
        int id2 = qv.toInt();
        Player p2 = pmngr->getPlayer(id2);
        result.append(PlayerPair(p1, p2, (*it).getId()));
        singlePlayers.removeAll(p2);
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
    for (int i=0; i < singlePlayers.count(); i++)
    {
      result.append(PlayerPair(singlePlayers.at(i)));
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

    DbTab pairTab = (*db)[TAB_PAIRS];
    QVariantList qvl;
    qvl << PAIRS_CAT_REF << getId();
    if (grp > 0) qvl << PAIRS_GRP_NUM << grp;
    return pairTab.getMatchCountForColumnValue(qvl);
  }

  //----------------------------------------------------------------------------

  QList<Player> Category::getAllPlayersInCategory() const
  {
    QList<Player> result;
    PlayerMngr* pmngr = Tournament::getPlayerMngr();

    DbTab::CachingRowIterator it = db->getTab(TAB_P2C).getRowsByColumnValue(P2C_CAT_REF, getId());
    while (!(it.isEnd()))
    {
      result.append(pmngr->getPlayer((*it)[P2C_PLAYER_REF].toInt()));
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
    QString whereClause = "(" + PAIRS_PLAYER1_REF + " = ? OR " + PAIRS_PLAYER2_REF + " = ?) ";
    whereClause += "AND (" + PAIRS_CAT_REF + " = ?)";
    QVariantList qvl;
    qvl << p.getId();
    qvl << p.getId();
    qvl << getId();

    // see if we have a row that matches the query
    DbTab pairTab = db->getTab(TAB_PAIRS);
    return (pairTab.getMatchCountForWhereClause(whereClause, qvl) != 0);
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
    QVariantList qvl;
    qvl << PAIRS_CAT_REF << getId();
    qvl << PAIRS_PLAYER1_REF << p1.getId();
    qvl << PAIRS_PLAYER2_REF << p2.getId();
    DbTab pairsTab = db->getTab(TAB_PAIRS);
    if (pairsTab.getMatchCountForColumnValue(qvl) != 0)
    {
      return OK;
    }

    // swap player 1 and player 2 and make a new query
    qvl.clear();
    qvl << PAIRS_CAT_REF << getId();
    qvl << PAIRS_PLAYER1_REF << p2.getId();
    qvl << PAIRS_PLAYER2_REF << p1.getId();
    if (pairsTab.getMatchCountForColumnValue(qvl) != 0)
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
    QString whereClause = "(" + PAIRS_PLAYER1_REF + " = ? OR " + PAIRS_PLAYER2_REF + " = ?) ";
    whereClause += "AND (" + PAIRS_CAT_REF + " = ?)";
    QVariantList qvl;
    qvl << p.getId();
    qvl << p.getId();
    qvl << getId();

    // see if we have a row that matches the query
    int partnerId = -1;
    DbTab pairTab = db->getTab(TAB_PAIRS);
    try
    {
      TabRow r = pairTab.getSingleRowByWhereClause(whereClause, qvl);
      partnerId = (r[PAIRS_PLAYER1_REF].toInt() == p.getId()) ? r[PAIRS_PLAYER2_REF].toInt() : r[PAIRS_PLAYER1_REF].toInt();
    } catch (exception e) {
      throw std::invalid_argument("Player doesn't have a partner");
    }

    return Tournament::getPlayerMngr()->getPlayer(partnerId);
  }

  //----------------------------------------------------------------------------

  bool Category::hasUnpairedPlayers() const
  {
    PlayerPairList pp = getPlayerPairs();
    for (int i=0; i < pp.count(); i++)
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

    // THIS IS JUST A HOT FIX UNTIL WE HAVE
    // SPECIALIZED CLASSED FOR ALL MATCH SYSTEMS!!!
    //
    // Returning an object of the base class instead of the derived class
    // will end up in exceptions and abnormal program termination
    return unique_ptr<Category>(new Category(db, row));
  }

  //----------------------------------------------------------------------------

  ERR Category::canApplyGroupAssignment(QList<PlayerPairList> grpCfg)
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
    if (grpCfg.count() != cfg.getNumGroups()) return INVALID_GROUP_NUM;
    int pairsInGrpCfg = 0;
    for (int i=0; i<grpCfg.count(); i++)
    {
      pairsInGrpCfg += grpCfg.at(i).count();
    }
    int pairsInCategory = getPlayerPairs().count();
    if (pairsInCategory != pairsInGrpCfg) return INVALID_PLAYER_COUNT;

    // make sure we only have player pairs that actually belong to this category
    PlayerPairList allPairs = getPlayerPairs();
    for (int i=0; i<grpCfg.count(); i++)
    {
      PlayerPairList thisGroup = grpCfg.at(i);

      for (int cnt=0; cnt < thisGroup.count(); cnt++)
      {
        int ppId = thisGroup.at(cnt).getPairId();

        bool isValid = false;
        for (int n=0; n < allPairs.count(); n++)
        {
          if (allPairs.at(n).getPairId() == ppId)
          {
            isValid = true;
            allPairs.removeAt(n);  // reduce the number of checks for the next loop
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

    int pairsInCategory = getPlayerPairs().count();
    if (pairsInCategory != seed.count()) return INVALID_PLAYER_COUNT;

    // make sure we only have player pairs that actually belong to this category
    PlayerPairList allPairs = getPlayerPairs();
    for (int i=0; i<seed.count(); i++)
    {
      int ppId = seed.at(i).getPairId();

      bool isValid = false;
      for (int n=0; n < allPairs.count(); n++)
      {
        if (allPairs.at(n).getPairId() == ppId)
        {
          isValid = true;
          allPairs.removeAt(n);  // reduce the number of checks for the next loop
          break;
        }
      }

      if (!isValid) return PLAYER_NOT_IN_CATEGORY;
    }

    // okay, we're good to go!
    return OK;
  }

  //----------------------------------------------------------------------------

  ERR Category::applyGroupAssignment(QList<PlayerPairList> grpCfg)
  {
    ERR e = canApplyGroupAssignment(grpCfg);
    if (e != OK) return e;
    
    // The previous call checked for all possible errors or
    // misconfigurations. So we can safely write directly to the database
    DbTab pairTab = (*db)[TAB_PAIRS];
    int grpNum = 0;
    for (PlayerPairList ppl : grpCfg)
    {
      ++grpNum;   // we start counting groups with "1"
      for (PlayerPair pp : ppl)
      {
        int ppId = pp.getPairId();
        TabRow r = pairTab[ppId];
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
    DbTab pairTab = (*db)[TAB_PAIRS];
    for (int rank = 0; rank < seed.count(); ++rank)
    {
      int ppId = seed.at(rank).getPairId();
      TabRow r = pairTab[ppId];
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
    if (grpNum < 1) return INVALID_GROUP_NUM;

    RoundRobinGenerator rrg;
    auto mm = Tournament::getMatchMngr();
    int numPlayers = grpMembers.count();
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
      auto mg = mm->createMatchGroup(*this, firstRoundNum + internalRoundNum, grpNum, &e);
      if (e != OK) return e;

      // assign matches to this group
      for (auto match : matches)
      {
        int pairIndex1 = get<0>(match);
        int pairIndex2 = get<1>(match);

        PlayerPair pp1 = grpMembers.at(pairIndex1);
        PlayerPair pp2 = grpMembers.at(pairIndex2);

        auto newMatch = mm->createMatch(*mg, &e);
        if (e != OK) return e;

        e = mm->setPlayerPairsForMatch(*newMatch, pp1, pp2);
        if (e != OK) return e;

        if (progressNotificationQueue != nullptr) progressNotificationQueue->step();
      }

      // close this group (transition to FROZEN) and potentially promote it further to IDLE
      mm->closeMatchGroup(*mg);

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
    BracketMatchDataList bmdl = gen.getBracketMatches(seeding.size());

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
        BracketMatchData final;
        final.setInitialRanks(1, 2);
        final.nextMatchForLoser = -2;
        final.nextMatchForWinner = -1;
        final.depthInBracket = 0;

        // match for third place
        BracketMatchData thirdPlaceMatch;
        thirdPlaceMatch.setInitialRanks(3, 4);
        thirdPlaceMatch.nextMatchForLoser = -4;
        thirdPlaceMatch.nextMatchForWinner = -3;
        thirdPlaceMatch.depthInBracket = 0;

        // replace all previously generated matches with these two
        bmdl.clear();
        bmdl.push_back(final);
        bmdl.push_back(thirdPlaceMatch);
      }
    }

    // prepare the notification queue, if any
    if (progressNotificationQueue != nullptr)
    {
      progressNotificationQueue->reset(bmdl.size() * 2);
    }

    // sort the bracket data so that we have early rounds first
    std::sort(bmdl.begin(), bmdl.end(), BracketGenerator::getBracketMatchSortFunction_earlyRoundsFirst());

    // create match groups and matches "from left to right"
    MatchMngr* mm = Tournament::getMatchMngr();
    int curRound = -1;
    int curDepth = -1;
    unique_ptr<MatchGroup> curGroup = nullptr;
    QHash<int, int> bracket2Match;
    for (BracketMatchData bmd : bmdl)
    {
      // do we have to start a new round / group?
      if (bmd.depthInBracket != curDepth)
      {
        if (curGroup != nullptr)
        {
          mm->closeMatchGroup(*curGroup);
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
        curGroup = mm->createMatchGroup(*this, firstRoundNum+curRound, grpNum, &err);
        assert(err == OK);
        assert(curGroup != nullptr);
      }

      // create a new, empty match in this group and map it to the bracket match id
      ERR err;
      auto ma = mm->createMatch(*curGroup, &err);
      assert(err == OK);
      assert(ma != nullptr);

      bracket2Match.insert(bmd.getBracketMatchId(), ma->getId());

      if (progressNotificationQueue != nullptr) progressNotificationQueue->step();
    }
    // close the last open match group (the finals)
    if (curGroup != nullptr) mm->closeMatchGroup(*curGroup);

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
    for (BracketMatchData bmd : bmdl)
    {
      // "zero" is invalid for initialRank!
      assert(bmd.initialRank_Player1 != 0);
      assert(bmd.initialRank_Player2 != 0);

      auto ma = mm->getMatch(bracket2Match.value(bmd.getBracketMatchId()));
      assert(ma != nullptr);

      // case 1: we have "real" players that we can use
      if ((bmd.initialRank_Player1 > 0) && (bmd.initialRank_Player1 <= seeding.size()))
      {
        PlayerPair pp = seeding.at(bmd.initialRank_Player1 - 1);
        mm->setPlayerPairForMatch(*ma, pp, 1);
      }
      if ((bmd.initialRank_Player2 > 0) && (bmd.initialRank_Player2 <= seeding.size()))
      {
        PlayerPair pp = seeding.at(bmd.initialRank_Player2 - 1);
        mm->setPlayerPairForMatch(*ma, pp, 2);
      }

      // case 2: we have "symbolic" values like "winner of bracket match XYZ"
      if (bmd.initialRank_Player1 < 0)
      {
        int srcBracketMatchId = -(bmd.initialRank_Player1);
        BracketMatchData srcBracketMatch = *(getMatchById(srcBracketMatchId));

        int srcDatabaseMatchId = bracket2Match.value(srcBracketMatchId);
        auto srcDatabaseMatch = mm->getMatch(srcDatabaseMatchId);
        assert(srcDatabaseMatch != nullptr);

        if (srcBracketMatch.nextMatchForWinner == bmd.getBracketMatchId())  // player 1 of bmd is the winner of srcMatch
        {
          assert(srcBracketMatch.nextMatchPlayerPosForWinner == 1);
          mm->setSymbolicPlayerForMatch(*srcDatabaseMatch, *ma, true, 1);
        } else {
          // player 1 of bmd is the loser of srcMatch
          assert(srcBracketMatch.nextMatchPlayerPosForLoser == 1);
          mm->setSymbolicPlayerForMatch(*srcDatabaseMatch, *ma, false, 1);
        }
      }
      if (bmd.initialRank_Player2 < 0)
      {
        int srcBracketMatchId = -(bmd.initialRank_Player2);
        BracketMatchData srcBracketMatch = *(getMatchById(srcBracketMatchId));

        int srcDatabaseMatchId = bracket2Match.value(srcBracketMatchId);
        auto srcDatabaseMatch = mm->getMatch(srcDatabaseMatchId);

        if (srcBracketMatch.nextMatchForWinner == bmd.getBracketMatchId())  // player 2 of bmd is the winner of srcMatch
        {
          assert(srcBracketMatch.nextMatchPlayerPosForWinner == 2);
          mm->setSymbolicPlayerForMatch(*srcDatabaseMatch, *ma, true, 2);
        } else {
          // player 2 of bmd is the loser of srcMatch
          assert(srcBracketMatch.nextMatchPlayerPosForLoser == 2);
          mm->setSymbolicPlayerForMatch(*srcDatabaseMatch, *ma, false, 2);
        }
      }

      // case 3 (rare): only one player is used and the match does not need to be played,
      // BUT the match contains information about the final rank of the one player
      if ((bmd.initialRank_Player1 == BracketMatchData::UNUSED_PLAYER) && (bmd.nextMatchForWinner < 0))
      {
        mm->setPlayerToUnused(*ma, 1, -(bmd.nextMatchForWinner));
      }
      if ((bmd.initialRank_Player2 == BracketMatchData::UNUSED_PLAYER) && (bmd.nextMatchForWinner < 0))
      {
        mm->setPlayerToUnused(*ma, 2, -(bmd.nextMatchForWinner));
      }

      // last step: perhaps we have final ranks for winner and/or loser
      if (bmd.nextMatchForWinner < 0)
      {
        mm->setRankForWinnerOrLoser(*ma, true, -(bmd.nextMatchForWinner));
      }
      if (bmd.nextMatchForLoser < 0)
      {
        mm->setRankForWinnerOrLoser(*ma, false, -(bmd.nextMatchForLoser));
      }

      if (progressNotificationQueue != nullptr) progressNotificationQueue->step();
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
    MatchMngr* mm = Tournament::getMatchMngr();

    MatchGroupList mgl = mm->getMatchGroupsForCat(*this, round);
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
      if (remainingPlayers.contains(pp)) continue;
      eliminatedPlayers.append(pp);
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


  //----------------------------------------------------------------------------


  //----------------------------------------------------------------------------


  //----------------------------------------------------------------------------


  //----------------------------------------------------------------------------


  //----------------------------------------------------------------------------


  //----------------------------------------------------------------------------


  //----------------------------------------------------------------------------


  //----------------------------------------------------------------------------

}
