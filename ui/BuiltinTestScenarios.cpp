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

#include <QString>
#include <QDir>

#include "BuiltinTestScenarios.h"
#include "../HelperFunc.h"
#include "BackendAPI.h"

namespace QTournament
{
  namespace BuiltinScenarios
  {
    void setupTestScenario(const TournamentDB& db, int scenarioID)
    {

      switch (scenarioID)
      {
      case 1:
        setupScenario01(db);
        break;
      case 2:
        setupScenario02(db);
        break;
      case 3:
        setupScenario03(db);
        break;
      case 4:
        setupScenario04(db);
        break;
      case 5:
        setupScenario05(db);
        break;
      case 6:
        setupScenario06(db);
        break;
      case 7:
        setupScenario07(db);
        break;
      case 8:
        setupScenario08(db);
        break;
      }
    }

    //----------------------------------------------------------------------------

    void setupScenario01(const TournamentDB& db)
    {
      TeamMngr tmngr{db};
      PlayerMngr pmngr{db};
      CatMngr cmngr{db};

      tmngr.createNewTeam("Team 1");
      tmngr.createNewTeam("Team 2");

      pmngr.createNewPlayer("First1", "Last1", Sex::M, "Team 1");
      pmngr.createNewPlayer("First2", "Last2", Sex::F, "Team 1");
      pmngr.createNewPlayer("First3", "Last3", Sex::M, "Team 1");
      pmngr.createNewPlayer("First4", "Last4", Sex::F, "Team 1");
      pmngr.createNewPlayer("First5", "Last5", Sex::M, "Team 2");
      pmngr.createNewPlayer("First6", "Last6", Sex::F, "Team 2");

      // create one category of every kind
      cmngr.createNewCategory("MS");
      Category ms = cmngr.getCategory("MS");
      ms.setMatchType(MatchType::Singles);
      ms.setSex(Sex::M);

      cmngr.createNewCategory("MD");
      Category md = cmngr.getCategory("MD");
      md.setMatchType(MatchType::Doubles);
      md.setSex(Sex::M);

      cmngr.createNewCategory("LS");
      Category ls = cmngr.getCategory("LS");
      ls.setMatchType(MatchType::Singles);
      ls.setSex(Sex::F);

      cmngr.createNewCategory("LD");
      Category ld = cmngr.getCategory("LD");
      ld.setMatchType(MatchType::Doubles);
      ld.setSex(Sex::F);

      cmngr.createNewCategory("MX");
      Category mx = cmngr.getCategory("MX");
      mx.setMatchType(MatchType::Mixed);
      mx.setSex(Sex::M); // shouldn't matter at all
    }

    //----------------------------------------------------------------------------

    void setupScenario02(const TournamentDB& db)
    {
      setupScenario01(db);

      TeamMngr tmngr{db};
      PlayerMngr pmngr{db};
      CatMngr cmngr{db};

      Category md = cmngr.getCategory("MD");
      Category ms = cmngr.getCategory("MS");
      Category mx = cmngr.getCategory("MX");

      Player m1 = pmngr.getPlayer(1);
      Player m2 = pmngr.getPlayer(3);
      Player m3 = pmngr.getPlayer(5);
      Player l1 = pmngr.getPlayer(2);
      Player l2 = pmngr.getPlayer(4);
      Player l3 = pmngr.getPlayer(6);

      cmngr.addPlayerToCategory(m1, md);
      cmngr.addPlayerToCategory(m2, md);

      cmngr.addPlayerToCategory(m1, ms);
      cmngr.addPlayerToCategory(m2, ms);
      cmngr.addPlayerToCategory(m3, ms);

      cmngr.addPlayerToCategory(m1, mx);
      cmngr.addPlayerToCategory(m2, mx);
      cmngr.addPlayerToCategory(m3, mx);
      cmngr.addPlayerToCategory(l1, mx);
      cmngr.addPlayerToCategory(l2, mx);
      cmngr.addPlayerToCategory(l3, mx);
    }

    //----------------------------------------------------------------------------

