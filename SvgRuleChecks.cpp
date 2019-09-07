
#include <string>
#include <algorithm>
#include <optional>

#include "SvgBracket.h"
#include "SvgRuleChecks.h"

using namespace std;

namespace QTournament::SvgBracket
{
  std::optional<std::string> consistencyCheck(const std::vector<ParsedTag> allTags)
  {
    // dissect the input into players, matches and ranks
    std::vector<ParsedTag> allMatches;
    std::copy_if(begin(allTags), end(allTags), std::back_inserter(allMatches), [](const ParsedTag& pt)
    {
      return (pt.type == TagType::Match);
    });

    std::vector<ParsedTag> allPlayers;
    std::copy_if(begin(allTags), end(allTags), std::back_inserter(allPlayers), [](const ParsedTag& pt)
    {
      return (pt.type == TagType::Player);
    });

    std::vector<ParsedTag> allRanks;
    std::copy_if(begin(allTags), end(allTags), std::back_inserter(allRanks), [](const ParsedTag& pt)
    {
      return (pt.type == TagType::Rank);
    });

    // collect all rules
    using ruleCheckSig = std::function<std::optional<std::string>(const std::vector<ParsedTag>&, const std::vector<ParsedTag>&, const std::vector<ParsedTag>&)>;
    std::vector<ruleCheckSig> allChecks;
    allChecks.push_back(checkRule01);
    allChecks.push_back(checkRule02);
    allChecks.push_back(checkRule03);
    allChecks.push_back(checkRule04);
    allChecks.push_back(checkRule05);
    allChecks.push_back(checkRule06);
    //allChecks.push_back(checkRule01);
    //allChecks.push_back(checkRule01);
    //allChecks.push_back(checkRule01);

    // execute all checks
    for (const auto& rule : allChecks)
    {
      auto result = rule(allMatches, allPlayers, allRanks);
      if (result)
      {
        return result;
      }
    }

    return {};
  }

  //----------------------------------------------------------------------------

  std::optional<std::string> checkRule01(const std::vector<ParsedTag>& allMatches, const std::vector<ParsedTag>& allPlayers, const std::vector<ParsedTag>& allRanks)
  {
    std::cout << "Rule 01 called" << std::endl;

    if (allMatches.empty())
    {
      return "The list of matches in the bracket is empty";
    }

    return {};
  }

  //----------------------------------------------------------------------------

  std::optional<string> checkRule02(const std::vector<ParsedTag>& allMatches, const std::vector<ParsedTag>& allPlayers, const std::vector<ParsedTag>& allRanks)
  {
    std::cout << "Rule 02 called" << std::endl;

    // copy the match list
    std::vector<ParsedTag> ma{allMatches.begin(), allMatches.end()};

    // sort the list in ascending match number order
    std::sort(begin(ma), end(ma), [](const ParsedTag& t1, const ParsedTag& t2)
    {
      const auto& ma1 = get<MatchTag>(t1.content);
      const auto& ma2 = get<MatchTag>(t2.content);
      return (ma1.bracketMatchNum < ma2.bracketMatchNum);
    });

    // perform two checks at once:
    //
    // (1): matches must have consecutive match numbers
    //
    // (2): the round number must be equal or higher compared to the previous match
    Round prevRound{1};
    for (int n=1; n <= ma.size(); ++n)
    {
      const auto& mTag = get<MatchTag>(ma.at(n-1).content);
      if (mTag.bracketMatchNum != n)
      {
        return "Inconsistent match numbers";
      }

      if (mTag.roundNum < 1)
      {
        return "Round number for match " + to_string(mTag.bracketMatchNum.get()) + " invalid";
      }

      if (mTag.roundNum < prevRound)
      {
        return "Round number for match " + to_string(mTag.bracketMatchNum.get()) + " inconsistent";
      }

      prevRound = mTag.roundNum;
    }

    return {};
  }

  //----------------------------------------------------------------------------

  std::optional<string> checkRule03(const std::vector<ParsedTag>& allMatches, const std::vector<ParsedTag>& allPlayers, const std::vector<ParsedTag>& allRanks)
  {
    std::cout << "Rule 03 called" << std::endl;

    if (allPlayers.size() != (4 * allMatches.size()))
    {
      return "Expected " + to_string(4 * allMatches.size()) + " player labels but got only " + to_string(allPlayers.size());
    }

    // a helper that checks whether a given player tag exists
    auto hasPlayerTag = [&](int matchNum, int playerNum, LabelPos lp)
    {
      auto it = std::find_if(begin(allPlayers), end(allPlayers), [&](const ParsedTag& pt)
      {
        const auto& pl = get<PlayerTag>(pt.content);

        return ((pl.bracketMatchNum == matchNum) && (pl.playerPos == playerNum) && (pl.pos == lp));
      });

      return (it != end(allPlayers));
    };

    // check that the four player labels per match are of the correct type
    for (int n = 1; n <= allMatches.size(); ++n)
    {
      for (int pp : {1, 2})
      {
        if (!hasPlayerTag(n, pp, LabelPos::First))
        {
          return "No player " + to_string(pp) + " for match " + to_string(n) + " (first row) found";
        }
        if (!hasPlayerTag(n, pp, LabelPos::Second))
        {
          return "No player " + to_string(pp) + " for match " + to_string(n) + " (second row) found";
        }
      }
    }

    return {};
  }

