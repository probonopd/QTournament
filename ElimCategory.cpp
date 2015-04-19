/* 
 * File:   RoundRobinCategory.cpp
 * Author: volker
 * 
 * Created on August 25, 2014, 8:34 PM
 */

#include "ElimCategory.h"
#include "KO_Config.h"
#include "Tournament.h"
#include "CatRoundStatus.h"
#include "RankingEntry.h"
#include "RankingMngr.h"
#include "assert.h"
#include "BracketGenerator.h"

#include <QDebug>

using namespace dbOverlay;

namespace QTournament
{

  EliminationCategory::EliminationCategory(TournamentDB* db, int rowId)
  : Category(db, rowId)
  {
    qDebug() << "!!  Elim  !!";
  }

//----------------------------------------------------------------------------

  EliminationCategory::EliminationCategory(TournamentDB* db, dbOverlay::TabRow row)
  : Category(db, row)
  {
    qDebug() << "!!  Elim  !!";
  }

//----------------------------------------------------------------------------

  ERR EliminationCategory::canFreezeConfig()
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
    
    return OK;
  }

//----------------------------------------------------------------------------

  bool EliminationCategory::needsInitialRanking()
  {
    return true;
  }

//----------------------------------------------------------------------------

  bool EliminationCategory::needsGroupInitialization()
  {
    return false;
  }

//----------------------------------------------------------------------------

  ERR EliminationCategory::prepareFirstRound(ProgressQueue *progressNotificationQueue)
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

    // alright, this is a virgin category. Generate bracket matches
    // for each group
    PlayerPairList seeding = Tournament::getCatMngr()->getSeeding(*this);
    return generateBracketMatches(BracketGenerator::BRACKET_SINGLE_ELIM, seeding, 1, progressNotificationQueue);
  }

//----------------------------------------------------------------------------

  int EliminationCategory::calcTotalRoundsCount() const
  {
    OBJ_STATE stat = getState();
    if ((stat == STAT_CAT_CONFIG) || (stat == STAT_CAT_FROZEN))
    {
      return -1;   // category not yet fully configured; can't calc rounds
    }

    BracketGenerator bg{BracketGenerator::BRACKET_SINGLE_ELIM};
    int numPairs = getDatabasePlayerPairCount();
    return bg.getNumRounds(numPairs);
  }

//----------------------------------------------------------------------------

  // this return a function that should return true if "a" goes before "b" when sorting. Read:
  // return a function that return true true if the score of "a" is better than "b"
  std::function<bool (RankingEntry& a, RankingEntry& b)> EliminationCategory::getLessThanFunction()
  {
    return [](RankingEntry& a, RankingEntry& b) {
      return false;   // there is no definite ranking in elimination rounds, so simply return a dummy value
    };
  }

//----------------------------------------------------------------------------

  ERR EliminationCategory::onRoundCompleted(int round)
  {
    // create ranking entries for everyone who played.
    // this is only to get the accumulated values for the finalists right
    ERR err;
    RankingMngr* rm = Tournament::getRankingMngr();
    rm->createUnsortedRankingEntriesForLastRound(*this, &err);
    if (err != OK) return err;

    CatRoundStatus crs = getRoundStatus();

    // there's nothing to do for us except after the last round.
    // after the last roound, we have to create ranking entries
    int lastFinishedRound = crs.getFinishedRoundsCount();
    if (lastFinishedRound != calcTotalRoundsCount())
    {
      return OK;
    }

    // set the ranks for the winner / losers of the finals
    MatchMngr* mm = Tournament::getMatchMngr();
    for (MatchGroup mg : mm->getMatchGroupsForCat(*this, lastFinishedRound))
    {
      for (Match ma : mg.getMatches())
      {
        auto winner = ma.getWinner();
        assert(winner != nullptr);
        auto re = rm->getRankingEntry(*winner, lastFinishedRound);
        assert(re != nullptr);
        int winnerRank = ma.getWinnerRank();
        assert(winnerRank > 0);
        rm->forceRank(*re, winnerRank);

        auto loser = ma.getLoser();
        assert(loser != nullptr);
        re = rm->getRankingEntry(*loser, lastFinishedRound);
        assert(re != nullptr);
        int loserRank = ma.getLoserRank();
        assert(loserRank > 0);
        rm->forceRank(*re, loserRank);
      }
    }

    return OK;
  }

//----------------------------------------------------------------------------

  ERR EliminationCategory::prepareNextRound(PlayerPairList seeding, ProgressQueue* progressNotificationQueue)
  {
    return NOTHING_TO_PREPARE;
  }

//----------------------------------------------------------------------------

  PlayerPairList EliminationCategory::getRemainingPlayersAfterRound(int round, ERR* err) const
  {
    if (round == calcTotalRoundsCount())
    {
      if (err != nullptr) *err = OK;
      return PlayerPairList();  // no remaining players after last round
    }

    // we can only determine remaining players after completed rounds
    CatRoundStatus crs = getRoundStatus();
    if (round > crs.getFinishedRoundsCount())
    {
      if (err != nullptr) *err = INVALID_ROUND;
      return PlayerPairList();
    }

    // get the list for the previous round, if any
    PlayerPairList result;
    if (round > 1)
    {
      ERR e;
      result = this->getRemainingPlayersAfterRound(round-1, &e);
      if (e != OK)
      {
        if (err != nullptr) *err = INVALID_ROUND;
        return PlayerPairList();
      }
    } else {
      result = getPlayerPairs();
    }

    // get the match losers of this round
    // and remove them from the list of the previous round
    //
    // exception: losers in semi-finals will continue in the
    // match for 3rd place
    if (round == (calcTotalRoundsCount() - 1))  // semi-finals
    {
      if (err != nullptr) *err = OK;
      return result;
    }
    MatchMngr* mm = Tournament::getMatchMngr();
    for (MatchGroup mg : mm->getMatchGroupsForCat(*this, round))
    {
      for (Match ma : mg.getMatches())
      {
        auto loser = ma.getLoser();
        if (loser == nullptr) continue;   // shouldn't happen
        result.removeAll(*loser);
      }
    }

    if (err != nullptr) *err = OK;
    return result;
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


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


}
