#include <iostream>
#include <stdexcept>
#include <regex>
#include <algorithm>
#include <functional>

#include "SvgBracket.h"

using namespace std;

namespace QTournament::SvgBracket
{

  std::vector<TagData> findRawTags(const string_view& svgData, const string& openingBracket, const string& closingBracket)
  {
    std::vector<TagData> result;

    string_view::size_type idx = 0;
    while (true)
    {
      // find the next opening pattern
      auto idxOpen = svgData.find(openingBracket, idx);
      if (idxOpen == std::string_view::npos)
      {
        return result;
      }

      // advance beyond the opening characters
      idx = idxOpen + openingBracket.size();

      // search for the closing pattern
      auto idxClose = svgData.find(closingBracket, idx);
      if (idxClose == std::string_view::npos)
      {
        throw std::invalid_argument("SvgBracket::findRawTags(): un-closed tag");
      }

      // store the tag data
      TagData d;
      d.idxStart = idxOpen;
      d.idxEnd = idxClose + closingBracket.size() - 1;
      d.len = d.idxEnd - d.idxStart + 1;
      auto idxNameStart = idxOpen + openingBracket.size();  // pos. of the first name character
      auto idxNameEnd = idxClose - 1;   // pos. of the last name character
      d.name = svgData.substr(idxNameStart, idxNameEnd - idxNameStart + 1);
      result.push_back(std::move(d));

      // calculate the where to continue the search
      idx = idxClose + closingBracket.size();
    }
  }

  //----------------------------------------------------------------------------

  TagType determineTagTypeFromName(const string& tagName)
  {
    if (tagName.size() < 2)
    {
      throw std::invalid_argument("SvgBracket::determineTagTypeFromName(): invalid tag; needs to be at least two characters long");
    }

    // Tags starting with "M" and a number are match tags
    if ((tagName[0] == 'M') && (std::isdigit(tagName[1])))
    {
      return TagType::Match;
    }

    // Tags starting with "P" and a number are player tags
    if ((tagName[0] == 'P') && (std::isdigit(tagName[1])))
    {
      return TagType::Player;
    }

    // Tags starting with "R" and a number are rank tags
    if ((tagName[0] == 'R') && (std::isdigit(tagName[1])))
    {
      return TagType::Rank;
    }

    // anything else
    return TagType::Other;
  }

  //----------------------------------------------------------------------------

  MatchTag parseMatchTag(const string& tagName)
  {
    // RegEx for the first part (match number and round) of a match tag
    static const std::regex reNumAndRound{R"(M(\d+):R(\d+))"};
    std::smatch sm;
    if (!std::regex_search(tagName, sm, reNumAndRound))
    {
      throw std::invalid_argument("SvgBracket::parseMatchTag(): invalid tag name: " + tagName);
    }

    // the "M" has to be at position 0
    if (sm.position() != 0)
    {
      throw std::invalid_argument("SvgBracket::parseMatchTag(): invalid tag name: " + tagName);
    }

    MatchTag result;
    result.bracketMatchNum = std::stoi(sm[1].str());
    result.roundNum = std::stoi(sm[2].str());

    // check for additional winner / loser ranks
    static const std::regex reWinnerRank{R"(:W(\d+))"};
    static const std::regex reLoserRank{R"(:L(\d+))"};
    if (std::regex_search(tagName, sm, reWinnerRank))
    {
      result.winnerRank = std::stoi(sm[1].str());
    }
    if (std::regex_search(tagName, sm, reLoserRank))
    {
      result.loserRank = std::stoi(sm[1].str());
    }

    return result;
  }

  //----------------------------------------------------------------------------

  PlayerTag parsePlayerTag(const string& tagName)
  {
    // RegEx for the first part (match number and player number) of a player tag
    static const std::regex rePlayerBase{R"(P(\d+)\.([12]))"};
    std::smatch sm;
    if (!std::regex_search(tagName, sm, rePlayerBase))
    {
      throw std::invalid_argument("SvgBracket::parsePlayerTag(): invalid tag name: " + tagName);
    }

    // the "P" has to be at position 0
    if (sm.position() != 0)
    {
      throw std::invalid_argument("SvgBracket::parsePlayerTag(): invalid tag name: " + tagName);
    }

    PlayerTag result;

    result.bracketMatchNum = std::stoi(sm[1].str());
    result.playerPos = std::stoi(sm[2].str());

    // if the name ends with 'b' we have the label for the second name
    if (tagName.back() == 'b')
    {
      result.pos = LabelPos::Second;
      return result;
    }

    // ==> this is the primary label
    //
    // it MUST contain information about EITHER an initial rank for from which match
    // the player came from
    //
    // check with additional regexps that include the first parts as well
    // so that we get some consistency checks. Now with FULL MATCH!
    static const std::regex rePlayerInitial{R"(P(\d+)\.([12]):I(\d+))"};
    if (std::regex_match(tagName, sm, rePlayerInitial))
    {
      result.initialRank = std::stoi(sm[3].str());
      return result;
    }
    static const std::regex rePlayerWinnerLoser{R"(P(\d+)\.([12]):([WL])(\d+))"};
    if (std::regex_match(tagName, sm, rePlayerWinnerLoser))
    {
      result.srcMatch = std::stoi(sm[4].str());
      if (sm[3].str() == "L") result.srcMatch *= -1;  // losers get a negative source match number

      return result;
    }

    throw std::invalid_argument("SvgBracket::parsePlayerTag(): inconsistent tag name: " + tagName);
  }