    void setupScenario03(const TournamentDB& db)
    {
      setupScenario02(db);

      TeamMngr tmngr{db};
      PlayerMngr pmngr{db};
      CatMngr cmngr{db};

      tmngr.createNewTeam("Massive");
      Category ls = cmngr.getCategory("LS");

      for (int i=0; i < 250; i++)
      {
        QString lastName = "Massive" + QString::number(i);
        pmngr.createNewPlayer("Lady", lastName, Sex::F, "Massive");
        Player p = pmngr.getPlayer(i + 7);   // the first six IDs are already used by previous ini-functions above
        if (i < 40) ls.addPlayer(p);
        //if (i < 17) ls.addPlayer(p);
      }

      // create and set a valid group configuration for LS
      GroupDef d = GroupDef(5, 8);
      GroupDefList gdl;
      gdl.append(d);
      KO_Config cfg(KO_Start::Quarter, false, gdl);
      ls.setParameter(CatParameter::GroupConfig, cfg.toString());
    }

    //----------------------------------------------------------------------------

    void setupScenario04(const TournamentDB& db)
    {
      setupScenario03(db);

      TeamMngr tmngr{db};
      PlayerMngr pmngr{db};
      CatMngr cmngr{db};

      Category ls = cmngr.getCategory("LS");

      // run the category
      std::unique_ptr<Category> specialCat = ls.convertToSpecializedObject();
      Error e = cmngr.freezeConfig(ls);
      assert(e == Error::OK);

      // fake a list of player-pair-lists for the group assignments
      std::vector<PlayerPairList> ppListList;
      for (int grpNum=0; grpNum < 8; ++grpNum)
      {
        PlayerPairList thisGroup;
        for (int pNum=0; pNum < 5; ++pNum)
        {
          int playerId = (grpNum * 5) + pNum + 7;  // the first six IDs are already in use; see above

          Player p = pmngr.getPlayer(playerId);
          PlayerPair pp(p, (playerId-6));   // PlayerPairID starts at 1
          thisGroup.push_back(pp);
        }
        ppListList.push_back(thisGroup);
      }

      // make sure the faked group assignment is valid
      e = specialCat->canApplyGroupAssignment(ppListList);
      assert(e == Error::OK);

      // prepare an empty list for the (not required) initial ranking
      PlayerPairList initialRanking;

      // actually run the category
      e = cmngr.startCategory(ls, ppListList, initialRanking);
      assert(e == Error::OK);

      // we're done with LS here...

      // add 16 players to LD
      Category ld = cmngr.getCategory("LD");
      for (int id=100; id < 116; ++id)
      {
        e = ld.addPlayer(pmngr.getPlayer(id));
        assert(e == Error::OK);
      }

      // generate eight player pairs
      for (int id=100; id < 116; id+=2)
      {
        Player p1 = pmngr.getPlayer(id);
        Player p2 = pmngr.getPlayer(id+1);
        e = cmngr.pairPlayers(ld, p1, p2);
        assert(e == Error::OK);
      }

      // set the config to be 2 groups of 4 players each
      // and that KOs start with the final
      GroupDef d = GroupDef(4, 2);
      GroupDefList gdl;
      gdl.append(d);
      KO_Config cfg(KO_Start::Final, false, gdl);
      assert(ld.setParameter(CatParameter::GroupConfig, cfg.toString()) == true);

      // freeze
      specialCat = ld.convertToSpecializedObject();
      e = cmngr.freezeConfig(ld);
      assert(e == Error::OK);

      // fake a list of player-pair-lists for the group assignments
      ppListList.clear();
      PlayerPairList allPairsInCat = ld.getPlayerPairs();
      for (int grpNum=0; grpNum < 2; ++grpNum)
      {
        PlayerPairList thisGroup;
        for (int pNum=0; pNum < 4; ++pNum)
        {
          thisGroup.push_back(allPairsInCat.at(grpNum * 4 + pNum));
        }
        ppListList.push_back(thisGroup);
      }

      // make sure the faked group assignment is valid
      e = specialCat->canApplyGroupAssignment(ppListList);
      assert(e == Error::OK);

      // actually run the category
      e = cmngr.startCategory(ld, ppListList, initialRanking);  // "initialRanking" is reused from above
      assert(e == Error::OK);
    }

    //----------------------------------------------------------------------------

