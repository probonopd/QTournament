#include <iostream>
#include <stdexcept>
#include <regex>
#include <algorithm>
#include <functional>

#include <QFile>

#include <Sloppy/Memory.h>

#include "HelperFunc.h"
#include "SvgBracket.h"
#include "MatchMngr.h"

using namespace std;

namespace QTournament::SvgBracket
{

  std::vector<TagData> findRawTagsInResource(const string& resName, const string& openingBracket, const string& closingBracket)
  {
    const QString n{stdString2QString(resName)};

    if (!QFile::exists(n))
    {
      throw std::invalid_argument("findRawTagsInResource(): resource not found");
    }

    QFile data{n};
    if (!data.open(QIODevice::ReadOnly))
    {
      throw std::invalid_argument("findRawTagsInResource(): resource not found");
    }

    const auto allData = data.readAll();
    std::string_view view{allData.data(), static_cast<size_t>(allData.size())};

    return findRawTags(view, openingBracket, closingBracket);
  }

  //----------------------------------------------------------------------------

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
    result.bracketMatchNum = BracketMatchNumber{std::stoi(sm[1].str())};
    result.roundNum = Round{std::stoi(sm[2].str())};

    // check for additional winner / loser ranks
    static const std::regex reWinnerRank{R"(:W(\d+))"};
    static const std::regex reLoserRank{R"(:L(\d+))"};
    if (std::regex_search(tagName, sm, reWinnerRank))
    {
      result.winnerRank = Rank{std::stoi(sm[1].str())};
    }
    if (std::regex_search(tagName, sm, reLoserRank))
    {
      result.loserRank = Rank{std::stoi(sm[1].str())};
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

    result.bracketMatchNum = BracketMatchNumber{std::stoi(sm[1].str())};
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
      result.initialRank = Rank{std::stoi(sm[3].str())};
      return result;
    }
    static const std::regex rePlayerWinnerLoser{R"(P(\d+)\.([12]):([WL])(\d+))"};
    if (std::regex_match(tagName, sm, rePlayerWinnerLoser))
    {
      int maNum = std::stoi(sm[4].str());
      if (sm[3].str() == "L") maNum *= -1;  // losers get a negative source match number
      result.srcMatch = BracketMatchNumber{maNum};

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
      result.rank = Rank{std::stoi(sm[1].str())};
      result.pos = (sm[2].str() == "a") ? LabelPos::First : LabelPos::Second;

      return result;
    }

