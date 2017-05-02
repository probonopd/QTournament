#include <iostream>

#include <Sloppy/libSloppy.h>

#include <gtest/gtest.h>

#include "../SwissLadderGenerator.h"

using namespace QTournament;
using namespace Sloppy;

// a helper function that converts a string to a
// vector of tuples
//
// format: "i1.1,i1.2:i2.1,i2.2:,..."
vector<tuple<int, int>> strToVecOfTuples(const string& s)
{
  vector<tuple<int, int>> result;
  if (s.empty()) return result;

  StringList sTuples;
  stringSplitter(sTuples, s, ":", true);
  for (const string& t : sTuples)
  {
    StringList sInts;
    stringSplitter(sInts, t, ",", true);

    int i1 = stoi(sInts[0]);
    int i2 = stoi(sInts[1]);

    result.push_back(make_tuple(i1, i2));
  }
  return result;
}

//----------------------------------------------------------------------------

// a helper function that converts a vector of tuples
// to a string
string vecOfTuplesToStr(const vector<tuple<int, int>>& v)
{
  string result;

  for (const auto& t : v)
  {
    int i1 = get<0>(t);
    int i2 = get<1>(t);
    string s = "%1,%2:";
    strArg(s, i1);
    strArg(s, i2);
    result += s;
  }

  if (!(v.empty()))
  {
    result = result.substr(0, result.length() - 1);
  }

  return result;
}

//----------------------------------------------------------------------------

TEST(SwissLadderGen, Helpers)
{
  string s{"1,2:3,4:5,6"};
  auto vt = strToVecOfTuples(s);
  ASSERT_EQ(3, vt.size());
  tuple<int, int> t = vt[1];
  ASSERT_EQ(3, get<0>(t));
  ASSERT_EQ(4, get<1>(t));

  string s2 = vecOfTuplesToStr(vt);
  ASSERT_EQ(s, s2);
}

//----------------------------------------------------------------------------

TEST(SwissLadderGen, FirstRound_Even)
{
  vector<int> ranking{1,2,3,4,5,6};
  auto pastMatches = strToVecOfTuples("");

  SwissLadderGenerator slg{ranking, pastMatches};
  vector<tuple<int, int>> nextMatches;
  int rc = slg.getNextMatches(nextMatches);

  ASSERT_EQ(0, rc);
  string s = vecOfTuplesToStr(nextMatches);
  ASSERT_EQ("1,2:3,4:5,6", s);
}

//----------------------------------------------------------------------------

TEST(SwissLadderGen, FirstRound_Odd)
{
  vector<int> ranking{1,2,3,4,5};
  auto pastMatches = strToVecOfTuples("");

  SwissLadderGenerator slg{ranking, pastMatches};
  vector<tuple<int, int>> nextMatches;
  int rc = slg.getNextMatches(nextMatches);

  ASSERT_EQ(0, rc);
  string s = vecOfTuplesToStr(nextMatches);
  ASSERT_EQ("1,2:3,4", s);
}

//----------------------------------------------------------------------------

TEST(SwissLadderGen, DeadlockDetection_Even)
{
  vector<int> ranking{1,5,3,6,4,2};
  auto pastMatches = strToVecOfTuples("1,2 : 3,4 : 5,6   :   1,3 : 5,4 : 2,6  :  1,5 : 2,4 : 3,6");

  SwissLadderGenerator slg{ranking, pastMatches};
  vector<tuple<int, int>> nextMatches;
  int rc = slg.getNextMatches(nextMatches);

  ASSERT_EQ(-2, rc);
  ASSERT_TRUE(nextMatches.empty());
}

//----------------------------------------------------------------------------

TEST(SwissLadderGen, DeadlockPrevention_Even)
{
  vector<int> ranking{1,5,2,4,3,6};
  auto pastMatches = strToVecOfTuples("1,2 : 3,4 : 5,6   :   1,3 : 5,4 : 2,6");

  SwissLadderGenerator slg{ranking, pastMatches};
  vector<tuple<int, int>> nextMatches;
  int rc = slg.getNextMatches(nextMatches);

  ASSERT_EQ(0, rc);
  string s = vecOfTuplesToStr(nextMatches);
  ASSERT_NE("1,5:2,4:3,6", s);
  cout << "Deadlock prevented next matches: " << s << endl;
}

//----------------------------------------------------------------------------

TEST(SwissLadderGen, DeadlockDetection_Odd)
{
  vector<int> ranking{1,4,2,3,5};
  auto pastMatches = strToVecOfTuples("1,2 : 3,4   :   1,3 : 5,4  :  1,5 : 2,4");

  SwissLadderGenerator slg{ranking, pastMatches};
  vector<tuple<int, int>> nextMatches;
  int rc = slg.getNextMatches(nextMatches);

  ASSERT_EQ(-2, rc);
  ASSERT_TRUE(nextMatches.empty());
}

//----------------------------------------------------------------------------

TEST(SwissLadderGen, DeadlockPrevention_Odd)
{
  vector<int> ranking{1,5,2,4,3};
  auto pastMatches = strToVecOfTuples("1,2 : 3,4   :   1,3 : 5,4");

  SwissLadderGenerator slg{ranking, pastMatches};
  vector<tuple<int, int>> nextMatches;
  int rc = slg.getNextMatches(nextMatches);

  ASSERT_EQ(0, rc);
  string s = vecOfTuplesToStr(nextMatches);
  ASSERT_NE("1,5:2,4", s);
  ASSERT_EQ("1,4:5,2", s);
  cout << "Deadlock prevented next matches: " << s << endl;
}

//----------------------------------------------------------------------------

TEST(SwissLadderGen, NoDoubleBye)
{
  vector<int> ranking{1,2,3,4,5,6,7};
  auto pastMatches = strToVecOfTuples("1,2 : 3,4 : 5,6  :   1,3 : 5,4 : 2,7");

  SwissLadderGenerator slg{ranking, pastMatches};
  vector<tuple<int, int>> nextMatches;
  int rc = slg.getNextMatches(nextMatches);

  ASSERT_EQ(0, rc);
  string s = vecOfTuplesToStr(nextMatches);

  // the next round's bye should be player 5
  size_t pos = s.find('5');
  ASSERT_EQ(string::npos, pos);
}