  //----------------------------------------------------------------------------

  std::optional<string> checkRule04(const std::vector<ParsedTag>& allMatches, const std::vector<ParsedTag>& allPlayers, const std::vector<ParsedTag>& allRanks)
  {
    std::cout << "Rule 04 called" << std::endl;

    // filter for all first round matches
    std::vector<ParsedTag> frm;
    std::copy_if(begin(allMatches), end(allMatches), std::back_inserter(frm), [](const ParsedTag& pt)
    {
      const auto& ma = get<MatchTag>(pt.content);
      return (ma.roundNum == 1);
    });

    // filter for all first round players
    std::vector<ParsedTag> frp;
    for (const auto& maTag : frm)
    {
      const auto& ma = get<MatchTag>(maTag.content);

      std::copy_if(begin(allPlayers), end(allPlayers), std::back_inserter(frp), [&ma](const ParsedTag& pt)
      {
        const auto& pl = get<PlayerTag>(pt.content);
        return ((pl.bracketMatchNum == ma.bracketMatchNum) && (pl.pos == LabelPos::First));
      });
    }

    if ((frm.size() * 2) != frp.size())
    {
      return "Got " + to_string(frm.size()) + " matches in the first round but " + to_string(frp.size()) + " players";
    }

    // sort by initial rank
    std::sort(begin(frp), end(frp), [](const ParsedTag& t1, const ParsedTag& t2)
    {
      const auto& pl1 = get<PlayerTag>(t1.content);
      const auto& pl2 = get<PlayerTag>(t2.content);

      return (pl1.initialRank < pl2.initialRank);
    });

    // must have consecutive initial ranks starting at "1"
    for (int n = 1; n <= frp.size(); ++n)
    {
      const auto& pTag = get<PlayerTag>(frp.at(n-1).content);

      if (pTag.initialRank != n)
      {
        return "No player with initial rank " + to_string(n);
      }
    }

    // sort by match number
    std::sort(begin(frp), end(frp), [](const ParsedTag& t1, const ParsedTag& t2)
    {
      const auto& pl1 = get<PlayerTag>(t1.content);
      const auto& pl2 = get<PlayerTag>(t2.content);

      return (pl1.bracketMatchNum < pl2.bracketMatchNum);
    });

    // the sum of the initial ranks in a match must
    // always be equal to "number of player in first round + 1"
    for (int i=0; i < frm.size(); ++i)
    {
      const auto& p1 = get<PlayerTag>(frp.at(2 * i).content);
      const auto& p2 = get<PlayerTag>(frp.at(2 * i + 1).content);
      if (p1.bracketMatchNum != p2.bracketMatchNum)
      {
        throw std::runtime_error("SvgBracket::checkRule04(): Sorting error!!");
      }

      if ((p1.initialRank.get() + p2.initialRank.get()) != (frp.size() + 1))
      {
        return "Inconsistent initial ranks in match " + to_string(p1.bracketMatchNum.get()) + " (rank sum incorrect)";
      }
    }

    // initial rank after round 1 are illegal
    for (const auto& tag : allPlayers)
    {
      const auto& pl = get<PlayerTag>(tag.content);

      // "frm.size()" is identical with the number of the last match
      // in round 1. This is correct because we checked the correct
      // consecutive numbering of matches before
      //
      // thus, "bracketMatchNumber > frm.size" is identical with
      // "match later than round 1"

      if ((pl.initialRank > 0) && (pl.bracketMatchNum > frm.size()))
      {
        return "Player " + to_string(pl.playerPos) + " of match " + to_string(pl.bracketMatchNum.get()) +
               " has an initial rank although the match is not in round 1";
      }
    }

    return {};
  }

  //----------------------------------------------------------------------------

