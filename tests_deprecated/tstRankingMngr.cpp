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
  DbTab rankTab = db->getTab(TAB_MatchSystem::Ranking);
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
  CPPUNIT_ASSERT(reList.count() == 40);

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
    CPPUNIT_ASSERT(r[RA_CONFIGREF].toInt() == pp.getCategory(db)->getId());

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

void tstRankingMngr::testSortRanking__RoundRobin()
{
  printStartMsg("tstRankingMngr::testCreateUnsortedRanking");

  TournamentDB* db = getScenario06(true);
  Tournament t(getSqliteFileName());
  RankingMngr* rm = Tournament::getRankingMngr();
  CatMngr* cm = Tournament::getCatMngr();
  MatchMngr* mm = Tournament::getMatchMngr();
  Category ms = cm->getCategory("MS");

  // status at test case start:
  // 40 players in category MS with 8 groups of 5 players.
  // All round-robin-matches scheduled and round 1 completely played
  // and finished


  // make sure round 1 is completed
  CatRoundStatus crs = ms.getRoundStatus();
  CPPUNIT_ASSERT(crs.getFinishedRoundsCount() == 1);

  // create unsorted entries
  ERR e;
  rm->createUnsortedRankingEntriesForLastRound(ms, &e);
  CPPUNIT_ASSERT(e == OK);

  // sort entries
  RankingEntryListList reListList = rm->sortRankingEntriesForLastRound(ms, &e);
  CPPUNIT_ASSERT(e == OK);
  CPPUNIT_ASSERT(reListList.count() == 8);  // eight entries for eight groups

  // check the ranking in each group
  checkSortSequence__RoundRobin(1);

  // play a second round
  CPPUNIT_ASSERT(playRound(ms, 2, 16));
  rm->createUnsortedRankingEntriesForLastRound(ms, &e);
  CPPUNIT_ASSERT(e == OK);
  reListList = rm->sortRankingEntriesForLastRound(ms, &e);
  CPPUNIT_ASSERT(e == OK);
  CPPUNIT_ASSERT(reListList.count() == 8);  // eight entries for eight groups
  checkSortSequence__RoundRobin(2);

  // do a superficial check that the results of both rounds have been properly accumulated
  unique_ptr<RankingEntry> reFirst = rm->getRankingEntry(ms, 2, 1, 1);
  PlayerPair ppFirst = reFirst->getPlayerPair();
  auto maFirst1 = mm->getMatchForPlayerPairAndRound(ppFirst, 1);
  auto maFirst2 = mm->getMatchForPlayerPairAndRound(ppFirst, 2);
  auto scoreFirst1 = maFirst1->getScore();
  auto scoreFirst2 = maFirst2->getScore();
  CPPUNIT_ASSERT(get<0>(reFirst->getMatchStats()) == 2);   // first place must have won both rounds
  CPPUNIT_ASSERT(get<1>(reFirst->getMatchStats()) == 0);
  CPPUNIT_ASSERT(get<2>(reFirst->getMatchStats()) == 0);
  CPPUNIT_ASSERT(get<3>(reFirst->getMatchStats()) == 2);

  int lostGames = 0;
  if (scoreFirst1->getNumGames() == 3) ++lostGames;
  if (scoreFirst2->getNumGames() == 3) ++lostGames;
  CPPUNIT_ASSERT(get<0>(reFirst->getGameStats()) == 4);   // first place must have won both rounds
  CPPUNIT_ASSERT(get<1>(reFirst->getGameStats()) == lostGames);
  CPPUNIT_ASSERT(get<2>(reFirst->getGameStats()) == lostGames + 4);

  delete db;
  printEndMsg();
}

//----------------------------------------------------------------------------
    
void tstRankingMngr::checkSortSequence__RoundRobin(int round)
{
  RankingMngr* rm = Tournament::getRankingMngr();
  CatMngr* cm = Tournament::getCatMngr();
  Category ms = cm->getCategory("MS");

  RankingEntryListList reListList = rm->getSortedRanking(ms, round);

  // check the ranking in each group
  for (RankingEntryList reList : reListList)
  {
    CPPUNIT_ASSERT(reList.count() == 5);  // five players in each group

    int rank = 1;
    for (RankingEntry re : reList)
    {
      CPPUNIT_ASSERT(re.getRank() == rank);

      // compare each entry with the following one,
      // if available
      unique_ptr<RankingEntry> reNext = rm->getRankingEntry(ms, 1, re.getGroupNumber(), rank+1);
      if (reNext != nullptr)
      {
        int wonMatchesA = get<0>(re.getMatchStats());
        int wonMatchesB = get<0>(reNext->getMatchStats());
        CPPUNIT_ASSERT(wonMatchesA >= wonMatchesB);
        if (wonMatchesA > wonMatchesB) break;

        int wonGamesA = get<0>(re.getGameStats());
        int wonGamesB = get<0>(reNext->getGameStats());
        CPPUNIT_ASSERT(wonGamesA >= wonGamesB);
        if (wonGamesA > wonGamesB) break;

        int wonPointsA = get<0>(re.getPointStats());
        int wonPointsB = get<0>(reNext->getPointStats());
        CPPUNIT_ASSERT(wonPointsA >= wonPointsB);
      } else {
        CPPUNIT_ASSERT(rank == 5);  // only the last player may have no successor
      }

      ++rank;
    }
  }

}

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    
