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

#ifndef BRACKETGENERATOR_H
#define BRACKETGENERATOR_H

#include <memory>
#include <functional>
#include <vector>
#include <tuple>

#include <QList>

#include "reports/BracketVisData.h"

namespace QTournament
{

  class BracketMatchData
  {
  public:
    static constexpr int NO_INITIAL_RANK = 0;
    static constexpr int NO_NEXT_MATCH = 0;
    static constexpr int UNUSED_PLAYER = 999999;

    // Explicitly declare a default copy constructor
    BracketMatchData(const BracketMatchData& other) = default;

    // the next two values are either...
    //   * > 0 if they indicate an initial rank in the lef-most part of the bracket; or
    //   * < 0 the if player comes from a previous match (e. g. "-5" for bracketMatchId 5)
    int initialRank_Player1;
    int initialRank_Player2;

    // the next two values are either...
    //   * > 0 if they refer to the bracketMatchId for the next Match; or
    //   * == 0 if the player drops out without any final rank
    //   * < 0 the if player reaches a final rank (e. g. "-5" for rank 5)
    int nextMatchForWinner;
    int nextMatchForLoser;

    int nextMatchPlayerPosForWinner;  // 1 or 2 for player 1 or player 2
    int nextMatchPlayerPosForLoser;   // 1 or 2 for player 1 or player 2

    int depthInBracket;  // 0 = finals, 1 = semifinals, 2 = quarterfinals, 3 = last 16, ...

    // a tag that indicates a deleted match
    bool matchDeleted = false;

    static void resetBracketMatchId();
    int getBracketMatchId() const;
    void setInitialRanks(int initialRank_P1, int initialRank_P2);
    void setNextMatchForWinner(BracketMatchData& nextBracketMatch, int posInNextMatch);
    void setNextMatchForLoser(BracketMatchData& nextBracketMatch, int posInNextMatch);

    void dumpOnScreen();

    static BracketMatchData getNew();

  private:
    BracketMatchData();
    static int lastBracketMatchId;
    int bracketMatchId;
  };

  using upBracketMatchData = std::unique_ptr<BracketMatchData>;
  //typedef QList<BracketMatchData> BracketMatchDataList;
  using BracketMatchDataList = std::vector<BracketMatchData>;
  //typedef std::vector<upBracketMatchData> upBracketMatchDataVector;

  class BracketGenerator
  {
  public:
    static constexpr int BRACKET_SINGLE_ELIM = 1;
    static constexpr int BRACKET_DOUBLE_ELIM = 2;
    static constexpr int BRACKET_RANKING1 = 3;

    BracketGenerator();
    BracketGenerator(int type);

    void getBracketMatches(int numPlayers, BracketMatchDataList& bmdl__out, RawBracketVisDataDef& bvdd__out) const;
    static std::function<bool (const BracketMatchData&, const BracketMatchData&)> getBracketMatchSortFunction_earlyRoundsFirst();
    int getNumRounds(int numPlayers) const;

  private:
    int bracketType;
    void genBracket__SingleElim(int numPlayers, BracketMatchDataList& bmdl__out, RawBracketVisDataDef& bvdd__out) const;
    void genBracket__Ranking1(int numPlayers, BracketMatchDataList& bmdl__out, RawBracketVisDataDef& bvdd__out) const;
    void removeUnusedMatches(BracketMatchDataList& bracketMatches, int numPlayers) const;  // modifies the list IN PLACE!!
  };

}
#endif // BRACKETGENERATOR_H
