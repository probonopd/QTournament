#include <iostream>
#include <stdexcept>
#include <regex>
#include <algorithm>
#include <functional>
#include <tuple>

#include <QFile>

#include <Sloppy/Memory.h>

#include <SimpleReportGeneratorLib/SimpleReportGenerator.h>

#include "HelperFunc.h"
#include "SvgBracket.h"
#include "MatchMngr.h"
#include "BracketMatchData.h"
#include "BackendAPI.h"

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
        /*std::cout << tag.name << " --> Match tag with match num " << mTag.bracketMatchNum.get()
                  << ", round " << mTag.roundNum.get() << ", loser rank " << mTag.loserRank.get()
                  << " and winner rank " << mTag.winnerRank.get() << std::endl;*/

        result.push_back(std::move(p));
        continue;
      }

      if (type == TagType::Player)
      {
        auto pTag = parsePlayerTag(tag.name);
        //ParsedTag p{type, tag, MatchTag{}, pTag, RankTag{}};
        ParsedTag p{type, tag, pTag};
        /*std::cout << tag.name << " --> Player tag with match num " << pTag.bracketMatchNum.get()
                  << ", player " << pTag.playerPos << ", label pos " << ((pTag.pos == LabelPos::First) ? "First" : "Second")
                  << " source match " << pTag.srcMatch.get() << " and initial rank " << pTag.initialRank.get() << std::endl;*/

        result.push_back(std::move(p));
        continue;
      }

      if (type == TagType::Rank)
      {
        auto rTag = parseRankTag(tag.name);
        //ParsedTag p{type, tag, MatchTag{}, PlayerTag{}, rTag};
        ParsedTag p{type, tag, rTag};
        /*std::cout << tag.name << " --> Rank tag with for rank " << rTag.rank.get()
                  << ", label pos " << ((rTag.pos == LabelPos::First) ? "First" : "Second") << std::endl;*/

        result.push_back(std::move(p));
        continue;
      }

      if (type == TagType::Other)
      {
        //ParsedTag p{type, tag, MatchTag{}, PlayerTag{}, RankTag{}};
        ParsedTag p{type, tag, std::string{tag.name}};
        //std::cout << tag.name << " --> Other tag" << std::endl;

        result.push_back(std::move(p));
        continue;
      }

      throw std::invalid_argument("Tag " + tag.name + " not handled!");
    }

    return result;
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
            27.0,
            2.1,
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
      },
      {
        SvgBracketMatchSys::SingleElim,
        16,
        {
          {
            297.0,
            210.0,
            ":/brackets/SingleElim16.svg",
            "",
            51.5,
            2.6,
            findRawTagsInResource(":/brackets/SingleElim16.svg"),
          }
        },
        {
          GroupNum_L16,
          GroupNum_Quarter,
          GroupNum_Semi,
          GroupNum_Final,
        }
      },
      {
        SvgBracketMatchSys::SingleElim,
        8,
        {
          {
            297.0,
            210.0,
            ":/brackets/SingleElim8.svg",
            "",
            62.0,
            3.2,
            findRawTagsInResource(":/brackets/SingleElim8.svg"),
          }
        },
        {
          GroupNum_Quarter,
          GroupNum_Semi,
          GroupNum_Final,
        }
      },
      {
        SvgBracketMatchSys::SingleElim,
        4,
        {
          {
            297.0,
            210.0,
            ":/brackets/SingleElim4.svg",
            "",
            72.0,
            3.2,
            findRawTagsInResource(":/brackets/SingleElim4.svg"),
          }
        },
        {
          GroupNum_Semi,
          GroupNum_Final,
        }
      },
      {
        SvgBracketMatchSys::FinalAnd3rd,
        4,
        {
          {
            297.0,
            210.0,
            ":/brackets/SingleElim2.svg",
            "",
            72.0,
            3.2,
            findRawTagsInResource(":/brackets/SingleElim2.svg"),
          }
        },
        {
          GroupNum_Final,
        }
      },
      {
        SvgBracketMatchSys::SemiWithRanks,
        10,
        {
          {
            297.0,
            210.0,
            ":/brackets/SemiWithRanks.svg",
            "",
            65.0,
            3.2,
            findRawTagsInResource(":/brackets/SemiWithRanks.svg"),
          }
        },
        {
          GroupNum_Semi,
          GroupNum_Final,
        }
      },
      {
        SvgBracketMatchSys::FinalsWithRanks,
        10,
        {
          {
            297.0,
            210.0,
            ":/brackets/FinalsWithRanks.svg",
            "",
            65.0,
            3.2,
            findRawTagsInResource(":/brackets/FinalsWithRanks.svg"),
          }
        },
        {
          GroupNum_Final,
        }
      },
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

  std::pair<QString, QString> pair2bracketLabel(const PlayerPair& pp)
  {
    const QString a{pp.getPlayer1().getDisplayName()};
    if (pp.hasPlayer2())
    {
      const QString b{pp.getPlayer2().getDisplayName()};
      return std::pair{a, b};
    }

    return std::pair{a, QString{}};
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

  std::tuple<std::string, std::string> findRankTags(const std::vector<ParsedTag>& tagList, Rank r)
  {
    using namespace std::placeholders;

    // a generic predicate for finding ranks in a list
    // of parsed tags
    auto rankPred = [&r](const ParsedTag& t, LabelPos lp)
    {
      if (t.type != TagType::Rank) return false;

      const RankTag& rt = std::get<RankTag>(t.content);
      if (rt.rank != r) return false;
      return (rt.pos == lp);
    };

    // lambda for the actual search function incl.
    // exception throwing
    auto findRank = [&](LabelPos pos) -> std::string
    {
      auto pred = bind(rankPred, _1, pos);
      auto it = std::find_if(begin(tagList), end(tagList), pred);
      if (it == end(tagList))
      {
        return "";
      }

      return it->src.name;
    };

    // find all tags
    const string tagName1 = findRank(LabelPos::First);

    if (tagName1.empty())
    {
      return {"", ""};  // no tag for this rank
    }

    const string tagName2 = findRank(LabelPos::Second);

    return std::tuple{tagName1, tagName2};
  }

  //----------------------------------------------------------------------------

  std::vector<SvgPageDescr> applySvgSubstitution(const std::vector<SvgBracketPage>& pages, const std::unordered_map<string, QString>& dict)
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
          dstPage.content.append(it->second.toStdString());
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

  void addCommonTagsToSubstDict(std::unordered_map<string, QString>& dict, const CommonBracketTags& cbt)
  {
    static const auto tzDb = Sloppy::DateTime::getPopulatedTzDatabase();
    static const auto tzPtr = tzDb.time_zone_from_region("Europe/Berlin");

    dict.insert_or_assign("tnmtName", stdString2QString(cbt.tnmtName));
    dict.insert_or_assign("club", stdString2QString(cbt.club));
    dict.insert_or_assign("catName", stdString2QString(cbt.catName));
    dict.insert_or_assign("subtitle", stdString2QString(cbt.subtitle));

    Sloppy::DateTime::LocalTimestamp now{tzPtr};

    if (cbt.time)
    {
      dict.insert_or_assign("time", stdString2QString(cbt.time->getFormattedString("%R")));
    } else {
      dict.insert_or_assign("time", stdString2QString(now.getFormattedString("%R")));
    }

    if (cbt.date)
    {
      auto [y,m,d] = Sloppy::DateTime::YearMonthDayFromInt(cbt.date.value());
      Sloppy::DateTime::UTCTimestamp tmp{y, m, d, 12, 0, 0};
      dict.insert_or_assign("date", stdString2QString(tmp.getFormattedString("%d.%m.%Y")));
    } else {
      dict.insert_or_assign("date", stdString2QString(now.getFormattedString("%d.%m.%Y")));
    }
  }

  //----------------------------------------------------------------------------

  std::vector<SvgPageDescr> substSvgBracketTags(const TournamentDB& db, SvgBracketMatchSys msys, const PlayerPairList& seed, const std::vector<MatchDispInfo>& maList, const CommonBracketTags& cbt)
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

    // assign the matches, if requested
    std::vector<Match> basicMatchList;
    for (const auto& ma : maList)
    {
      if (ma.propagateWinnerLoser) basicMatchList.push_back(ma.ma);
    }
    allMatches.applyMatches(basicMatchList);

    // prepare a mapping between bracket match number
    // and real match
    std::unordered_map<int, MatchDispInfo> brNum2MatchDispInfo;
    for (const auto& mdi : maList)
    {
      brNum2MatchDispInfo.insert_or_assign(mdi.ma.bracketMatchNum()->get(), mdi);
    }

    // extract ranks from the matches
    MatchList matchesWithRankDisplay;
    for (const auto& mdi : maList)
    {
      if (mdi.showRanks) matchesWithRankDisplay.push_back(mdi.ma);
    }
    std::vector<SimplifiedRankingEntry> ranks = API::Qry::extractSortedRanksFromMatchList(matchesWithRankDisplay);


    // for rare cases in which a players earns a rank without
    // playing a match (e.g., short bracket, one round only,
    // and an odd number of players), we need to check for
    // "assigned player + dead branch + winner rank"
    const auto ranksFromBracket = API::Qry::extractSortedRanksFromBracket(allMatches);
    std::copy(begin(ranksFromBracket), end(ranksFromBracket), back_inserter(ranks));

    // a lambda for finding a match tag based on the
    // bracket match number
    auto matchTagByMatchNum = [&](const BracketMatchNumber& brNum)
    {
      auto pred = [&](const ParsedTag& tag)
      {
        if (tag.type != TagType::Match) return false;

        const MatchTag& ma = std::get<MatchTag>(tag.content);
        return (ma.bracketMatchNum == brNum);
      };

      // find the tag for "brNum"
      auto it = find_if(begin(parsedTags), end(parsedTags), pred);
      return *it;
    };

    // determine font size and text width for names
    //
    // FIX ME: we use constant values for all tags; in a proper
    // implementation this should be tag-specific or at least page-specific
    auto dummyRep = make_unique<SimpleReportLib::SimpleReportGenerator>(100, 100, 10);
    dummyRep->startNextPage();
    const double fntHeight = brDef->pages[0].bracketTextHeight_mm;
    const double maxNameWidth = brDef->pages[0].maxNameLen_mm;

    // iterate over all matches
    // and store the substitution strings in a dictionary
    std::unordered_map<std::string, QString> dict;
    addCommonTagsToSubstDict(dict, cbt);
    /*cout << "#################################" << endl;
    cout << "## substSvgBracketTags (final) ##" << endl;
    cout << "##################################" << endl;*/
    for (const auto& bmd : allMatches)
    {
      //cout << bmd << endl;

      // collect all data about this bracket match
      BracketMatchVisData vis;
      vis.brNum = bmd.matchNum();
      vis.canonicalName = bmd.canoName();
      auto itMatchDispData = brNum2MatchDispInfo.find(vis.brNum.get());
      if (itMatchDispData != brNum2MatchDispInfo.end())
      {
        int maNum = itMatchDispData->second.ma.getMatchNumber();
        if (maNum != MatchNumNotAssigned) vis.realMatchNum = maNum;

        vis.mdi = itMatchDispData->second;
      }
      for (int pos = 1; pos != 3; ++pos)
      {
        auto& descr = (pos == 1) ? vis.p1 : vis.p2;
        const auto ppId = (pos == 1) ? bmd.assignedPair1() : bmd.assignedPair2();

        if (ppId.get() > 0)
        {
          descr.pairId = ppId;
        }
        if (bmd.round() == 1)
        {
          descr.role = PairRole::Initial;
        } else {
          descr.role = (pos == 1) ? bmd.inLink1().role : bmd.inLink2().role;
        }

        if (bmd.round() > 1)
        {
          const auto& inLink = (pos == 1) ? bmd.inLink1() : bmd.inLink2();

          const auto& srcMatch = allMatches.at(inLink.srcMatch.get() - 1);
          descr.srcCanonicalName = srcMatch.canoName();
          auto it = brNum2MatchDispInfo.find(inLink.srcMatch.get());
          if (it != brNum2MatchDispInfo.end())
          {
            int maNum = it->second.ma.getMatchNumber();
            if (maNum != MatchNumNotAssigned) descr.srcRealMatchNum = maNum;
          }
        }
      }

      // get the tag names for which we need substitution strings
      auto [p1a, p1b, p2a, p2b] = findPlayerTagsforMatchNum(parsedTags, bmd.matchNum());
      const auto& maTag = matchTagByMatchNum(bmd.matchNum());

      // assign all substitution strings
      auto items = defSubstStringsForBracketElement(db, bmd, vis);
      const std::vector<std::pair<std::string, QString>> tag2value {
        {p1a.src.name, items.p1aName},
        {p1b.src.name, items.p1bName},
        {p2a.src.name, items.p2aName},
        {p2b.src.name, items.p2bName},
      };
      for (const auto& [tagName, tagVal] : tag2value)
      {
        if (tagVal.isEmpty()) continue;

        dict[tagName] = dummyRep->shortenTextToWidth(tagVal, fntHeight, false, maxNameWidth);
      }
      if (!items.matchText.isEmpty()) dict[maTag.src.name] = items.matchText;
    }

    // fill the rank tags
    for (const auto& sre : ranks)
    {
      auto [tagName1, tagName2] = findRankTags(parsedTags, sre.rank);
      if (tagName1.empty()) continue;  // no tag for this rank

      PlayerPair pp{db, sre.ppId.get()};
      auto [a, b] = pair2bracketLabel(pp);
      if (b.isEmpty())
      {
        // single players on the bottom row, leave the top row empty
        dict[tagName2] = dummyRep->shortenTextToWidth(a, fntHeight, false, maxNameWidth);
      } else {
        // doubles
        dict[tagName1] = dummyRep->shortenTextToWidth(a, fntHeight, false, maxNameWidth);
        dict[tagName2] = dummyRep->shortenTextToWidth(b, fntHeight, false, maxNameWidth);
      }
    }

    return applySvgSubstitution(brDef->pages, dict);
  }

  //----------------------------------------------------------------------------

  BracketElementTextItems defSubstStringsForBracketElement(const TournamentDB& db, const BracketMatchData& bmd, const BracketMatchVisData& vis)
  {
    BracketElementTextItems result;

    // if this is a bracket without match, display pair names or symbolic names
    MatchDispInfo::PairRepresentation pairRep = (vis.mdi.has_value()) ? vis.mdi->pairRep : MatchDispInfo::PairRepresentation::RealOrSymbolic;
    MatchDispInfo::ResultFieldContent resultRep = (vis.mdi.has_value()) ? vis.mdi->resultRep : MatchDispInfo::ResultFieldContent::MatchNumberOnly;

    //
    // deal with the pair tags
    //
    for (int pos=1; pos < 3; ++pos)
    {
      // don't print anything for dead branches
      BracketMatchData::BranchState bState = (pos == 1) ? bmd.pair1State() : bmd.pair2State();
      if (bState == BracketMatchData::BranchState::Dead)
      {
        continue;
      }

      const auto& pairDescr = (pos == 1) ? vis.p1 : vis.p2;

      if (pairRep == MatchDispInfo::PairRepresentation::None)
      {
        // Do nothing; tags without subst string will be implicitly erased later
        continue;
      }

      // print nothing if we need real names but don't have one
      if (!pairDescr.pairId && (pairRep == MatchDispInfo::PairRepresentation::RealNamesOnly))
      {
        continue;
      }

      QString a;
      QString b;

      // print the real name, if we need one and have one
      if (pairDescr.pairId &&
          ((pairRep == MatchDispInfo::PairRepresentation::RealNamesOnly) || (pairRep == MatchDispInfo::PairRepresentation::RealOrSymbolic)))
      {
        PlayerPair pp{db, pairDescr.pairId->get()};
        tie(a, b) = pair2bracketLabel(pp);
      }

      // if "a" is still empty, we definitely need the symbolic name
      if (a.isEmpty())
      {
        if (pairDescr.role == PairRole::Initial)
        {
          a = "--";  // no better symbolic name for un-seeded players in the first round...
        } else {
          a = "(";
          if (pairDescr.role == PairRole::AsWinner)
          {
            a += QObject::tr("Winner of ");
          } else {
            a += QObject::tr("Loser of ");
          }

          // do we have a real match including match number
          // connected with the incoming link?
          if (pairDescr.srcRealMatchNum)
          {
            a += "#" + QString::number(pairDescr.srcRealMatchNum.value());
          } else {
            a += stdString2QString(pairDescr.srcCanonicalName);
          }
          a += ")";
        }
      }

      if (pos == 1) result.p1aName = a;
      else result.p2aName = a;
      if (!b.isEmpty())
      {
        if (pos == 1) result.p1bName = b;
        else result.p2bName = b;
      }
    }

    //
    // Deal with the match tag
    //

    if (resultRep == MatchDispInfo::ResultFieldContent::None) return result;;

    std::optional<MatchScore> score;
    if (vis.mdi.has_value()) score = vis.mdi->ma.getScore();

    // print match number or score?
    bool useScore = (score.has_value() && (resultRep != MatchDispInfo::ResultFieldContent::MatchNumberOnly));

    if (useScore)
    {
      QString tmp = score->toString();
      tmp = tmp.replace(",", ", ");
      result.matchText = tmp;
    } else {
      BracketMatchData::BranchState b1State = bmd.pair1State();
      BracketMatchData::BranchState b2State = bmd.pair2State();

      // if both branches are dead, print no match number
      if ((b1State == BracketMatchData::BranchState::Dead) && (b2State == BracketMatchData::BranchState::Dead))
      {
        return result;
      }

      // if one branch is assigned and the other is dead
      // print no match number because it's a "fast-forward" match
      // with already known winner
      if ((b1State == BracketMatchData::BranchState::Assigned) && (b2State == BracketMatchData::BranchState::Dead))
      {
        return result;
      }
      if ((b1State == BracketMatchData::BranchState::Dead) && (b2State == BracketMatchData::BranchState::Assigned))
      {
        return result;
      }

      // do we have a real match including match number?
      if (vis.realMatchNum)
      {
        result.matchText = "#" + QString::number(vis.realMatchNum.value());
      } else {
        result.matchText = "(" + stdString2QString(vis.canonicalName) + ")";
      }
    }

    return result;
  }
}
