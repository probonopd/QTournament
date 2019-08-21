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

#include <tuple>

#include <QList>

#include "BracketSheet.h"
#include <SimpleReportGeneratorLib/SimpleReportGenerator.h>
#include <SimpleReportGeneratorLib/TextStyle.h>

#include "ui/GuiHelpers.h"
#include "Match.h"
#include "MatchMngr.h"
#include "BracketGenerator.h"
#include "PlayerMngr.h"
#include "HelperFunc.h"

using namespace SimpleReportLib;


namespace QTournament
{

  constexpr char BracketSheet::BracketStyle[];
  constexpr char BracketSheet::BracketStyleItalics[];
  constexpr char BracketSheet::BracketStyleBold[];


BracketSheet::BracketSheet(const TournamentDB& _db, const QString& _name, const Category& _cat)
  :AbstractReport(_db, _name), cat(_cat), rawReport(nullptr)
{
  // make sure the requested category has bracket visualization data
  auto bvd = BracketVisData::getExisting(cat);
  if (!bvd)
  {
    throw std::runtime_error("Requested bracket report for a category without bracket visualization data!");
  }
}

//----------------------------------------------------------------------------

upSimpleReport BracketSheet::regenerateReport()
{
  // get the handle of the overall bracket visualization data
  auto bvd = BracketVisData::getExisting(cat);
  if (!bvd)
  {
    auto result = createEmptyReport_Portrait();
    setHeaderAndHeadline(result.get(), "Tournament Bracket");
    result->writeLine(tr("No visualization data available for this category."));
    return result;
  }

  // update/fill potentially missing names in the bracket
  bvd->fillMissingPlayerNames();


  BracketPageOrientation pgOrientation;
  BracketLabelPos labelPos;

  // get the orientation of the first page
  //
  // FIX: as of today, all pages in a report must have the same orientation!
  // Even if the orientation information in the database changes from page to page,
  // the report will still use the same orientation as for the first page.
  tie(pgOrientation, labelPos) = bvd->getPageInfo(0);

  // initialize the report's first page
  upSimpleReport rep = (pgOrientation == BracketPageOrientation::Landscape) ? createEmptyReport_Landscape() : createEmptyReport_Portrait();
  rawReport = rep.get();

  // determine the conversion factor between "grid units" and "paper units" (millimeter)
  determineGridSize();

  // setup the text style, including a dynamically adapted text height
  setupTextStyle();

  // two helper-lambdas for drawing horizontal and vertical lines
  // using grid units instead of millimeters
  auto drawHorLine = [&](int gridX0, int gridY0, int gridXLen) {
    double x0;
    double y0;
    tie(x0, y0) = grid2MM(gridX0, gridY0);
    rep->drawHorLine(x0, y0, gridXLen * xFac);
  };
  auto drawVertLine = [&](int gridX0, int gridY0, int gridYLen) {
    double x0;
    double y0;
    tie(x0, y0) = grid2MM(gridX0, gridY0);
    rep->drawVertLine(x0, y0, gridYLen * yFac);
  };

  // create all pages in advance
  //
  // we need this later to continue a bracket on page 1 although we're
  // currently painting the bracket elements for page 0, for instance
  for (int idxPage=0; idxPage < bvd->getNumPages(); ++idxPage)
  {
    if (idxPage > 0)
    {
      rep->startNextPage();
    }
  }

  // loop over all pages and bracket elements and draw them one by one
  for (int idxPage=0; idxPage < bvd->getNumPages(); ++idxPage)
  {
    rawReport->setActivePage(idxPage);

    for (const BracketVisElement& el : bvd->getVisElements(idxPage))
    {
      int x0 = el.getGridX0();
      int y0 = el.getGridY0();
      int spanY = el.getSpanY();
      int yPageBreakSpan = el.getYPageBreakSpan();
      BracketOrientation orientation = el.getOrientation();
      BracketTerminator terminator = el.getTerminator();

      int xLen = -1;
      if (orientation == BracketOrientation::Right)
      {
        xLen = 1;
      }

      // if the element spans multiple pages, adjust the "span"
      // value accordingly
      if (yPageBreakSpan > 0)
      {
        spanY = yPageBreakSpan;
      }

      // draw the "open rectangle", but draw the bottom horizontal
      // line only if we don't have a page break
      drawHorLine(x0, y0, xLen);
      drawVertLine(x0 + xLen, y0, spanY);
      if (yPageBreakSpan == 0) drawHorLine(x0, y0 + spanY, xLen);

      // restore the original spanY value for the further steps
      if (yPageBreakSpan != 0)
      {
        spanY = el.getSpanY();
      }

      // if we have a bracket element that spans multiple pages,
      // draw the lines on the second page
      if (yPageBreakSpan != 0)
      {
        // jump to the continuation page
        rawReport->setActivePage(el.getNextPageNum());

        // draw the remaining part of the bracket element
        int remainingYSpan = spanY - yPageBreakSpan;
        drawVertLine(x0 + xLen, 0, remainingYSpan);
        drawHorLine(x0, remainingYSpan, xLen);

        // return to our current page
        rawReport->setActivePage(idxPage);
      }

      // draw the terminator, if any
      int termOffset = el.getTerminatorOffset();
      if (terminator == BracketTerminator::Outwards)
      {
        drawHorLine(x0 + xLen, y0 + spanY / 2 + termOffset, xLen);
      }
      if (terminator == BracketTerminator::Inwards)
      {
        drawHorLine(x0 + xLen, y0 + spanY / 2 + termOffset, -xLen);
      }

      // draw the initial rank, if any
      int iniRank = el.getInitialRank1();
      if (iniRank > 0)
      {
        drawBracketTextItem(x0, y0, spanY, orientation,
                            QString::number(iniRank) + ".",
                            BracketTextElement::InitialRank1);
      }
      iniRank = el.getInitialRank2();
      if (iniRank > 0)
      {
        drawBracketTextItem(x0, y0, spanY, orientation,
                            QString::number(iniRank) + ".",
                            BracketTextElement::InitialRank2);
      }

      //
      // print the actual or symbolic player names, if any
      //
      PlayerMngr pm{db};
      QString pairName;
      bool isSymbolic = false;
      int ppId = determineEffectivePlayerPairId(el, 1);
      if (ppId < 0)
      {
        pairName = determineSymbolicPlayerPairDisplayText(el, 1);
        isSymbolic = true;
      } else {
        PlayerPair pp = pm.getPlayerPair(ppId);
        pairName = getTruncatedPlayerName(pp, xFac - 2 * GapLineTxt_mm, rawReport->getTextStyle(BracketStyle));
      }
      drawBracketTextItem(x0, y0, spanY, orientation, pairName, BracketTextElement::Pair1,
                          isSymbolic ? BracketStyleItalics : QString());

      ppId = determineEffectivePlayerPairId(el, 2);
      isSymbolic = false;
      if (ppId < 0)
      {
        pairName = determineSymbolicPlayerPairDisplayText(el, 2);
        isSymbolic = true;
      } else {
        PlayerPair pp = pm.getPlayerPair(ppId);
        pairName = getTruncatedPlayerName(pp, xFac - 2 * GapLineTxt_mm, rawReport->getTextStyle(BracketStyle));
      }
      if (yPageBreakSpan > 0)   // does the name of the second pair go on the next page?
      {
        // jump to the continuation page
        rawReport->setActivePage(el.getNextPageNum());

        // draw the remaining part of the bracket element
        int remainingYSpan = spanY - yPageBreakSpan;
        drawBracketTextItem(x0, 0, remainingYSpan, orientation, pairName, BracketTextElement::Pair2,
                            isSymbolic ? BracketStyleItalics : QString());

        // return to our current page
        rawReport->setActivePage(idxPage);
      } else {
        drawBracketTextItem(x0, y0, spanY, orientation, pairName, BracketTextElement::Pair2,
                            isSymbolic ? BracketStyleItalics : QString());
      }

      //
      // Decorate the bracket with match data, if existing
      //

      // is there a match connected to this bracket element?
      auto ma = el.getLinkedMatch();
      if (ma)
      {
        // print match number or result, if any
        ObjState stat = ma->getState();
        int matchNum = ma->getMatchNumber();
        if (stat == ObjState::MA_Finished)
        {
          QString scTxt;
          if (ma->isWonByWalkover())
          {
            scTxt = tr("walkover");
          } else {
            auto score = ma->getScore();
            scTxt = score->toString();
            scTxt.replace(",", "  ");
          }
          drawBracketTextItem(x0, y0, spanY, orientation, scTxt, BracketTextElement::Score);
        }
        else if (matchNum > 0)
        {
          QString s = "#" + QString::number(matchNum);
          drawBracketTextItem(x0, y0, spanY, orientation, s, BracketTextElement::MatchNum);
        }
      }

      // print final rank for winner, if any
      int winRank = -1;
      std::optional<PlayerPair> winnerName{};
      if (ma)
      {
        // case 1: get the winner rank from the direcly linked match
        winRank = ma->getWinnerRank();
        winnerName = ma->getWinner();
      } else {
        // case 2: if there was no match in the last round,
        // derived the winner rank from the NextMatchId but ONLY
        // if we have a directly linked player pair.
        // This case covers that a player was "warped" to the final
        // rank because there is an insufficient number of players
        // to actually play all matches
        auto pp1 = el.getLinkedPlayerPair(1);
        auto pp2 = el.getLinkedPlayerPair(2);
        if (pp1 || pp2)
        {
          int nextWinnerMatch = el.getNextBracketElementForWinner();
          if (nextWinnerMatch < 0) winRank = -nextWinnerMatch;
          winnerName = pp1 ? pp1 : pp2;
        }
      }
      if ((terminator != BracketTerminator::None) && (winRank > 0))
      {
        // add an offset to x0 in case we have inwards offsets
        int tmpX0 = x0;
        if (terminator == BracketTerminator::Inwards)
        {
          if (orientation == BracketOrientation::Left)
          {
            ++x0;
          } else {
            --x0;
          }
        }

        QString txt = QString::number(winRank) + ". " + tr("Place");
        drawBracketTextItem(x0, y0 + termOffset, spanY, orientation, txt, BracketTextElement::WinnerRank);

        // if the match is finished, print the winner's name as well
        if (winnerName)
        {
          QString txt = getTruncatedPlayerName(*winnerName, xFac - 2 * GapLineTxt_mm, rawReport->getTextStyle(BracketStyle));
          drawBracketTextItem(x0, y0 + termOffset, spanY, orientation, txt, BracketTextElement::TerminatorName);
        }

        // restore the original x0
        x0 = tmpX0;
      }
    }
  }

  // decorate all pages with labels, if necessary
  printHeaderAndFooterOnAllPages();

  // done. Reset the internal raw pointer and return the unique_ptr to the caller
  rawReport = nullptr;
  return rep;
}

//----------------------------------------------------------------------------

QStringList BracketSheet::getReportLocators() const
{
  QStringList result;

  QString loc = tr("Brackets::");
  loc += cat.getName();

  result.append(loc);

  return result;
}

//----------------------------------------------------------------------------

/**
 * @brief BracketSheet::determineGridSize calculates the size of one grid unit
 * in millimeters, depending on the extends of bracket
 *
 * @return none. This directly manipulates the private members xFac and yFac
 */
void BracketSheet::determineGridSize()
{
  // determine the maximum extends of the bracket for this category
  // by searching through all bracket visualisation entries
  // get the handle of the overall bracket visualization data
  auto bvd = BracketVisData::getExisting(cat);
  assert(bvd);
  int maxX = -1;
  int maxY = -1;
  for (const BracketVisElement& el : bvd->getVisElements())
  {
    int x = el.getGridX0();
    int y = el.getGridY0();
    int spanY = el.getSpanY();
    BracketOrientation orientation = el.getOrientation();
    BracketTerminator terminator = el.getTerminator();

    if (orientation == BracketOrientation::Right)
    {
      ++x;  // reserve space to the right for the bracket itself
    }

    if (terminator == BracketTerminator::Outwards)
    {
      ++x;  // reserve space for the terminator-line
    }

    // reduce the spanY in case the bracket element spans multiple pages
    if (el.getYPageBreakSpan() > 0)
    {
      spanY = el.getYPageBreakSpan();
    }

    if (x > maxX) maxX = x;
    if ((y + spanY) > maxY) maxY = y + spanY;
  }

  xFac = rawReport->getUsablePageWidth() / maxX;
  yFac = rawReport->getUsablePageHeight() / maxY;
}

//----------------------------------------------------------------------------

void BracketSheet::setupTextStyle()
{
  // check if the text style for bracket text already exists
  SimpleReportLib::TextStyle* baseStyle = rawReport->getTextStyle(BracketStyle);
  if (baseStyle == nullptr)
  {
    baseStyle = rawReport->createChildTextStyle(BracketStyle);
  }

  // set the text size as 40% of the y grid size
  baseStyle->setFontSize_MM(yFac * 0.4);

  // create childs in italics and bold as well
  auto childStyle = rawReport->getTextStyle(BracketStyleItalics);
  if (childStyle == nullptr)
  {
    childStyle = rawReport->createChildTextStyle(BracketStyleItalics, BracketStyle);
  }
  childStyle->setItalicsState(true);
  childStyle = rawReport->getTextStyle(BracketStyleBold);
  if (childStyle == nullptr)
  {
    childStyle = rawReport->createChildTextStyle(BracketStyleBold, BracketStyle);
  }
  childStyle->setBoldState(true);
}

//----------------------------------------------------------------------------

tuple<double, double> BracketSheet::grid2MM(int gridX, int gridY) const
{
  double x = gridX * xFac + DefaultMargin_mm;
  double y = gridY * yFac + DefaultMargin_mm;

  return make_tuple(x, y);
}

//----------------------------------------------------------------------------

/*
 * The following code uses the following points for text elements:
 *
 *   x---------------x p1a
 *   p1              |
 *                   |
 *          x        +-------x--------
 *         p2        |      p3
 *                   |
 *   x---------------x p4a
 *   p4
 */
void BracketSheet::drawBracketTextItem(int bracketX0, int bracketY0, int ySpan, BracketOrientation orientation, QString txt, BracketSheet::BracketTextElement item, const QString& styleNameOverride) const
{
  // get the default text style for bracket text
  auto style = rawReport->getTextStyle(BracketStyle);

  // apply a different text style, if requested by the user
  if (!(styleNameOverride.isEmpty()))
  {
    style = rawReport->getTextStyle(styleNameOverride);
  }

  assert(style != nullptr);  // the style must have been created before!

  // prepare common points for all text elements
  double p1x;
  double p1y;
  tie(p1x, p1y) = grid2MM(bracketX0, bracketY0);
  QPointF p1{p1x, p1y};

  // calculate p4
  QPointF p4 = p1 + QPointF(0, ySpan * yFac);

  // calculate p2
  bool isLeftToRight = (orientation == BracketOrientation::Right);
  double dx = (isLeftToRight) ? xFac / 2.0 : -xFac / 2.0;
  double dy = (ySpan * yFac) / 2.0;
  QPointF p2 = p1 + QPointF(dx, dy);

  // calculate p1a and p4a
  QPointF p1a{p1 + QPointF(2 * dx, 0)};
  QPointF p4a{p4 + QPointF(2 * dx, 0)};

  // calculate p3
  QPointF p3 = p1 + QPointF(3 * dx, dy);

  // prepare an offset-vector of one GAP_LINE_TXT__MM in x- and y-direction
  QPointF gapOffsetX{GapLineTxt_mm, 0};
  QPointF gapOffsetY{0, GapLineTxt_mm};

  // check if we have a multiline text. That indicates a double player name
  bool isMultiline = (txt.split("\n").length() > 1);

  // calc the line space for multiline text
  double lineSpace = 0.1 * style->getFontSize_MM();

  if (item == BracketTextElement::Pair1)
  {
    if (isLeftToRight)
    {
      if (isMultiline)
      {
        // Pair 1, left-to-right, multiline
        rawReport->drawMultilineText(p1 + gapOffsetX, SimpleReportLib::RECT_CORNER::MID_LEFT, txt, SimpleReportLib::LEFT, lineSpace, style);
      } else {
        // Pair 1, left-to-right, single line
        rawReport->drawMultilineText(p1 + gapOffsetX - 0.5 * gapOffsetY, SimpleReportLib::RECT_CORNER::BOTTOM_LEFT, txt, SimpleReportLib::LEFT, lineSpace, style);
      }
    } else {
      if (isMultiline)
      {
        // Pair 1, right-to-left, multiline
        rawReport->drawMultilineText(p1a + gapOffsetX, SimpleReportLib::RECT_CORNER::MID_LEFT, txt, SimpleReportLib::LEFT, lineSpace, style);
      } else {
        // Pair 1, right-to-left, single
        rawReport->drawMultilineText(p1a + gapOffsetX - 0.5 * gapOffsetY, SimpleReportLib::RECT_CORNER::BOTTOM_LEFT, txt, SimpleReportLib::LEFT, lineSpace, style);
      }
    }
    return;
  }

  if (item == BracketTextElement::Pair2)
  {
    if (isLeftToRight)
    {
      if (isMultiline)
      {
        // Pair 2, left-to-right, multiline
        rawReport->drawMultilineText(p4 + gapOffsetX, SimpleReportLib::RECT_CORNER::MID_LEFT, txt, SimpleReportLib::LEFT, lineSpace, style);
      } else {
        // Pair 2, left-to-right, single line
        rawReport->drawMultilineText(p4 + gapOffsetX - 0.5 * gapOffsetY, SimpleReportLib::RECT_CORNER::BOTTOM_LEFT, txt, SimpleReportLib::LEFT, lineSpace, style);
      }
    } else {
      if (isMultiline)
      {
        // Pair 2, right-to-left, multiline
        rawReport->drawMultilineText(p4a + gapOffsetX, SimpleReportLib::RECT_CORNER::MID_LEFT, txt, SimpleReportLib::LEFT, lineSpace, style);
      } else {
        // Pair 2, right-to-left, single
        rawReport->drawMultilineText(p4a + gapOffsetX - 0.5 * gapOffsetY, SimpleReportLib::RECT_CORNER::BOTTOM_LEFT, txt, SimpleReportLib::LEFT, lineSpace, style);
      }
    }
    return;
  }

  if ((item == BracketTextElement::Score) || (item == BracketTextElement::MatchNum))
  {
    rawReport->drawMultilineText(p2, SimpleReportLib::RECT_CORNER::CENTER, txt, SimpleReportLib::CENTER, lineSpace, style);
    return;
  }

  if (item == BracketTextElement::InitialRank1)
  {
    if (isLeftToRight)
    {
      rawReport->drawMultilineText(p1 - gapOffsetX, SimpleReportLib::RECT_CORNER::BOTTOM_RIGHT, txt, SimpleReportLib::CENTER, lineSpace, style);
    } else {
      rawReport->drawMultilineText(p1 + gapOffsetX, SimpleReportLib::RECT_CORNER::BOTTOM_LEFT, txt, SimpleReportLib::CENTER, lineSpace, style);
    }
    return;
  }

  if (item == BracketTextElement::InitialRank2)
  {
    if (isLeftToRight)
    {
      rawReport->drawMultilineText(p4 - gapOffsetX, SimpleReportLib::RECT_CORNER::BOTTOM_RIGHT, txt, SimpleReportLib::CENTER, lineSpace, style);
    } else {
      rawReport->drawMultilineText(p4 + gapOffsetX, SimpleReportLib::RECT_CORNER::BOTTOM_LEFT, txt, SimpleReportLib::CENTER, lineSpace, style);
    }
    return;
  }

  if (item == BracketTextElement::WinnerRank)
  {
    style = rawReport->getTextStyle(BracketStyleBold);
    rawReport->drawMultilineText(p3 + gapOffsetY, SimpleReportLib::RECT_CORNER::TOP_CENTER, txt, SimpleReportLib::CENTER, lineSpace, style);
    return;
  }

  if (item == BracketTextElement::TerminatorName)
  {
    rawReport->drawMultilineText(p3 - gapOffsetY, SimpleReportLib::RECT_CORNER::BOTTOM_CENTER, txt, SimpleReportLib::CENTER, lineSpace, style);
    return;
  }
}

//----------------------------------------------------------------------------

QString BracketSheet::getTruncatedPlayerName(const Player& p, const QString& postfix, double maxWidth, SimpleReportLib::TextStyle* style) const
{
  int fullLen = p.getDisplayName().length();

  // we can't have truncated names of less than six characters
  if (fullLen < 6) return p.getDisplayName() + postfix;

  QString truncName;
  for (int len = fullLen; len > 3; --len)
  {
    truncName = p.getDisplayName(len) + postfix;
    double width = rawReport->getTextDimensions_MM(truncName, style).width();
    if (width <= maxWidth) return truncName;
  }

  return truncName;
}

//----------------------------------------------------------------------------

QString BracketSheet::getTruncatedPlayerName(const PlayerPair& pp, double maxWidth, SimpleReportLib::TextStyle* style) const
{
  if (pp.hasPlayer2())
  {
    QString p1Name = getTruncatedPlayerName(pp.getPlayer1(), " /", maxWidth, style);
    QString p2Name = getTruncatedPlayerName(pp.getPlayer2(), QString(), maxWidth, style);
    return p1Name + "\n" + p2Name;
  }

  return getTruncatedPlayerName(pp.getPlayer1(), QString(), maxWidth, style);
}

//----------------------------------------------------------------------------

void BracketSheet::drawWinnerNameOnTerminator(const QPointF& txtBottomCenter, const PlayerPair& pp, double gridWidth, SimpleReportLib::TextStyle* style) const
{
  QString p1Postfix;
  QString p2Name;
  if (pp.hasPlayer2())
  {
    p1Postfix = " /";
    p2Name = getTruncatedPlayerName(pp.getPlayer2(), QString(), gridWidth - 2 * GapLineTxt_mm, style);
  }
  QString p1Name = getTruncatedPlayerName(pp.getPlayer1(), p1Postfix, gridWidth - 2 * GapLineTxt_mm, style);

  QString fullName = (p2Name.isEmpty()) ? p1Name : p1Name + "\n" + p2Name;
  rawReport->drawMultilineText(txtBottomCenter, SimpleReportLib::RECT_CORNER::BOTTOM_CENTER, fullName, SimpleReportLib::CENTER,
                               0.1 * style->getFontSize_MM(), style);
}

//----------------------------------------------------------------------------

QString BracketSheet::determineSymbolicPlayerPairDisplayText(const BracketVisElement& el, int pos) const
{
  assert(el.getCategoryId() == cat.getId());
  assert((pos > 0) && (pos < 3));

  // is there a symbolic name?
  auto ma = el.getLinkedMatch();
  if (ma)
  {
    int symbName = (pos == 1) ? ma->getSymbolicPlayerPair1Name() : ma->getSymbolicPlayerPair2Name();
    if (symbName < 0)   // process only loser; we don't need to print "Winner of #xxx", because that's indicated by the graph
    {
      QString txt = "(%1 #%2)";
      txt = txt.arg(tr("Loser"));
      txt = txt.arg(-symbName);
      return txt;
    }

    return QString();  // nothing to display for this match
  }

  // default: the branch is unused, so we label it with "--"
  return "--";
}

//----------------------------------------------------------------------------

int BracketSheet::determineEffectivePlayerPairId(const BracketVisElement& el, int pos) const
{
  assert(el.getCategoryId() == cat.getId());
  assert((pos > 0) && (pos < 3));

  // is there a match connected to this bracket element?
  auto ma = el.getLinkedMatch();

  // case 1: there is valid match linked to this bracket element
  if (ma)
  {
    // case 1a: the match has valid player pairs
    // in this case, return their display name
    bool foundPlayerPair = (pos == 1) ? ma->hasPlayerPair1() : ma->hasPlayerPair2();
    if (foundPlayerPair)
    {
      PlayerPair pp = (pos == 1) ? ma->getPlayerPair1() : ma->getPlayerPair2();
      return pp.getPairId();
    }

    return -1;  // nothing found for this match
  }

  // case 2: we have fixed, static player pair references stored for this bracket element
  auto pp = el.getLinkedPlayerPair(pos);
  if (pp)
  {
    return pp->getPairId();
  }

  return -1;   // no player pair at all
}

//----------------------------------------------------------------------------

void BracketSheet::printHeaderAndFooterOnAllPages()
{
  // get the handle of the overall bracket visualization data
  auto bvd = BracketVisData::getExisting(cat);
  if (!bvd) return;

  assert(bvd->getNumPages() == rawReport->getPageCount());

  // prepare the elements of the label: headline, organizer name, date
  QString headline = cat.getName() + " -- " + tr("Bracket");
  QString org = "%1 -- %2";
  org = org.arg(stdString2QString(cfg[CfgKey_TnmtOrga]));
  org = org.arg(stdString2QString(cfg[CfgKey_TnmtName]));
  QString dat = tr("As of %1, %2");
  dat = dat.arg(SimpleReportLib::HeaderFooterStrings::TOKEN_CURDATE);
  dat = dat.arg(SimpleReportLib::HeaderFooterStrings::TOKEN_CURTIME);
  SimpleReportLib::HeaderFooterStrings::substTokensInPlace(dat, -1, -1);

  // prepare the styles for the text elements
  auto headlineStyle = rawReport->getTextStyle(HeadlineStyle);
  assert(headlineStyle != nullptr);
  auto orgStyle = rawReport->getTextStyle(SimpleReportLib::SimpleReportGenerator::DEFAULT_HEADER_STYLE_NAME);
  assert(orgStyle != nullptr);

  // loop over all pages to check if and where to print a label
  for (int pg=0; pg < rawReport->getPageCount(); ++pg)
  {
    BracketPageOrientation orientation;
    BracketLabelPos labelPos;
    tie(orientation, labelPos) = bvd->getPageInfo(pg);

    if (labelPos == BracketLabelPos::None) continue;

    rawReport->setActivePage(pg);

    // determine text alignment and base point
    double x0 = DefaultMargin_mm;
    double y0 = DefaultMargin_mm;
    SimpleReportLib::HOR_TXT_ALIGNMENT align = SimpleReportLib::LEFT;
    if ((labelPos == BracketLabelPos::TopRight) || (labelPos == BracketLabelPos::BottomRight))
    {
      align = SimpleReportLib::RIGHT;
      x0 = rawReport->getPageWidth() - DefaultMargin_mm;
    }
    if ((labelPos == BracketLabelPos::BottomLeft) || (labelPos == BracketLabelPos::BottomRight))
    {
      y0 = rawReport->getPageHeight() - DefaultMargin_mm;

      // subtract the height for three lines of text
      double txtHeight = headlineStyle->getFontSize_MM();  // headline height
      txtHeight += orgStyle->getFontSize_MM();      // orga line height
      txtHeight += rawReport->getTextStyle()->getFontSize_MM();   // date line height

      // add factor for line space
      txtHeight *= 1.1;

      // shift the text's base position up by txtHeight
      y0 -= txtHeight;
    }

    //
    // actually print the text
    //
    // for top-aligned text the sequence is: org, headline, date
    // for bottom aligned text the sequence is: headline, date, org
    //

    if ((labelPos == BracketLabelPos::TopLeft) || (labelPos == BracketLabelPos::TopRight))
    {
      y0 -= orgStyle->getFontSize_MM();

      // org
      QRectF bb = rawReport->drawText(x0, y0, org, orgStyle, align);
      y0 += bb.height() * 2.0;

      // headline
      bb = rawReport->drawText(x0, y0, headline, headlineStyle, align);
      y0 += bb.height() * 1.1;

      // date
      rawReport->drawText(x0, y0, dat, "", align);
    }

    if ((labelPos == BracketLabelPos::BottomLeft) || (labelPos == BracketLabelPos::BottomRight))
    {
      // headline
      QRectF bb = rawReport->drawText(x0, y0, headline, headlineStyle, align);
      y0 += bb.height() * 1.1;

      // date
      bb = rawReport->drawText(x0, y0, dat, "", align);
      y0 += bb.height() * 1.1;

      // org
      rawReport->drawText(x0, y0, org, orgStyle, align);
    }


    // determine text base point

  }
}

//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


}