    void setupScenario05(const TournamentDB& db)
    {
      setupScenario04(db);

      TeamMngr tmngr{db};
      PlayerMngr pmngr{db};
      CatMngr cmngr{db};
      MatchMngr mm{db};
      CourtMngr courtm{db};

      Category ls = cmngr.getCategory("LS");

      auto mg = mm.getMatchGroup(ls, 1, 3);  // round 1, players group 3
      assert(mg);
      mm.stageMatchGroup(*mg);
      mm.scheduleAllStagedMatchGroups();

      Category ld = cmngr.getCategory("LD");
      mg = mm.getMatchGroup(ld, 1, 1);  // round 1, players group 1
      assert(mg);
      mm.stageMatchGroup(*mg);
      mg = mm.getMatchGroup(ld, 1, 2);  // round 1, players group 2
      assert(mg);
      mm.stageMatchGroup(*mg);
      mg = mm.getMatchGroup(ld, 2, 1);  // round 2, players group 1
      assert(mg);
      mm.stageMatchGroup(*mg);
      mm.scheduleAllStagedMatchGroups();

      // add four courts
      for (int i=1; i <= 4; ++i)
      {
        auto co = courtm.createNewCourt(i, "XX");
        assert(co);
      }
    }

    //----------------------------------------------------------------------------

    void setupScenario06(const TournamentDB& db)
    {
      setupScenario05(db);

      TeamMngr tmngr{db};
      PlayerMngr pmngr{db};
      CatMngr cmngr{db};
      MatchMngr mm{db};
      CourtMngr courtm{db};

      Category ls = cmngr.getCategory("LS");
      Category ld = cmngr.getCategory("LD");

      // stage and schedule all matches in round 1
      // of LS and LD
      bool canStageMatchGroups = true;
      while (canStageMatchGroups)
      {
        canStageMatchGroups = false;
        for (MatchGroup mg : mm.getMatchGroupsForCat(ls))
        {
          if (mg.is_NOT_InState(ObjState::MG_Idle)) continue;
          if (mm.canStageMatchGroup(mg) != Error::OK) continue;
          mm.stageMatchGroup(mg);
          canStageMatchGroups = true;
        }
        for (MatchGroup mg : mm.getMatchGroupsForCat(ld))
        {
          if (mg.is_NOT_InState(ObjState::MG_Idle)) continue;
          if (mm.canStageMatchGroup(mg) != Error::OK) continue;
          mm.stageMatchGroup(mg);
          canStageMatchGroups = true;
        }
      }
      mm.scheduleAllStagedMatchGroups();

      // play all scheduled matches
      QDateTime curDateTime = QDateTime::currentDateTimeUtc();
      uint epochSecs = curDateTime.toTime_t();
      SqliteOverlay::DbTab matchTab{db, TabMatch, false};
      while (true)
      {
        auto nextMatch = API::Qry::nextCallableMatch(db);
        if (!nextMatch) break;

        auto nextCourt = courtm.getNextUnusedCourt();
        if (!nextCourt) break;

        if (mm.assignMatchToCourt(*nextMatch, *nextCourt) != Error::OK) break;
        auto score = MatchScore::genRandomScore();
        mm.setMatchScoreAndFinalizeMatch(*nextMatch, *score);

        // overwrite the match finish time to get a fake match duration
        // the duration is at least 15 minutes and max 25 minutes
        int fakeDuration = 15 * 60  +  10 * 60 * (qrand() / (RAND_MAX * 1.0));
        auto maRow = matchTab[nextMatch->getId()];
        maRow.update(MA_FinishTime, std::to_string(epochSecs + fakeDuration));
      }
    }

    //----------------------------------------------------------------------------