  std::optional<string> checkRule05(const std::vector<ParsedTag>& allMatches, const std::vector<ParsedTag>& allPlayers, const std::vector<ParsedTag>& allRanks)
  {
    std::cout << "Rule 05 called" << std::endl;

    // count how often a match is referenced as a source match
    vector<int> winnerRefs(static_cast<int>(allMatches.size()), 0);
    vector<int> loserRefs(static_cast<int>(allMatches.size()), 0);
    for (const auto& tag : allPlayers)
    {
      const auto& pl = get<PlayerTag>(tag.content);

      if (abs(pl.srcMatch.get()) >= allMatches.size())
      {
        return "Player " + to_string(pl.playerPos) + " of match " + to_string(pl.bracketMatchNum.get()) +
               " references an invalid match number";
      }

      if (pl.srcMatch > 0)
      {
        winnerRefs.at(pl.srcMatch.get() - 1) += 1;
      }
      if (pl.srcMatch < 0)
      {
        loserRefs.at(-pl.srcMatch.get() - 1) += 1;
      }
    }

    // checks occurencies
    auto it = std::find_if(begin(winnerRefs), end(winnerRefs), [](const int& i)
    {
      return (i > 1);
    });
    if (it != end(winnerRefs))
    {
      return "Match " + to_string(std::distance(begin(winnerRefs), it) + 1) + " was referenced "
             " more than once as a source game (winner)";
    }
    it = std::find_if(begin(loserRefs), end(loserRefs), [](const int& i)
    {
      return (i > 1);
    });
    if (it != end(loserRefs))
    {
      return "Match " + to_string(std::distance(begin(loserRefs), it) + 1) + " was referenced "
             " more than once as a source game (loser)";
    }

    // check that each player without an initial rank
    // has a source match
    for (const auto& tag : allPlayers)
    {
      const auto& pl = get<PlayerTag>(tag.content);

      if (pl.pos != LabelPos::First) continue;

      if ((pl.initialRank < 1) && (pl.srcMatch == 0))
      {
        return "Player " + to_string(pl.playerPos) + " of match " + to_string(pl.bracketMatchNum.get()) +
               " has neither an initial rank nor a source match";
      }
    }

    return {};
  }

  //----------------------------------------------------------------------------

  std::optional<string> checkRule06(const std::vector<ParsedTag>& allMatches, const std::vector<ParsedTag>& allPlayers, const std::vector<ParsedTag>& allRanks)
  {
    std::cout << "Rule 06 called" << std::endl;

    // helper function
    auto hasFollowUpMatch = [&](int srcMatchNum)
    {
      auto it = find_if(begin(allPlayers), end(allPlayers), [&srcMatchNum](const ParsedTag& tag)
      {
        const auto& pl = get<PlayerTag>(tag.content);
        return (pl.srcMatch == srcMatchNum);
      });

      return (it != end(allPlayers));
    };

    // filter for all first round matches
    std::vector<ParsedTag> frm;
    std::copy_if(begin(allMatches), end(allMatches), std::back_inserter(frm), [](const ParsedTag& pt)
    {
      const auto& ma = get<MatchTag>(pt.content);
      return (ma.roundNum == 1);
    });
    auto nPlayers = frm.size() * 2;

    // count how often a rank has been assigned
    vector<int> cntRank(nPlayers, 0);

    // loop over all matches, check winner/loser references
    // and rank assignment
    for (const auto& tag : allMatches)
    {
      const auto& ma = get<MatchTag>(tag.content);

      if (ma.loserRank > 0)
      {
        if (ma.loserRank > nPlayers)
        {
          return "Match " + to_string(ma.bracketMatchNum.get()) + " has an invalid loser rank";
        }
        cntRank.at(ma.loserRank.get() - 1) += 1;
      } else {
        if (!hasFollowUpMatch(-ma.bracketMatchNum.get()))
        {
          return "Loser of match " + to_string(ma.bracketMatchNum.get()) + " has no final rank and no follow-up match";
        }
      }

      if (ma.winnerRank > 0)
      {
        if (ma.winnerRank > nPlayers)
        {
          return "Match " + to_string(ma.bracketMatchNum.get()) + " has an invalid winner rank";
        }
        cntRank.at(ma.winnerRank.get() - 1) += 1;
      } else {
        if (!hasFollowUpMatch(ma.bracketMatchNum.get()))
        {
          return "Winner of match " + to_string(ma.bracketMatchNum.get()) + " has no final rank and no follow-up match";
        }
      }
    }

    // checks occurencies
    auto it = std::find_if(begin(cntRank), end(cntRank), [](const int& i)
    {
      return (i > 1);
    });
    if (it != end(cntRank))
    {
      return "Rank " + to_string(std::distance(begin(cntRank), it) + 1) + " was assigned "
             " more than once as winner/loser rank";
    }

    return {};
  }

  //----------------------------------------------------------------------------

}