  //----------------------------------------------------------------------------

  RankTag parseRankTag(const string& tagName)
  {
    // RegEx for a rank label
    static const std::regex reRankLabel{R"(R(\d+)([ab]))"};

    // the label must fully match the regex
    std::smatch sm;
    if (std::regex_match(tagName, sm, reRankLabel))
    {
      RankTag result;
      result.rank = std::stoi(sm[1].str());
      result.pos = (sm[2].str() == "a") ? LabelPos::First : LabelPos::Second;

      return result;
    }

    throw std::invalid_argument("SvgBracket::parseRankTag(): inconsistent tag name: " + tagName);
  }

  //----------------------------------------------------------------------------

  std::vector<ParsedTag> parseContent(const string_view& svgData, const string& openingBracket, const string& closingBracket)
  {
    std::vector<ParsedTag> result;

    // find all tag positions
    auto allTags = findRawTags(svgData, openingBracket, closingBracket);

    // parse each tag
    for (const auto& tag : allTags)
    {
      auto type = determineTagTypeFromName(tag.name);

      if (type == TagType::Match)
      {
        auto mTag = parseMatchTag(tag.name);
        //ParsedTag p{type, tag, mTag, PlayerTag{}, RankTag{}};
        ParsedTag p{type, tag, mTag};
        std::cout << tag.name << " --> Match tag with match num " << mTag.bracketMatchNum
                  << ", round " << mTag.roundNum << ", loser rank " << mTag.loserRank
                  << " and winner rank " << mTag.winnerRank << std::endl;

        result.push_back(std::move(p));
        continue;
      }

      if (type == TagType::Player)
      {
        auto pTag = parsePlayerTag(tag.name);
        //ParsedTag p{type, tag, MatchTag{}, pTag, RankTag{}};
        ParsedTag p{type, tag, pTag};
        std::cout << tag.name << " --> Player tag with match num " << pTag.bracketMatchNum
                  << ", player " << pTag.playerPos << ", label pos " << ((pTag.pos == LabelPos::First) ? "First" : "Second")
                  << " source match " << pTag.srcMatch << " and initial rank " << pTag.initialRank << std::endl;

        result.push_back(std::move(p));
        continue;
      }

      if (type == TagType::Rank)
      {
        auto rTag = parseRankTag(tag.name);
        //ParsedTag p{type, tag, MatchTag{}, PlayerTag{}, rTag};
        ParsedTag p{type, tag, rTag};
        std::cout << tag.name << " --> Rank tag with for rank " << rTag.rank
                  << ", label pos " << ((rTag.pos == LabelPos::First) ? "First" : "Second") << std::endl;

        result.push_back(std::move(p));
        continue;
      }

      if (type == TagType::Other)
      {
        //ParsedTag p{type, tag, MatchTag{}, PlayerTag{}, RankTag{}};
        ParsedTag p{type, tag, std::string{tag.name}};
        std::cout << tag.name << " --> Other tag" << std::endl;

        result.push_back(std::move(p));
        continue;
      }

      throw std::invalid_argument("Tag " + tag.name + " not handled!");
    }

    return result;
  }

  //----------------------------------------------------------------------------

  std::optional<string> consistencyCheck(const std::vector<ParsedTag> allTags)
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
    int prevRound{1};
    for (int n=1; n <= ma.size(); ++n)
    {
      const auto& mTag = get<MatchTag>(ma.at(n-1).content);
      if (mTag.bracketMatchNum != n)
      {
        return "Inconsistent match numbers";
      }

      if (mTag.roundNum < 1)
      {
        return "Round number for match " + to_string(mTag.bracketMatchNum) + " invalid";
      }

      if (mTag.roundNum < prevRound)
      {
        return "Round number for match " + to_string(mTag.bracketMatchNum) + " inconsistent";
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

      if ((p1.initialRank + p2.initialRank) != (frp.size() + 1))
      {
        return "Inconsistent initial ranks in match " + to_string(p1.bracketMatchNum) + " (rank sum incorrect)";
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
        return "Player " + to_string(pl.playerPos) + " of match " + to_string(pl.bracketMatchNum) +
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

      if (abs(pl.srcMatch) >= allMatches.size())
      {
        return "Player " + to_string(pl.playerPos) + " of match " + to_string(pl.bracketMatchNum) +
               " references an invalid match number";
      }

      if (pl.srcMatch > 0)
      {
        winnerRefs.at(pl.srcMatch - 1) += 1;
      }
      if (pl.srcMatch < 0)
      {
        loserRefs.at(-pl.srcMatch - 1) += 1;
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
        return "Player " + to_string(pl.playerPos) + " of match " + to_string(pl.bracketMatchNum) +
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
          return "Match " + to_string(ma.bracketMatchNum) + " has an invalid loser rank";
        }
        cntRank.at(ma.loserRank - 1) += 1;
      } else {
        if (!hasFollowUpMatch(-ma.bracketMatchNum))
        {
          return "Loser of match " + to_string(ma.bracketMatchNum) + " has no final rank and no follow-up match";
        }
      }

      if (ma.winnerRank > 0)
      {
        if (ma.winnerRank > nPlayers)
        {
          return "Match " + to_string(ma.bracketMatchNum) + " has an invalid winner rank";
        }
        cntRank.at(ma.winnerRank - 1) += 1;
      } else {
        if (!hasFollowUpMatch(ma.bracketMatchNum))
        {
          return "Winner of match " + to_string(ma.bracketMatchNum) + " has no final rank and no follow-up match";
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
