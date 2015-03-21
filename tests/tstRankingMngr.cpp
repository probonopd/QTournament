/* 
 * File:   tstGenericDBObject.cpp
 * Author: volker
 * 
 * Created on March 2, 2014, 3:46 PM
 */

#include <stdexcept>

#include "tstRankingMngr.h"
#include "TournamentDB.h"
#include "TournamentDataDefs.h"
#include "TournamentErrorCodes.h"
#include "RankingMngr.h"
#include "RankingEntry.h"
#include "Tournament.h"
#include "CatRoundStatus.h"
#include "MatchMngr.h"
#include "Match.h"
#include "Score.h"

using namespace QTournament;
using namespace dbOverlay;

//----------------------------------------------------------------------------

void tstRankingMngr::testCreateUnsortedRanking()
{
  printStartMsg("tstRankingMngr::testCreateUnsortedRanking");
  
  
  TournamentDB* db = getScenario06(true);
  Tournament t(getSqliteFileName());
  DbTab rankTab = db->getTab(TAB_RANKING);
  RankingMngr* rm = Tournament::getRankingMngr();
  CatMngr* cm = Tournament::getCatMngr();
  MatchMngr* mm = Tournament::getMatchMngr();
  Category ms = cm->getCategory("MS");

  // status at test case start:
  // 40 players in category MS with 8 groups of 5 players.
  // All round-robin-matches scheduled and round 1 completely played
  // and finished


  // make sure we have no ranking entries
  CPPUNIT_ASSERT(rankTab.length() == 0);

  // make sure round 1 is completed
  CatRoundStatus crs = ms.getRoundStatus();
  CPPUNIT_ASSERT(crs.getFinishedRoundsCount() == 1);

  // create unsorted entries
  ERR e;
  RankingEntryList reList = rm->createUnsortedRankingEntriesForLastRound(ms, &e);
  CPPUNIT_ASSERT(e == OK);

  // loop through the table entries and check the contents
  DbTab::CachingRowIterator it = rankTab.getAllRows();
  while (!(it.isEnd()))
  {
    // Get the match data
    TabRow r = *it;
    int ppId = r[RA_PAIR_REF].toInt();
    PlayerPair pp = Tournament::getPlayerMngr()->getPlayerPair(ppId);
    unique_ptr<Match> ma = mm->getMatchForPlayerPairAndRound(pp, 1);

    // check correct category assignment
    CPPUNIT_ASSERT(r[RA_CAT_REF].toInt() == pp.getCategory(db)->getId());

    // make sure that no rank has been assigned yet
    CPPUNIT_ASSERT(r[RA_RANK].isNull());

    // check players with a bye
    if (ma == nullptr)
    {
      // this player hasn't played, to all entries should be zero
      CPPUNIT_ASSERT(r[RA_MATCHES_LOST].toInt() == 0);
      CPPUNIT_ASSERT(r[RA_MATCHES_WON].toInt() == 0);
      CPPUNIT_ASSERT(r[RA_MATCHES_DRAW].toInt() == 0);
      CPPUNIT_ASSERT(r[RA_GAMES_WON].toInt() == 0);
      CPPUNIT_ASSERT(r[RA_GAMES_LOST].toInt() == 0);
      CPPUNIT_ASSERT(r[RA_POINTS_WON].toInt() == 0);
      CPPUNIT_ASSERT(r[RA_POINTS_LOST].toInt() == 0);

      ++it;
      continue;
    }

    int playerNum = (ma->getPlayerPair1().getPairId() == ppId) ? 1 : 2;
    unique_ptr<MatchScore> score = ma->getScore();

    // check the ranking entries
    if (score->getWinner() == playerNum)
    {
      CPPUNIT_ASSERT(r[RA_MATCHES_LOST].toInt() == 0);
      CPPUNIT_ASSERT(r[RA_MATCHES_WON].toInt() == 1);
    } else {
      CPPUNIT_ASSERT(r[RA_MATCHES_LOST].toInt() == 1);
      CPPUNIT_ASSERT(r[RA_MATCHES_WON].toInt() == 0);
    }
    CPPUNIT_ASSERT(r[RA_MATCHES_DRAW].toInt() == 0);

    int gamesSum;
    int scoreSum;
    if (playerNum == 1)
    {
      gamesSum = get<0>(score->getGameSum());
      scoreSum = get<0>(score->getScoreSum());
    } else {
      gamesSum = get<1>(score->getGameSum());
      scoreSum = get<1>(score->getScoreSum());
    }
    CPPUNIT_ASSERT(r[RA_GAMES_WON].toInt() == gamesSum);
    CPPUNIT_ASSERT(r[RA_GAMES_LOST].toInt() == (score->getNumGames() - gamesSum));
    CPPUNIT_ASSERT(r[RA_POINTS_WON].toInt() == scoreSum);
    CPPUNIT_ASSERT(r[RA_POINTS_LOST].toInt() == (score->getPointsSum() - scoreSum));

    ++it;
  }

  // every player should have exactly one entry
  for (PlayerPair pp : ms.getPlayerPairs())
  {
    CPPUNIT_ASSERT(rankTab.getMatchCountForColumnValue(RA_PAIR_REF, pp.getPairId()) == 1);
  }
  
  delete db;
  printEndMsg();
}

//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    
