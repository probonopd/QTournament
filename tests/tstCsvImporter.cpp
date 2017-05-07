#include <iostream>

#include <Sloppy/libSloppy.h>

#include <gtest/gtest.h>

#include "../TournamentDB.h"
#include "../CSVImporter.h"

#include "BasicTestClass.h"

using namespace QTournament;
using namespace Sloppy;


//----------------------------------------------------------------------------

TEST(CSVImport, Splitter)
{
  // empty string
  vector<vector<string>> result = splitCSV("");
  ASSERT_TRUE(result.empty());

  // all empty fields
  result = splitCSV(",,,,,");
  ASSERT_TRUE(result.empty());

  // normal case incl. trimming
  result = splitCSV("  l,    f   , m, t1, c1, c2,    c3    ,, c4  , ");
  ASSERT_EQ(1, result.size());
  vector<string> f = result[0];
  ASSERT_EQ(5, f.size());
  ASSERT_EQ("l", f[0]);
  ASSERT_EQ("f", f[1]);
  ASSERT_EQ("m", f[2]);
  ASSERT_EQ("t1", f[3]);
  ASSERT_EQ("c1, c2, c3, c4", f[4]);

  // empty fields in between
  result = splitCSV("  l,    , m, t1, c1");
  ASSERT_EQ(1, result.size());
  f = result[0];
  ASSERT_EQ(5, f.size());
  ASSERT_EQ("l", f[0]);
  ASSERT_TRUE(f[1].empty());
  ASSERT_EQ("m", f[2]);
  ASSERT_EQ("t1", f[3]);
  ASSERT_EQ("c1", f[4]);

  // empty fields at the end
  result = splitCSV("  l,  ,");
  ASSERT_EQ(1, result.size());
  f = result[0];
  ASSERT_EQ(1, f.size());
  ASSERT_EQ("l", f[0]);

  // no redundant categories
  result = splitCSV("l,f,m,t,c,c,c,x,x");
  ASSERT_EQ(1, result.size());
  f = result[0];
  ASSERT_EQ(5, f.size());
  ASSERT_EQ("c, x", f[4]);
}

//----------------------------------------------------------------------------

TEST_F(BasicTestFixture, CSVAnalysis)
{
  unique_ptr<QTournament::TournamentDB> _db;
  getScenario03(_db);
  TournamentDB* db = _db.get();

  // a helper
  auto checkErrList = [db](const string& raw, const vector<CSVError>& errList) {
    vector<vector<string>> splitData = splitCSV(raw, ",");
    auto e = analyseCSV(db, splitData);

    ASSERT_EQ(errList.size(), e.size());

    for (const auto& expectedErr : errList)
    {
      bool found = false;
      for (const CSVError& realErr : e)
      {
        if ((realErr.row) == (expectedErr.row) && (realErr.column == expectedErr.column) &&
            (realErr.err == expectedErr.err) && (realErr.para == expectedErr.para) &&
            (realErr.isFatal == expectedErr.isFatal))
        {
          found = true;
          break;
        }
      }

      ASSERT_TRUE(found);
    }
  };

  //
  // the actual test cases
  //

  checkErrList("l,f,m,t", {});

  checkErrList("l,f,m", {CSVError{0, 3, CSVErrCode::NoTeamName, "", true}, });

  checkErrList("l,f", {
                 CSVError{0, 3, CSVErrCode::NoTeamName, "", true},
                 CSVError{0, 2, CSVErrCode::NoSex, "", true}}
               );
  checkErrList("l", {
                 CSVError{0, 3, CSVErrCode::NoTeamName, "", true},
                 CSVError{0, 2, CSVErrCode::NoSex, "", true},
                 CSVError{0, 1, CSVErrCode::NoFirstName, "", true}}
               );
  checkErrList("l,,m", {
                 CSVError{0, 3, CSVErrCode::NoTeamName, "", true},
                 CSVError{0, 1, CSVErrCode::NoFirstName, "", true}}
               );
  checkErrList("m1,a,m", {
                 CSVError{0, 3, CSVErrCode::NoTeamName, "", true},
                 CSVError{0, 1, CSVErrCode::NameNotUnique, "", true},
                 CSVError{0, 0, CSVErrCode::NameNotUnique, "", true}}
               );
  checkErrList("l,f,x", {
                 CSVError{0, 3, CSVErrCode::NoTeamName, "", true},
                 CSVError{0, 2, CSVErrCode::InvalidSexIndicator, "", true}}
               );
  checkErrList("l,f,m,t,xxx", {
                 CSVError{0, 4, CSVErrCode::CategoryNotExisting, "xxx", false},
                 }
               );
  checkErrList("l,f,m,t,MS,xxx,MX, ", {
                 CSVError{0, 4, CSVErrCode::CategoryNotExisting, "xxx", false},
                 }
               );
  checkErrList("l,f,m,t,RR, ", {
                 CSVError{0, 4, CSVErrCode::CategoryLocked, "RR", false},
                 }
               );
  checkErrList("l,f,m,t,LD, ", {
                 CSVError{0, 4, CSVErrCode::CategoryNotSuitable, "LD", false},
                 }
               );
}
