/*
 *    This is libSloppy, a library of sloppily implemented helper functions.
 *    Copyright (C) 2016 - 2017  Volker Knollmann
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

#include <boost/filesystem.hpp>
#include <boost/log/trivial.hpp>

#include "../TournamentDB.h"
#include "../TournamentDataDefs.h"
#include "../CatMngr.h"
#include "../TeamMngr.h"
#include "../PlayerMngr.h"

#include "BasicTestClass.h"

namespace boostfs = boost::filesystem;

using namespace QTournament;

void BasicTestFixture::SetUp()
{
  // create a dir for temporary files created during testing
  tstDirPath = boostfs::temp_directory_path();
  if (!(boostfs::exists(tstDirPath)))
  {
    throw std::runtime_error("Could not create temporary directory for test files!");
  }
}

//----------------------------------------------------------------------------

string BasicTestFixture::getTestDir() const
{
  return tstDirPath.native();
}

//----------------------------------------------------------------------------

string BasicTestFixture::genTestFilePath(string fName) const
{
  boostfs::path p = tstDirPath;
  p /= fName;
  return p.native();
}

//----------------------------------------------------------------------------

void BasicTestFixture::getScenario01(unique_ptr<TournamentDB>& result) const
{
  // prepare a brand-new scenario
  TournamentSettings cfg;
  cfg.organizingClub = "SV Whatever";
  cfg.tournamentName = "World Championship";
  cfg.useTeams = true;
  cfg.refereeMode = REFEREE_MODE::NONE;
  result = std::move(TournamentDB::createNew(":memory:", cfg));
  ASSERT_TRUE(result != nullptr);
}

//----------------------------------------------------------------------------

void BasicTestFixture::getScenario02(unique_ptr<TournamentDB>& result) const
{
  // start with an empty scenario
  getScenario01(result);

  // add a few dummy categories
  CatMngr cm{result.get()};
  ERR e = cm.createNewCategory("MS");
  ASSERT_EQ(OK, e);
  e = cm.createNewCategory("LD");
  ASSERT_EQ(OK, e);
  e = cm.createNewCategory("MX");
  ASSERT_EQ(OK, e);
  e = cm.createNewCategory("RR");  // a small round-robin with men's singles
  ASSERT_EQ(OK, e);
  auto ms = cm.getCategory("MS");
  auto ld = cm.getCategory("LD");
  auto mx = cm.getCategory("MX");
  auto rr = cm.getCategory("RR");

  // configure categories
  cm.setSex(ld, F);
  cm.setMatchType(ld, DOUBLES);
  cm.setMatchType(mx, MIXED);
  cm.setMatchSystem(rr, ROUND_ROBIN);

  // an empty team
  TeamMngr tm{result.get()};
  e = tm.createNewTeam("T1");

  // add 20 male and 20 female players to the tournament
  // and assign them to the applicable categories
  PlayerMngr pm{result.get()};
  for (int i=0; i < 20; ++i)
  {
    QString l1 = "m%1";
    l1 = l1.arg(i);

    QString l2 = "f%1";
    l2 = l2.arg(i);

    e = pm.createNewPlayer("a", l1, M, "T1");
    ASSERT_EQ(OK, e);
    e = pm.createNewPlayer("a", l2, F, "T1");
    ASSERT_EQ(OK, e);

    Player p = pm.getPlayer("a", l1);
    e = cm.addPlayerToCategory(p, ms);
    ASSERT_EQ(OK, e);
    e = cm.addPlayerToCategory(p, mx);
    ASSERT_EQ(OK, e);
    if (i < 4)
    {
      e = cm.addPlayerToCategory(p, rr);
      ASSERT_EQ(OK, e);
    }

    p = pm.getPlayer("a", l2);
    e = cm.addPlayerToCategory(p, ld);
    ASSERT_EQ(OK, e);
    e = cm.addPlayerToCategory(p, mx);
    ASSERT_EQ(OK, e);
  }
}

//----------------------------------------------------------------------------

void BasicTestFixture::getScenario03(unique_ptr<TournamentDB>& result) const
{
  getScenario02(result);

  // start the small round-robin category
  CatMngr cm{result.get()};
  auto rr = cm.getCategory("RR");
  ERR e = cm.freezeConfig(rr);
  ASSERT_EQ(OK, e);
  e = cm.startCategory(rr, {}, {});
  ASSERT_EQ(OK, e);
}
