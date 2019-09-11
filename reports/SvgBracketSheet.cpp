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

#include <QList>

#include <SimpleReportGeneratorLib/SimpleReportGenerator.h>
#include <SimpleReportGeneratorLib/TableWriter.h>

#include "SvgBracketSheet.h"
#include "SvgBracket.h"
#include "MatchMngr.h"
#include "CatMngr.h"
#include "../HelperFunc.h"
#include "KO_Config.h"


namespace QTournament
{


SvgBracketSheet::SvgBracketSheet(const TournamentDB& _db, const QString& _name, BracketReportType reportType, const Category& _cat, Round _round)
  :AbstractReport(_db, _name), repType{reportType},
    msys{static_cast<SvgBracketMatchSys>(_cat.getParameter_int(CatParameter::BracketMatchSystem))},
    cat{_cat}, round{_round}
{
  // in "groups with KO", the first round of bracket matches is not "1"
  if (cat.getMatchSystem() == MatchSystem::GroupsWithKO)
  {
    KO_Config ko{cat.getParameter_string(CatParameter::GroupConfig)};
    firstBracketRound = Round{ko.getNumRounds() + 1};
  }

  // For the report type "seeding" the parameter _round
  // is initialized with 0
  // ==> set it to the "real" value of the first round
  if (repType == BracketReportType::Seeding)
  {
    round = firstBracketRound;
  }
  
  roundOffset = cat.getParameter_int(CatParameter::FirstRoundOffset);
}

//----------------------------------------------------------------------------

SvgBracketSheet::~SvgBracketSheet()
{

}

//----------------------------------------------------------------------------

upSimpleReport SvgBracketSheet::regenerateReport()
{
  upSimpleReport result = createEmptyReport_Landscape();

  CatMngr cm{db};
  MatchMngr mm{db};

  // for any report, we need the seeding
  const auto seeding = cm.getSeeding(cat);

  // generate the resulting SVG for several cases
  std::vector<SvgPageDescr> pages;
  if (repType == BracketReportType::Seeding)
  {
    pages = prepReport_Seeding(result.get(), seeding);
  }

  if (repType == BracketReportType::AfterRound)
  {
    pages = prepReport_AfterRound(result.get(), seeding);
  }

  if (repType == BracketReportType::Current)
  {
    pages = prepReport_Current(result.get(), seeding);
  }

  // append all pages to the report
  bool needsNewPage{false};
  for (const auto& pg : pages)
  {
    if (needsNewPage) result->startNextPage();
    result->addSVG_byData_setW(QPointF{0,0}, SimpleReportLib::RECT_CORNER::TOP_LEFT, pg.content, pg.width_mm);
    needsNewPage = true;
  }

  return result;
}

//----------------------------------------------------------------------------

QStringList SvgBracketSheet::getReportLocators() const
{
  QStringList result;

  QString loc = tr("Brackets::");
  loc += cat.getName() + "::";

  if (repType == BracketReportType::Seeding)
  {
    loc += tr("Initial seeding");
  }
  if (repType == BracketReportType::Current)
  {
    loc += tr("Current");
  }
  if (repType == BracketReportType::AfterRound)
  {
    loc += tr("after round ") + QString::number(round.get() + roundOffset);
  }

  result.append(loc);
  return result;
}

//----------------------------------------------------------------------------

SvgBracket::CommonBracketTags SvgBracketSheet::commonTags() const
{
  SvgBracket::CommonBracketTags result;

  auto cfg = SqliteOverlay::KeyValueTab{db, TabCfg};
  result.tnmtName = cfg[CfgKey_TnmtName];
  result.club = cfg[CfgKey_TnmtOrga];
  result.catName = QString2StdString(cat.getName());

  if (repType == BracketReportType::Seeding)
  {
    result.subtitle = QString2StdString(tr("Initial seeding and first matches"));
  }
  if (repType == BracketReportType::Current)
  {
    result.subtitle = QString2StdString(tr("Current status"));
  }
  if (repType == BracketReportType::AfterRound)
  {
    auto special = cat.convertToSpecializedObject();

    QString tmp;
    if (round < special->calcTotalRoundsCount())
    {
      tmp = tr("After round %1 and next matches");
      tmp = tmp.arg(round.get() + roundOffset);
    } else {
      tmp = tr("Final results");
    }

    result.subtitle = QString2StdString(tmp);
  }

  // leave date and time empty

  return result;
}

//----------------------------------------------------------------------------

std::vector<SvgPageDescr> SvgBracketSheet::prepReport_Seeding(SimpleReportLib::SimpleReportGenerator* rep, const PlayerPairList& seeding) const
{
  MatchMngr mm{db};

  // prepare a list bracket matches for the first
  std::vector<SvgBracket::MatchDispInfo> firstRoundMatches;
  const auto mgl = mm.getMatchGroupsForCat(cat, round.get());
  if (!mgl.empty())
  {
    for (const auto& ma : mgl[0].getMatches())
    {
      SvgBracket::MatchDispInfo mdi{
        ma,
        false,
        false,
        SvgBracket::MatchDispInfo::PairRepresentation::RealNamesOnly,
        SvgBracket::MatchDispInfo::ResultFieldContent::MatchNumberOnly
      };
      firstRoundMatches.push_back(mdi);
    }
  }
  return SvgBracket::substSvgBracketTags(db, msys, seeding, firstRoundMatches, commonTags());
}

//----------------------------------------------------------------------------

std::vector<SvgPageDescr> SvgBracketSheet::prepReport_AfterRound(SimpleReportLib::SimpleReportGenerator* rep, const PlayerPairList& seeding) const
{
  // determine the first round that contains bracket matches
  Round curRound{firstBracketRound};

  // prepare a list bracket matches for the all
  // bracket matches up to "our" round
  std::vector<SvgBracket::MatchDispInfo> bracketMatches;
  MatchMngr mm{db};
  for (; curRound <= round; curRound = Round{curRound.get() + 1})
  {
    const auto mgl = mm.getMatchGroupsForCat(cat, curRound.get());
    if (!mgl.empty())
    {
      for (const auto& ma : mgl[0].getMatches())
      {
        SvgBracket::MatchDispInfo mdi{
          ma,
          true,
          true,
          SvgBracket::MatchDispInfo::PairRepresentation::RealNamesOnly,
          SvgBracket::MatchDispInfo::ResultFieldContent::ResultOnly
        };
        bracketMatches.push_back(mdi);
      }
    }
  }

  // if possible, add also the matches of the following round
  // in order to show the next match numbers
  const auto mgl = mm.getMatchGroupsForCat(cat, curRound.get());
  if (!mgl.empty())
  {
    for (const auto& ma : mgl[0].getMatches())
    {
      SvgBracket::MatchDispInfo mdi{
        ma,
        false,
        false,
        SvgBracket::MatchDispInfo::PairRepresentation::RealNamesOnly,
        SvgBracket::MatchDispInfo::ResultFieldContent::MatchNumberOnly
      };
      bracketMatches.push_back(mdi);
    }
  }

  // if possible, add also matches for all further rounds
  // so that we show the (possibly already assigned) match
  // numbers.
  // Names should only be symbolic, though, because we pretend
  // that no further matches have been played
  while (true)
  {
    curRound = Round{curRound.get() + 1};
    const auto mgl = mm.getMatchGroupsForCat(cat, curRound.get());
    if (mgl.empty()) break; // no more rounds
    for (const auto& ma : mgl[0].getMatches())
    {
      SvgBracket::MatchDispInfo mdi{
        ma,
        false,
        false,
        SvgBracket::MatchDispInfo::PairRepresentation::RealOrSymbolic,
        SvgBracket::MatchDispInfo::ResultFieldContent::MatchNumberOnly
      };
      bracketMatches.push_back(mdi);
    }
  }

  return SvgBracket::substSvgBracketTags(db, msys, seeding, bracketMatches, commonTags());
}

//----------------------------------------------------------------------------

std::vector<SvgPageDescr> SvgBracketSheet::prepReport_Current(SimpleReportLib::SimpleReportGenerator* rep, const PlayerPairList& seeding) const
{
  // determine the first round that contains bracket matches
  Round curRound{firstBracketRound};

  // prepare a list bracket matches for the all
  // available bracket matches up to "our" round
  std::vector<SvgBracket::MatchDispInfo> bracketMatches;
  MatchMngr mm{db};
  while (true)
  {
    const auto mgl = mm.getMatchGroupsForCat(cat, curRound.get());
    if (mgl.empty()) break; // done, no more rounds
    for (const auto& mg : mgl)
    {
      for (const auto& ma : mg.getMatches())
      {
        SvgBracket::MatchDispInfo mdi{
          ma,
          true,
          true,
          SvgBracket::MatchDispInfo::PairRepresentation::RealOrSymbolic,
          SvgBracket::MatchDispInfo::ResultFieldContent::ResultOrNumber
        };
        bracketMatches.push_back(mdi);
      }
    }
    curRound = Round{curRound.get() + 1};
  }

  return SvgBracket::substSvgBracketTags(db, msys, seeding, bracketMatches, commonTags());
}

//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


}
