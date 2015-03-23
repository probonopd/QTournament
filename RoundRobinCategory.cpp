/* 
 * File:   RoundRobinCategory.cpp
 * Author: volker
 * 
 * Created on August 25, 2014, 8:34 PM
 */

#include "RoundRobinCategory.h"
#include "KO_Config.h"
#include "Tournament.h"

#include <QDebug>

using namespace dbOverlay;

namespace QTournament
{

  RoundRobinCategory::RoundRobinCategory(TournamentDB* db, int rowId)
  : Category(db, rowId)
  {
    qDebug() << "!!  RR  !!";
  }

//----------------------------------------------------------------------------

  RoundRobinCategory::RoundRobinCategory(TournamentDB* db, dbOverlay::TabRow row)
  : Category(db, row)
  {
    qDebug() << "!!  RR  !!";
  }

//----------------------------------------------------------------------------

  ERR RoundRobinCategory::canFreezeConfig()
  {
    if (getState() != STAT_CAT_CONFIG)
    {
      return CONFIG_ALREADY_FROZEN;
    }
    
    // make sure there no unpaired players in singles or doubles
    if ((getMatchType() != SINGLES) && (hasUnpairedPlayers()))
    {
      return UNPAIRED_PLAYERS;
    }
    
    // make sure we have a valid group configuration
    PlayerPairList pp = getPlayerPairs();
    KO_Config cfg = KO_Config(getParameter_string(GROUP_CONFIG));
    if (!(cfg.isValid(pp.count())))
    {
      return INVALID_KO_CONFIG;
    }
    
    return OK;
  }

//----------------------------------------------------------------------------

  bool RoundRobinCategory::needsInitialRanking()
  {
    return false;
  }

//----------------------------------------------------------------------------

  bool RoundRobinCategory::needsGroupInitialization()
  {
    return true;
  }

//----------------------------------------------------------------------------

  ERR RoundRobinCategory::prepareFirstRound(ProgressQueue *progressNotificationQueue)
  {
    if (getState() != STAT_CAT_IDLE) return WRONG_STATE;

    auto mm = Tournament::getMatchMngr();
    auto pp = Tournament::getPlayerMngr();

    // make sure we have not been called before; to this end, just
    // check that there have no matches been created for us so far
    auto allGrp = mm->getMatchGroupsForCat(*this);
    // do not return an error here, because obviously we have been
    // called successfully before and we only want to avoid
    // double initialization
    if (allGrp.count() != 0) return OK;

    // alright, this is a virgin category. Generate group matches
    // for each group
    KO_Config cfg = KO_Config(getParameter_string(GROUP_CONFIG));
    if (progressNotificationQueue != nullptr)
    {
      progressNotificationQueue->reset(cfg.getNumGroupMatches());
    }
    for (int grpIndex = 0; grpIndex < cfg.getNumGroups(); ++grpIndex)
    {
      PlayerPairList grpMembers = getPlayerPairs(grpIndex+1);
      ERR e = generateGroupMatches(grpMembers, grpIndex+1, 1, progressNotificationQueue);
      if (e != OK) return e;
    }

    return OK;
  }

//----------------------------------------------------------------------------

  int RoundRobinCategory::calcTotalRoundsCount()
  {
    OBJ_STATE stat = getState();
    if ((stat == STAT_CAT_CONFIG) || (stat == STAT_CAT_FROZEN))
    {
      return -1;   // category not yet fully configured; can't calc rounds
    }

    // the following call must succeed, since we made it past the
    // configuration point
    KO_Config cfg = KO_Config(getParameter_string(GROUP_CONFIG));

    // the number of rounds is
    // (number of group rounds) + (number of KO rounds)
    int groupRounds = cfg.getNumRounds();

    KO_START startLevel = cfg.getStartLevel();
    int eliminationRounds = 1;  // finals
    if (startLevel != FINAL) ++eliminationRounds; // semi-finals for all, except we dive straight into finals
    if ((startLevel == QUARTER) || (startLevel == L16)) ++eliminationRounds;  // QF and last 16
    if (startLevel == L16) ++eliminationRounds;  // round of last 16

    return groupRounds + eliminationRounds;
  }

//----------------------------------------------------------------------------

  // this return a function that should return true if "a" goes before "b" when sorting. Read:
  // return a function that return true true if the score of "a" is better than "b"
  std::function<bool (RankingEntry& a, RankingEntry& b)> RoundRobinCategory::getLessThanFunction()
  {
    return [](RankingEntry& a, RankingEntry& b) {
      // first criteria: won matches
      tuple<int, int, int, int> matchStatsA = a.getMatchStats();
      tuple<int, int, int, int> matchStatsB = b.getMatchStats();
      if ((get<0>(matchStatsA)) > (get<0>(matchStatsB))) return true;

      // second criteria: won games
      tuple<int, int, int> gameStatsA = a.getGameStats();
      tuple<int, int, int> gameStatsB = b.getGameStats();
      if ((get<0>(gameStatsA)) > (get<0>(gameStatsB))) return true;

      // third criteria: more points
      tuple<int, int> pointStatsA = a.getPointStats();
      tuple<int, int> pointStatsB = b.getPointStats();
      if ((get<0>(pointStatsA)) > (get<0>(pointStatsB))) return true;

      // TODO: add a direct comparison as additional criteria?

      // Default: "a" is not strictly better than "b", so we return false
      return false;
    };
  }

//----------------------------------------------------------------------------

  ERR RoundRobinCategory::onRoundCompleted(int round)
  {
    // determine the number of group rounds.
    //
    // The following call must succeed, since we made it past the
    // configuration point
    KO_Config cfg = KO_Config(getParameter_string(GROUP_CONFIG));
    int groupRounds = cfg.getNumRounds();

    // if we are still in group rounds, simply calculate the
    // new ranking
    if (round <= groupRounds)
    {
      RankingMngr* rm = Tournament::getRankingMngr();
      ERR err;
      rm->createUnsortedRankingEntriesForLastRound(*this, &err);
      if (err != OK) return err;  // shouldn't happen
      rm->sortRankingEntriesForLastRound(*this, &err);
      if (err != OK) return err;  // shouldn't happen
    }

    // if this was the last round in group rounds,
    // we need to wait for user input (seeding)
    // before we can enter the KO rounds
    if (round == groupRounds)
    {
      Tournament::getCatMngr()->switchCatToWaitForSeeding(*this);
    }

    // TODO: add action for KO rounds
    return OK;
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


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


}
