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
#include "BracketMatchData.h"

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

    // assign the matches
    std::vector<Match> basicMatchList;
    for (const auto& ma : maList)
    {
      basicMatchList.push_back(ma.ma);
    }
    allMatches.applyMatches(basicMatchList);

    // prepare a mapping between bracket match number
    // and real match
    std::unordered_map<int, MatchDispInfo> brNum2MatchDispInfo;
    for (const auto& mdi : maList)
    {
      brNum2MatchDispInfo.insert_or_assign(mdi.ma.bracketMatchNum()->get(), mdi);
    }

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

    // iterate over all matches
    // and store the substitution strings in a dictionary
    std::unordered_map<std::string, std::string> dict;
    addCommonTagsToSubstDict(dict, cbt);
    for (const auto& bmd : allMatches)
    {
      // find the associated display info
      // and the match tag
      std::optional<MatchDispInfo> mdi{};
      auto it = brNum2MatchDispInfo.find(bmd.matchNum().get());
      if (it != end(brNum2MatchDispInfo))
      {
        mdi = it->second;
      }
      const auto& maTag = matchTagByMatchNum(bmd.matchNum());

      // get the tag names for which we need substitution strings
      BracketElementTagNames betl;
      auto [p1a, p1b, p2a, p2b] = findPlayerTagsforMatchNum(parsedTags, bmd.matchNum());
      betl.p1aTagName = p1a.src.name;
      betl.p1bTagName = p1b.src.name;
      betl.p2aTagName = p2a.src.name;
      betl.p2bTagName = p2b.src.name;
      betl.matchTagName = maTag.src.name;

      // assign all substitution strings
      defSubstStringsForBracketElement(db, dict, betl, bmd, mdi);
    }

    return applySvgSubstitution(brDef->pages, dict);
  }

  //----------------------------------------------------------------------------

  void defSubstStringsForBracketElement(const TournamentDB& db, std::unordered_map<string, string>& dict, const BracketElementTagNames& betl, const BracketMatchData& bmd, const std::optional<MatchDispInfo>& mdi)
  {
    // if this is a bracket without match, display pair names or symbolic names
    MatchDispInfo::PairRepresentation pairRep = (mdi.has_value()) ? mdi->pairRep : MatchDispInfo::PairRepresentation::RealOrSymbolic;

    //
    // deal with the pair tags
    //
    for (int pos=1; pos < 3; ++pos)
    {
      auto pairId = (pos == 1) ? bmd.assignedPair1() : bmd.assignedPair2();

      if (pairRep == MatchDispInfo::PairRepresentation::None)
      {
        // Do nothing; tags without subst string will be implicitly erased later
        continue;
      }

      // print nothing if we need real names but don't have one
      if ((pairId.get() <= 0) && (pairRep == MatchDispInfo::PairRepresentation::RealNamesOnly))
      {
        continue;
      }

      int symName{0};
      if (mdi)
      {
        symName = (pos == 1) ? mdi->ma.getSymbolicPlayerPair1Name() : mdi->ma.getSymbolicPlayerPair2Name();
      }

      // print nothing if we have neither real names nor symbolic names
      if ((pairId.get() <= 0) && (symName == 0))
      {
        continue;
      }

      string a;
      string b;

      // always use real names, if available
      if (pairId.get() > 0)
      {
        PlayerPair pp{db, pairId.get()};
        a = QString2StdString(pp.getPlayer1().getDisplayName());
        if (pp.hasPlayer2())
        {
          b = QString2StdString(pp.getPlayer2().getDisplayName());
        }
      } else {
        // use symbolic names otherwise
        QString tmp;
        if (symName > 0)
        {
          tmp = QObject::tr("(Winner of #");
        } else {
          tmp = QObject::tr("(Loser of #");
        }
        tmp += QString::number(abs(symName)) + ")";
        a = QString2StdString(tmp);
      }

      dict.insert_or_assign( (pos == 1) ? betl.p1aTagName : betl.p2aTagName, a);
      if (!b.empty())
      {
        dict.insert_or_assign( (pos == 1) ? betl.p1bTagName : betl.p2bTagName, b);
      }
    }

    //
    // Deal with the match tag
    //

    if (!mdi) return;

    if (mdi->resultRep == MatchDispInfo::ResultFieldContent::None)
    {
      return;
    }

    auto score = mdi->ma.getScore();
    if (mdi->resultRep == MatchDispInfo::ResultFieldContent::ResultOnly)
    {
      if (score)
      {
        QString tmp = score->toString();
        tmp = tmp.replace(",", ", ");
        dict[betl.matchTagName] = QString2StdString(tmp);
      }
      return;
    }

    auto maNum = mdi->ma.getMatchNumber();
    if (mdi->resultRep == MatchDispInfo::ResultFieldContent::MatchNumberOnly)
    {
      if (maNum != MatchNumNotAssigned)
      {
        dict[betl.matchTagName] = "#" + to_string(maNum);
      }
      return;
    }

    //
    // print score or match number, with a priority on the score
    //
    if (score)
    {
      QString tmp = score->toString();
      tmp = tmp.replace(",", ", ");
      dict[betl.matchTagName] = QString2StdString(tmp);
      return;
    }

    if (maNum != MatchNumNotAssigned)
    {
      dict[betl.matchTagName] = "#" + to_string(maNum);
      return;
    }
  }

}