    throw std::invalid_argument("SvgBracket::parseRankTag(): inconsistent tag name: " + tagName);
  }

  //----------------------------------------------------------------------------

  std::vector<ParsedTag> parseContent(const string_view& svgData, const string& openingBracket, const string& closingBracket)
  {
    // find all tag positions
    auto allTags = findRawTags(svgData, openingBracket, closingBracket);

    return parseRawTagList(allTags);
  }

  //----------------------------------------------------------------------------

  std::vector<ParsedTag> parseRawTagList(const std::vector<TagData>& tagList)
  {
    std::vector<ParsedTag> result;

    // parse each tag
    for (const auto& tag : tagList)
    {
      auto type = determineTagTypeFromName(tag.name);

      if (type == TagType::Match)
      {
        auto mTag = parseMatchTag(tag.name);
        //ParsedTag p{type, tag, mTag, PlayerTag{}, RankTag{}};
        ParsedTag p{type, tag, mTag};
        std::cout << tag.name << " --> Match tag with match num " << mTag.bracketMatchNum.get()
                  << ", round " << mTag.roundNum.get() << ", loser rank " << mTag.loserRank.get()
                  << " and winner rank " << mTag.winnerRank.get() << std::endl;

        result.push_back(std::move(p));
        continue;
      }

      if (type == TagType::Player)
      {
        auto pTag = parsePlayerTag(tag.name);
        //ParsedTag p{type, tag, MatchTag{}, pTag, RankTag{}};
        ParsedTag p{type, tag, pTag};
        std::cout << tag.name << " --> Player tag with match num " << pTag.bracketMatchNum.get()
                  << ", player " << pTag.playerPos << ", label pos " << ((pTag.pos == LabelPos::First) ? "First" : "Second")
                  << " source match " << pTag.srcMatch.get() << " and initial rank " << pTag.initialRank.get() << std::endl;

        result.push_back(std::move(p));
        continue;
      }

      if (type == TagType::Rank)
      {
        auto rTag = parseRankTag(tag.name);
        //ParsedTag p{type, tag, MatchTag{}, PlayerTag{}, rTag};
        ParsedTag p{type, tag, rTag};
        std::cout << tag.name << " --> Rank tag with for rank " << rTag.rank.get()
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

  std::optional<SvgBracketDef> findSvgBracket(SvgBracketMatchSys msys, int nPlayers)
  {
    // statically store all available bracket definitions here
    static const vector<SvgBracketDef> allBrackets
    {
      {
        SvgBracketMatchSys::RankSys,
        16,
        {
          {
            297.0,
            210.0,
            ":/brackets/RankSys16.svg",
            "",
            10.0,
            findRawTagsInResource(":/brackets/RankSys16.svg"),
          }
        },
        {
          GroupNum_Iteration,
          GroupNum_Iteration,
          GroupNum_Iteration,
          GroupNum_Iteration,
          GroupNum_Iteration,
        }
      }
    };

    // for all brackets we need at least two players
    if (nPlayers < 2) return {};

    // iterate over all brackets and look for a suitable match
    //
    // search for the one with the lowest number
    // of maximum players for the best match
    int bestIdx{-1};
    int maxNum{999999};
    for (size_t idx=0; idx < allBrackets.size(); ++idx)
    {
      const auto& br = allBrackets.at(idx);

      if (br.sys != msys) continue;
      if (br.maxNumPlayers < nPlayers) continue;

      if (br.maxNumPlayers < maxNum)
      {
        bestIdx = idx;
        maxNum = br.maxNumPlayers;
      }
    }

    return (bestIdx >= 0) ? allBrackets.at(bestIdx) : std::optional<SvgBracketDef>{};
  }

  //----------------------------------------------------------------------------

  BracketMatchDataList convertToBracketMatches(const SvgBracketDef& def)
  {
    // create a merged list of all tags on all pages
    vector<TagData> allTags;
    for (const auto& pg : def.pages)
    {
      std::copy(begin(pg.rawTags), end(pg.rawTags), back_inserter(allTags));
    }

    // extract player and match data
    auto allPlayers = sortedPlayersFromTagList(allTags);
    auto allMatches = sortedMatchesFromTagList(allTags);

    // helper that determines where a winner / loser of a given match goes to
    auto findNextMatch = [&](const BracketMatchNumber& srcMatch, PairRole ro) -> std::optional<OutgoingBracketLink>
    {
      const int mNum = (ro == PairRole::AsWinner) ? srcMatch.get() : -srcMatch.get();

      auto it = find_if(allPlayers.begin(), allPlayers.end(), [&](const PlayerTag& p)
      {
        return (p.srcMatch == mNum);
      });

      if (it == end(allPlayers)) return std::optional<OutgoingBracketLink>{};

      return OutgoingBracketLink{BracketMatchNumber{it->bracketMatchNum}, it->playerPos};
    };

    // helper function that finds the player tag for a given
    // match and position
    auto findPlayerTag = [&](const BracketMatchNumber& ma, int pos) -> PlayerTag&
    {
      auto it = find_if(begin(allPlayers), end(allPlayers), [&](const PlayerTag& p)
      {
        return ((p.bracketMatchNum == ma.get()) && (p.playerPos == pos));
      });

      return *it;
    };

    // create the match data skeleton
    BracketMatchDataList result;
    std::transform(begin(allMatches), end(allMatches), back_inserter(result), [&](const MatchTag& m)
    {
      // basic match data
      BracketMatchNumber n{m.bracketMatchNum};
      Round r{m.roundNum};

      // what happens to the winner / loser
      std::variant<OutgoingBracketLink, Rank> winnerAction{Rank{-1}};  // dummy
      std::variant<OutgoingBracketLink, Rank> loserAction{Rank{-1}};  // dummy
      if (m.loserRank > 0)
      {
        loserAction = Rank{m.loserRank};
      } else {
        loserAction = findNextMatch(n, PairRole::AsLoser).value();
      }
      if (m.winnerRank > 0)
      {
        winnerAction = Rank{m.winnerRank};
      } else {
        winnerAction = findNextMatch(n, PairRole::AsWinner).value();
      }

      // find the player for this match
      auto p1 = findPlayerTag(n, 1);
      auto p2 = findPlayerTag(n, 2);

      // initial ranks in round 1
      if (m.roundNum == 1)
      {
        Rank ini1{Rank{p1.initialRank}};
        Rank ini2{Rank{p2.initialRank}};

        // insert a bracket match with initial ranks
        return BracketMatchData{n, ini1, ini2, winnerAction, loserAction};
      }

      // match in all rounds later than round 1
      const auto role1 = (p1.srcMatch > 0) ? PairRole::AsWinner : PairRole::AsLoser;
      const BracketMatchNumber srcMatch1{abs(p1.srcMatch.get())};
      const IncomingBracketLink in1{srcMatch1, role1};
      const auto role2 = (p2.srcMatch > 0) ? PairRole::AsWinner : PairRole::AsLoser;
      const BracketMatchNumber srcMatch2{abs(p2.srcMatch.get())};
      const IncomingBracketLink in2{srcMatch2, role1};

      return BracketMatchData{n, r, in1, in2, winnerAction, loserAction};
    });

    for (const auto& bmd : result)
    {
      cout << "#" << bmd.matchNum().get() << ": Round = " << bmd.round().get() << ", ";
      if (bmd.round().get() == 1)
      {
        cout << "P1.ini = " << bmd.initialRank(1).value().get() << ", P2.ini = " << bmd.initialRank(2).value().get() << ", ";
      } else {
        cout << "P1 = ";
        const auto in1 = bmd.inLink1();
        if (in1.role == SvgBracket::PairRole::AsWinner) cout << "W";
        else cout << "L";
        cout << in1.srcMatch.get() << ", ";

        cout << "P2 = ";
        const auto in2 = bmd.inLink2();
        if (in2.role == SvgBracket::PairRole::AsWinner) cout << "W";
        else cout << "L";
        cout << in2.srcMatch.get() << ", ";
      }

      if (bmd.winnerRank())
      {
        cout << "WR = " << bmd.winnerRank()->get() << ", ";
      } else {
        cout << "W --> ";
        const auto& out = bmd.nextWinnerMatch();
        cout << out.dstMatch.get() << "." << out.pos << ", ";
      }
      if (bmd.loserRank())
      {
        cout << "LR = " << bmd.loserRank()->get();
      } else {
        cout << "L --> ";
        const auto& out = bmd.nextLoserMatch();
        cout << out.dstMatch.get() << "." << out.pos;
      }

      cout << endl;
    }

    return result;
  }

  //----------------------------------------------------------------------------

  BracketMatchData::BracketMatchData(const BracketMatchNumber& n, const Rank& p1InitialRank, const Rank& p2InitialRank, const std::variant<OutgoingBracketLink, Rank>& dstWinner, const std::variant<OutgoingBracketLink, Rank>& dstLoser)
    :brMaNum{n}, _round{1}, winnerAction{dstWinner}, loserAction{dstLoser}, src1{p1InitialRank}, src2{p2InitialRank},
      p1State{BranchState::Alive}, p2State{BranchState::Alive}
  {

  }

  //----------------------------------------------------------------------------

  BracketMatchData::BracketMatchData(const BracketMatchNumber& n, const Round& r, const IncomingBracketLink& inLink1, const IncomingBracketLink& inLink2, const std::variant<OutgoingBracketLink, Rank>& dstWinner, const std::variant<OutgoingBracketLink, Rank>& dstLoser)
    :brMaNum{n}, _round{r}, winnerAction{dstWinner}, loserAction{dstLoser}, src1{inLink1}, src2{inLink2},
      p1State{BranchState::Alive}, p2State{BranchState::Alive}
  {

  }

  //----------------------------------------------------------------------------

  std::optional<Rank> BracketMatchData::initialRank(int pos) const
  {
    if (_round.get() != 1) return std::optional<Rank>{};

    return (pos == 1) ? get<Rank>(src1) : get<Rank>(src2);
  }

  //----------------------------------------------------------------------------

  void BracketMatchData::assignPlayerPair(const PlayerPairRefId& ppId, int pos)
  {
    if (pos == 1)
    {
      p1Pair = ppId;
      p1State = BranchState::Assigned;
    } else {
      p2Pair = ppId;
      p2State = BranchState::Assigned;
    }
  }

  //----------------------------------------------------------------------------

  void BracketMatchData::setPairUnused(int pos)
  {
    if (pos == 1)
    {
      p1Pair = PlayerPairRefId{-1};;
      p1State = BranchState::Dead;
    } else {
      p2Pair = PlayerPairRefId{-1};;
      p2State = BranchState::Dead;
    }
    cout << "M " << brMaNum.get() << "." << pos << " = DEAD" << endl;
  }

  //----------------------------------------------------------------------------

  void BracketMatchDataList::applySeeding(const PlayerPairList& seed)
  {
    // step 1:
    // assign player pairs as far as possible
    std::for_each(begin(), end(), [&](BracketMatchData& bmd)
    {
      if (bmd.round().get() == 1)
      {
        for (int pos=1; pos < 3; ++pos)
        {
          Rank iniRank = bmd.initialRank(pos).value();
          if (iniRank.get() <= seed.size())
          {
            const auto& pp = seed.at(iniRank.get() - 1);
            const PlayerPairRefId ppId{pp.getPairId()};
            bmd.assignPlayerPair(ppId, pos);
            cout << "Seed assignment: M " << bmd.matchNum().get() << "." << pos << " = PlayerPair " << ppId.get() << endl;
          } else {
            bmd.setPairUnused(pos);
          }
        }
      }
    });

    // step 2:
    // apply all "fast forwards" in all rounds
    //
    // matches are sorted by number and thus implicitly by rounds ==> it is sufficient to iterate
    // over all matches
    std::for_each(begin(), end(), [&](BracketMatchData& bmd)
    {
      using bs = BracketMatchData::BranchState;

      const auto& p1State = bmd.pair1State();
      const auto& p2State = bmd.pair2State();

      // Case 1: both branches have assigned players
      // ==> nothing to do for us


      // Case 2: both branches are "alive"
      // ==> nothing to do for us, the players depend on
      // other matches


      // Case 3: one branch assigned, one branch dead
      // ==> forward the assigned player to the winner match
      if ((p1State == bs::Assigned) && (p2State == bs::Dead))
      {
        fastForward(bmd, 1);
      }
      if ((p1State == bs::Dead) && (p2State == bs::Assigned))
      {
        fastForward(bmd, 2);
      }


      // Case 4: one branch alive, one branch dead
      // ==> declare the loser branch dead because the
      // one alive player will be "passed through" as winner
      if (((p1State == bs::Alive) && (p2State == bs::Dead)) || ((p1State == bs::Dead) && (p2State == bs::Alive)))
      {
        declareLoserBranchDead(bmd);
      }


      // Case 5: both branches dead
      // ==> declare the winner and loser branches dead as well
      if ((p1State == bs::Dead) && (p2State == bs::Dead))
      {
        if (bmd.hasWinnerMatch())
        {
          const auto& winnerMatchInfo = bmd.nextWinnerMatch();
          auto& winnerMatch = at(winnerMatchInfo.dstMatch.get() - 1);  // only works because the list is SORTED by match number
          winnerMatch.setPairUnused(winnerMatchInfo.pos);
        }

        // declare the loser branch dead
        if (bmd.hasLoserMatch())
        {
          declareLoserBranchDead(bmd);
        }
      }
    });
  }

  //----------------------------------------------------------------------------

  void BracketMatchDataList::fastForward(BracketMatchData& ma, int pos)
  {
    PlayerPairRefId pairToForward = (pos == 1) ? ma.assignedPair1() : ma.assignedPair2();
    assert(pairToForward.get() > 0);

    // assign the player pair 1 of this match to the linked
    // player pair of the winner match
    if (ma.hasWinnerMatch())
    {
      const auto& winnerMatchInfo = ma.nextWinnerMatch();
      auto& winnerMatch = at(winnerMatchInfo.dstMatch.get() - 1);  // only works because the list is SORTED by match number
      winnerMatch.assignPlayerPair(pairToForward, winnerMatchInfo.pos);

      cout << "FastForward " << ma.matchNum().get() << "." << pos << " --> " << winnerMatchInfo.dstMatch.get() << "." << winnerMatchInfo.pos << endl;
    }

    // declare the loser branch dead
    if (ma.hasLoserMatch())
    {
      declareLoserBranchDead(ma);
    }
  }

  //----------------------------------------------------------------------------

  void BracketMatchDataList::declareLoserBranchDead(const BracketMatchData& ma)
  {
    const auto& loserMatchInfo = ma.nextLoserMatch();
    auto& loserMatch = at(loserMatchInfo.dstMatch.get() - 1);
    loserMatch.setPairUnused(loserMatchInfo.pos);
  }

  //----------------------------------------------------------------------------

  std::variant<OutgoingBracketLink, Rank> BracketMatchDataList::traverseForward(const BracketMatchData& ma, PairRole role) const
  {
    using bs = BracketMatchData::BranchState;

    // check whether we can start the traversal at all
    if ((role == PairRole::AsWinner) && !ma.hasWinnerMatch())
    {
      return ma.winnerRank().value();
    }
    if ((role == PairRole::AsLoser) && !ma.hasLoserMatch())
    {
      return ma.loserRank().value();
    }

    // initialize the traversal with the next match number,
    // based on whether we start as a winner or loser
    OutgoingBracketLink curMatchLink = (role == PairRole::AsWinner) ? ma.nextWinnerMatch() : ma.nextLoserMatch();

    // the iteration
    while (true)
    {
      const auto& curMatch = at(curMatchLink.dstMatch.get() - 1);  // only works because the list is SORTED by match number

      const auto& p1State = curMatch.pair1State();
      const auto& p2State = curMatch.pair2State();

      // both branches should NEVER be dead because then
      // we should have never reached them
      assert( ! ((p1State == bs::Dead) && (p2State == bs::Dead)));

      // this is a regular match that can be played
      if ((p1State != bs::Dead) && (p2State != bs::Dead))
      {
        return curMatchLink;
      }

      // one of the branches is dead and thus we traverse
      // in winner direction
      if (!curMatch.hasWinnerMatch())
      {
        return curMatch.winnerRank().value();
      }
      curMatchLink = curMatch.nextWinnerMatch();
    }
  }

  //----------------------------------------------------------------------------

  std::vector<PlayerTag> sortedPlayersFromTagList(const std::vector<TagData>& tagList)
  {
    std::vector<PlayerTag> allPlayers;

    for (const auto& rawTag : tagList)
    {
      auto type = determineTagTypeFromName(rawTag.name);

      if (type == TagType::Player)
      {
        auto pTag = parsePlayerTag(rawTag.name);
        if (pTag.pos == LabelPos::Second) continue;

        allPlayers.push_back(std::move(pTag));
      }
    }

    sort(begin(allPlayers), end(allPlayers), [](const PlayerTag& p1, const PlayerTag& p2)
    {
      if (p1.bracketMatchNum != p2.bracketMatchNum)
      {
        return (p1.bracketMatchNum < p2.bracketMatchNum);
      }

      return (p1.pos == LabelPos::First);  // sort player 1 before player 2 in the same match
    });

    return allPlayers;
  }

  //----------------------------------------------------------------------------

  std::vector<MatchTag> sortedMatchesFromTagList(const std::vector<TagData>& tagList)
  {
    // extract all match and all (primary) player tags
    std::vector<MatchTag> allMatches;

    for (const auto& rawTag : tagList)
    {
      auto type = determineTagTypeFromName(rawTag.name);

      if (type == TagType::Match)
      {
        auto mTag = parseMatchTag(rawTag.name);
        allMatches.push_back(std::move(mTag));
      }
    }

    // sort matches and players by round
    sort(begin(allMatches), end(allMatches), [](const MatchTag& ma1, const MatchTag& ma2)
    {
      return (ma1.bracketMatchNum < ma2.bracketMatchNum);   // implies sorting by round
    });

    return allMatches;
  }

  //----------------------------------------------------------------------------

  std::pair<string, string> pair2bracketLabel(const PlayerPair& pp)
  {
    const string a{QString2StdString(pp.getPlayer1().getDisplayName())};
    if (pp.hasPlayer2())
    {
      const string b{QString2StdString(pp.getPlayer2().getDisplayName())};
      return std::pair{a, b};
    }

    return std::pair{a, std::string{}};
  }

  //----------------------------------------------------------------------------

  std::vector<SvgPageDescr> substSvgBracketTags(SvgBracketMatchSys msys, const PlayerPairList& seed, const CommonBracketTags& cbt, const std::vector<std::pair<BracketMatchNumber, int> >& maNumMapping)
  {
    // retrieve the bracket definition for the selected system
    // and the required number of players
    auto brDef = SvgBracket::findSvgBracket(msys, seed.size());
    if (!brDef)
    {
      throw std::runtime_error("substSvgBracketTags(): no suitable bracket"); // this should never happen
    }

    // merge all tags into one large list and parse it
    std::vector<SvgBracket::TagData> allTags;
    for (const auto& pg : brDef->pages)
    {
      std::copy(begin(pg.rawTags), end(pg.rawTags), back_inserter(allTags));
    }
    auto parsedTags = SvgBracket::parseRawTagList(allTags);

    // generate the bracket data for the player list
    //
    // matches come already sorted by match number
    auto allMatches = SvgBracket::convertToBracketMatches(*brDef);

    // assign the intial seeding to the bracket.
    // this automatically tags all unused matches as either "fast forward" (only
    // one player) or "empty" (no players at all)
    allMatches.applySeeding(seed);

    // a lambda for a reverse-search in the seeding list:
    // find a player pair by its ID, not by its rank
    auto pairById = [&](const PlayerPairRefId& ppId)
    {
      const int id = ppId.get();
      auto it = find_if(begin(seed), end(seed), [&](const PlayerPair& pp)
      {
        return (pp.getPairId() == id);
      });

      return *it;
    };

    // iterate over all matches that have a player pair assigned
    // and store the substitution string in a dictionary
    std::unordered_map<std::string, std::string> dict;
    addCommonTagsToSubstDict(dict, cbt);
    for (const auto& ma : allMatches)
    {
      auto pair1Id = ma.assignedPair1();
      auto pair2Id = ma.assignedPair2();

      // skip matches that don't have any players assigned
      if ((pair1Id.get() <= 0) && (pair2Id.get() <= 0)) continue;

      // get the player tags for this match
      auto [p1a, p1b, p2a, p2b] = findPlayerTagsforMatchNum(parsedTags, ma.matchNum());

      if (pair1Id.get() > 0)
      {
        auto pp1 = pairById(pair1Id);
        dict[p1a.src.name] = QString2StdString(pp1.getPlayer1().getDisplayName());
        if (pp1.hasPlayer2())
        {
          dict[p1b.src.name] = QString2StdString(pp1.getPlayer2().getDisplayName());
        }
      }
      if (pair2Id.get() > 0)
      {
        auto pp2 = pairById(pair2Id);
        dict[p2a.src.name] = QString2StdString(pp2.getPlayer1().getDisplayName());
        if (pp2.hasPlayer2())
        {
          dict[p2b.src.name] = QString2StdString(pp2.getPlayer2().getDisplayName());
        }
      }
    }

    // include match numbers, if available
    for (const auto& maNumMap : maNumMapping)
    {
      auto pred = [](const ParsedTag& tag, const BracketMatchNumber n)
      {
        if (tag.type != TagType::Match) return false;

        const MatchTag& ma = std::get<MatchTag>(tag.content);
        return (ma.bracketMatchNum == n);
      };

      auto _pred = bind(pred, std::placeholders::_1, maNumMap.first);

      // find the tag for "brNum"
      auto it = find_if(begin(parsedTags), end(parsedTags), _pred);
      if (it != end(parsedTags))
      {
        dict[it->src.name] = "#" + to_string(maNumMap.second);
      }
    }

    return applySvgSubstitution(brDef->pages, dict);
  }

  //----------------------------------------------------------------------------

  std::tuple<ParsedTag, ParsedTag, ParsedTag, ParsedTag> findPlayerTagsforMatchNum(const std::vector<ParsedTag>& tagList, BracketMatchNumber brNum)
  {
    using namespace std::placeholders;

    // a generic predicate for finding players in a list
    // of parsed tags
    auto playerPred = [&brNum](const ParsedTag& t, int playerPos, LabelPos lp)
    {
      if (t.type != TagType::Player) return false;

      const PlayerTag& p = std::get<PlayerTag>(t.content);
      if (p.bracketMatchNum != brNum) return false;
      if (p.playerPos != playerPos) return false;
      return (p.pos == lp);
    };

    // lambda for the actual search function incl.
    // exception throwing
    auto findPlayer = [&](int playerPos, LabelPos pos) -> ParsedTag
    {
      auto pred = bind(playerPred, _1, playerPos, pos);
      auto it = std::find_if(begin(tagList), end(tagList), pred);
      if (it == end(tagList))
      {
        throw std::invalid_argument("findPlayerTagsforMatchNum(): invalid match number, no players found");
      }

      return *it;
    };

    // find all players
    const ParsedTag p1a = findPlayer(1, LabelPos::First);
    const ParsedTag p1b = findPlayer(1, LabelPos::Second);
    const ParsedTag p2a = findPlayer(2, LabelPos::First);
    const ParsedTag p2b = findPlayer(2, LabelPos::Second);

    return std::tuple{p1a, p1b, p2a, p2b};
  }

  //----------------------------------------------------------------------------

  std::vector<SvgPageDescr> applySvgSubstitution(const std::vector<SvgBracketPage>& pages, const std::unordered_map<string, string>& dict)
  {
    std::vector<SvgPageDescr> result;

    for (const auto& srcPage : pages)
    {
      SvgPageDescr dstPage{srcPage};

      // calculate the net size difference that
      // results from replacing tags with content
      int sizeDiff{0};
      for (const auto& tag : srcPage.rawTags)
      {
        // substract the tag itself plus the surrounding brackets
        sizeDiff -= tag.name.size();
        sizeDiff -= 4;

        // do we have a substitution string?
        auto it = dict.find(tag.name);
        if (it != end(dict))
        {
          sizeDiff += it->second.size();
        }
      }

      // read the input data
      QFile svgFile{stdString2QString(srcPage.resName)};
      if (!svgFile.open(QIODevice::OpenModeFlag::ReadOnly))
      {
        throw std::runtime_error("applySvgSubstitution(): resource not found"); // should never happen
      }
      auto allContent = svgFile.readAll();
      Sloppy::MemView contentView{allContent.data(), static_cast<size_t>(allContent.size())};

      // prepare the memory for the target data
      dstPage.content.clear();
      dstPage.content.reserve(contentView.size() + sizeDiff);

      // replace all tags with data
      size_t srcIdx{0};
      for (const auto& tag : srcPage.rawTags)
      {
        // copy everything up to the tag
        if (srcIdx < tag.idxStart)
        {
          auto slice = contentView.slice_byIdx(srcIdx, tag.idxStart - 1);
          dstPage.content.append(slice.to_charPtr(), slice.size());
        }

        // if we have a replacement string, insert it
        auto it = dict.find(tag.name);
        if (it != end(dict))
        {
          dstPage.content.append(it->second);
        }

        // forward the source to the character right
        // after the tag
        srcIdx = tag.idxEnd + 1;
      }

      // append everything after the last tag
      if (srcIdx < contentView.size())
      {
        auto slice = contentView.slice_byIdx(srcIdx, contentView.size() - 1);
        dstPage.content.append(slice.to_charPtr(), slice.size());
      }

      // store the page, done
      result.push_back(std::move(dstPage));
    }

    return result;
  }

  //----------------------------------------------------------------------------

  void addCommonTagsToSubstDict(std::unordered_map<string, string>& dict, const CommonBracketTags& cbt)
  {
    static const auto tzDb = Sloppy::DateTime::getPopulatedTzDatabase();
    static const auto tzPtr = tzDb.time_zone_from_region("Europe/Berlin");

    dict.insert_or_assign("tnmtName", cbt.tnmtName);
    dict.insert_or_assign("club", cbt.club);
    dict.insert_or_assign("catName", cbt.catName);
    dict.insert_or_assign("subtitle", cbt.subtitle);

    Sloppy::DateTime::LocalTimestamp now{tzPtr};

    if (cbt.time)
    {
      dict.insert_or_assign("time", cbt.time->getFormattedString("%R"));
    } else {
      dict.insert_or_assign("time", now.getFormattedString("%R"));
    }

    if (cbt.date)
    {
      auto [y,m,d] = Sloppy::DateTime::YearMonthDayFromInt(cbt.date.value());
      Sloppy::DateTime::UTCTimestamp tmp{y, m, d, 12, 0, 0};
      dict.insert_or_assign("date", tmp.getFormattedString("%d.%m.%Y"));
    } else {
      dict.insert_or_assign("date", now.getFormattedString("%d.%m.%Y"));
    }
  }




}