    void setupScenario07(const TournamentDB& db)
    {
      setupScenario03(db);

      TeamMngr tmngr{db};
      PlayerMngr pmngr{db};
      CatMngr cmngr{db};
      MatchMngr mm{db};
      CourtMngr courtm{db};

      Category ls = cmngr.getCategory("LS");

      // set the match system to Single Elimination
      Error e = ls.setMatchSystem(MatchSystem::SingleElim) ;
      assert(e == Error::OK);

      // run the category
      std::unique_ptr<Category> specialCat = ls.convertToSpecializedObject();
      e = cmngr.freezeConfig(ls);
      assert(e == Error::OK);

      // prepare an empty list for the not-required initial group assignment
      std::vector<PlayerPairList> ppListList;

      // prepare a list for the (faked) initial ranking
      PlayerPairList initialRanking = ls.getPlayerPairs();

      // actually run the category
      e = cmngr.startCategory(ls, ppListList, initialRanking);
      assert(e == Error::OK);

      // stage all match groups
      auto mg = mm.getMatchGroup(ls, 1, GroupNum_Iteration);  // round 1
      assert(mg);
      mm.stageMatchGroup(*mg);
      mg = mm.getMatchGroup(ls, 2, GroupNum_Iteration);  // round 2
      assert(mg);
      mm.stageMatchGroup(*mg);
      mg = mm.getMatchGroup(ls, 3, GroupNum_L16);  // round 3
      assert(mg);
      mm.stageMatchGroup(*mg);
      mg = mm.getMatchGroup(ls, 4, GroupNum_Quarter);  // round 4
      assert(mg);
      mm.stageMatchGroup(*mg);
      mg = mm.getMatchGroup(ls, 5, GroupNum_Semi);  // round 5
      assert(mg);
      mm.stageMatchGroup(*mg);
      mg = mm.getMatchGroup(ls, 6, GroupNum_Final);  // round 6
      assert(mg);
      mm.stageMatchGroup(*mg);
      mm.scheduleAllStagedMatchGroups();

      // add four courts
      for (int i=1; i <= 4; ++i)
      {
        auto co = courtm.createNewCourt(i, "XX");
        assert(co);
      }

      // play all matches
      while (true)
      {
        auto nextMatch = API::Qry::nextCallableMatch(db);
        if (!nextMatch) break;

        auto nextCourt = courtm.getNextUnusedCourt();
        if (!nextCourt) break;

        if (mm.assignMatchToCourt(*nextMatch, *nextCourt) != Error::OK) break;
        auto score = MatchScore::genRandomScore();
        mm.setMatchScoreAndFinalizeMatch(*nextMatch, *score);
      }
    }

    //----------------------------------------------------------------------------

    void setupScenario08(const TournamentDB& db)
    {
      setupScenario02(db);

      TeamMngr tmngr{db};
      PlayerMngr pmngr{db};
      CatMngr cmngr{db};

      tmngr.createNewTeam("Ranking Team");
      Category ls = cmngr.getCategory("LS");
      Category ld = cmngr.getCategory("LD");

      int evenPlayerId = -1;
      for (int i=0; i < 28; i++)   // must be an even number, for doubles!
      {
        QString lastName = "Ranking" + QString::number(i+1);
        pmngr.createNewPlayer("Lady", lastName, Sex::F, "Ranking Team");
        Player p = pmngr.getPlayer(i + 7);   // the first six IDs are already used by previous ini-functions above
        ls.addPlayer(p);
        ld.addPlayer(p);

        // pair every two players
        if ((i % 2) == 0)
        {
          evenPlayerId = p.getId();
        } else {
          Player evenPlayer = pmngr.getPlayer(evenPlayerId);
          cmngr.pairPlayers(ld, p, evenPlayer);
        }
      }

      ls.setMatchSystem(MatchSystem::Ranking);
      ld.setMatchSystem(MatchSystem::Ranking);

      // freeze the LS category
      Error e = cmngr.freezeConfig(ls);
      assert(e == Error::OK);

      // prepare an empty list for the not-required initial group assignment
      std::vector<PlayerPairList> ppListList;

      // prepare a list for the (faked) initial ranking
      PlayerPairList initialRanking = ls.getPlayerPairs();

      // actually run the category
      e = cmngr.startCategory(ls, ppListList, initialRanking);
      assert(e == Error::OK);

      // freeze the LD category
      e = cmngr.freezeConfig(ld);
      assert(e == Error::OK);

      // prepare a list for the (faked) initial ranking
      initialRanking = ld.getPlayerPairs();

      // actually run the category
      e = cmngr.startCategory(ld, ppListList, initialRanking);
      assert(e == Error::OK);
    }










  }
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


//----------------------------------------------------------------------------

