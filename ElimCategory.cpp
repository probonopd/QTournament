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

  EliminationCategory::EliminationCategory(TournamentDB* db, int rowId, int eliminationMode)
  : Category(db, rowId)
  {
    if ((eliminationMode != BracketGenerator::BRACKET_SINGLE_ELIM) &&
        (eliminationMode != BracketGenerator::BRACKET_DOUBLE_ELIM) &&
        (eliminationMode != BracketGenerator::BRACKET_RANKING1))
    {
      throw std::invalid_argument("Invalid elimination mode in ctor of EliminationCategory!");
    }

    elimMode = eliminationMode;
  }

//----------------------------------------------------------------------------

  EliminationCategory::EliminationCategory(TournamentDB* db, dbOverlay::TabRow row, int eliminationMode)
  : Category(db, row)
  {
    if ((eliminationMode != BracketGenerator::BRACKET_SINGLE_ELIM) &&
        (eliminationMode != BracketGenerator::BRACKET_DOUBLE_ELIM) &&
        (eliminationMode != BracketGenerator::BRACKET_RANKING1))
    {
      throw std::invalid_argument("Invalid elimination mode in ctor of EliminationCategory!");
    }

    elimMode = eliminationMode;
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

    // we should have at least two players / pairs
    int numPairs = getAllPlayersInCategory().size();
    if (getMatchType() != SINGLES)
    {
      numPairs = numPairs / 2;    // numPairs before division must be even, because we had no unpaired players (see check above)
    }
    if (numPairs < 2)
    {
      return INVALID_PLAYER_COUNT;
    }

    // for the bracket mode "ranking1" we may not have more
    // than 32 players
    if ((elimMode == BracketGenerator::BRACKET_RANKING1) && (numPairs > 32))
    {
      return INVALID_PLAYER_COUNT;
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
    return generateBracketMatches(elimMode, seeding, 1, progressNotificationQueue);
  }

//----------------------------------------------------------------------------

  int EliminationCategory::calcTotalRoundsCount() const
  {
    OBJ_STATE stat = getState();
    if ((stat == STAT_CAT_CONFIG) || (stat == STAT_CAT_FROZEN))
    {
      return -1;   // category not yet fully configured; can't calc rounds
    }

    BracketGenerator bg{elimMode};
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
    // create ranking entries for everyone who played
    // and for everyone who achieved a final rank in a
    // previous round
    ERR err;
    RankingMngr* rm = Tournament::getRankingMngr();
    PlayerPairList ppList;
    if (round == 1)
    {
      ppList = getPlayerPairs();
    } else {
      ppList = this->getRemainingPlayersAfterRound(round - 1, &err);
      if (err != OK) return err;
    }
    auto rll = rm->getSortedRanking(*this, round-1);
    for (auto rl : rll)
    {
      for (RankingEntry re : rl)
      {
        if (re.getRank() != RankingEntry::NO_RANK_ASSIGNED)
        {
          auto pp = re.getPlayerPair();
          assert(pp != nullptr);
          if (!(ppList.contains(*pp)))
          {
            ppList.append(*pp);
          }
        }
      }
    }

    // create unsorted entries for everyone who played in this round
    // or who achieved a final rank in a previous round
    rm->createUnsortedRankingEntriesForLastRound(*this, &err, ppList);
    if (err != OK) return err;

    // set the rank for all players that ended up at a final rank
    // in this or any prior round
    MatchMngr* mm = Tournament::getMatchMngr();
    for (int r=1; r <= round; ++r)
    {
      for (MatchGroup mg : mm->getMatchGroupsForCat(*this, r))
      {
        for (Match ma : mg.getMatches())
        {
          int winnerRank = ma.getWinnerRank();
          if (winnerRank > 0)
          {
            auto w = ma.getWinner();
            assert(w != nullptr);
            auto re = rm->getRankingEntry(*w, round);
            assert(re != nullptr);
            rm->forceRank(*re, winnerRank);
          }

          int loserRank = ma.getLoserRank();
          if (loserRank > 0)
          {
            auto l = ma.getLoser();
            assert(l != nullptr);
            auto re = rm->getRankingEntry(*l, round);
            assert(re != nullptr);
            rm->forceRank(*re, loserRank);
          }
        }
      }
    }

    return OK;
  }

//----------------------------------------------------------------------------

  PlayerPairList EliminationCategory::getRemainingPlayersAfterRound(int round, ERR* err) const
  {
    int lastRoundInThisCat = calcTotalRoundsCount();
    if (round == lastRoundInThisCat)
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
      // round 0 (before first round)
      if (err != nullptr) *err = OK;
      return getPlayerPairs();
    }

    // now that we have the survivors of the previous round (or
    // the initial list of all players if this is round one) we
    // sort out all players are not further used in future matches
    //
    // for this, we walk through all matches in this round and remove
    // those players that have no future matches.
    //
    // "no future match" can mean player "eliminated" or "ranked"
    MatchMngr* mm = Tournament::getMatchMngr();
    for (MatchGroup mg : mm->getMatchGroupsForCat(*this, round))
    {
      for (Match ma : mg.getMatches())
      {
        auto loser = ma.getLoser();
        assert(loser != nullptr);
        int loserPairId = loser->getPairId();
        assert(loserPairId > 0);

        auto winner = ma.getWinner();
        assert(winner != nullptr);
        int winnerPairId = winner->getPairId();
        assert(winnerPairId > 0);

        bool winnerOut = false;
        bool loserOut = false;

        // check 1: is there a final rank for the winner?
        if (ma.getWinnerRank() > 0)
        {
          result.removeAll(*winner);
          winnerOut = true;
        }

        // check 2: is there a final rank for the loser?
        if (ma.getLoserRank() > 0)
        {
          result.removeAll(*loser);
          loserOut = true;
        }

        //
        // Intermezzo: a helper function for searching
        // for future matches of a pair ID
        //
        DbTab matchTab = db->getTab(TAB_MATCH);
        auto hasFutureMatch = [&](const PlayerPair& pp, bool asWinner) {
          // step 1: search by pair
          for (int r=round+1; r <= lastRoundInThisCat; ++r)
          {
            auto next = mm->getMatchForPlayerPairAndRound(pp, r);
            if (next != nullptr)
            {
              return true;
            }
          }

          // step 2: search for "is winner of" or "is loser of"
          // this match
          QString where = MA_PAIR1_SYMBOLIC_VAL + " = ? OR ";
          where += MA_PAIR2_SYMBOLIC_VAL + " = ?";
          QVariantList qvl;
          int symbMatchId = asWinner ? ma.getId() : -(ma.getId());
          qvl << symbMatchId << symbMatchId;
          if (matchTab.getMatchCountForWhereClause(where, qvl) > 0)
          {
            return true;
          }

          return false;
        };
        //   --------- Intermezzo end -----------------


        // check 3: if the winner is still in: is there
        // a future game in this category for the winner?
        if (!winnerOut)
        {
          if (!(hasFutureMatch(*winner, true)))
          {
            result.removeAll(*winner);
          }
        }

        // check 4: if the loser is still in: is there
        // a future game in this category for the winner?
        if (!loserOut)
        {
          if (!(hasFutureMatch(*loser, false)))
          {
            result.removeAll(*loser);
          }
        }
      }
    }

    // everyone who has not yet been kicked from the
    // list survives this round
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
