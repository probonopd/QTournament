#include <tuple>

#include <QList>

#include "BracketSheet.h"
#include "SimpleReportGenerator.h"
#include "TextStyle.h"

#include "ui/GuiHelpers.h"
#include "Match.h"
#include "MatchMngr.h"
#include "BracketGenerator.h"

namespace QTournament
{

  constexpr char BracketSheet::BRACKET_STYLE[];
  constexpr char BracketSheet::BRACKET_STYLE_ITALICS[];
  constexpr char BracketSheet::BRACKET_STYLE_BOLD[];


BracketSheet::BracketSheet(TournamentDB* _db, const QString& _name, const Category& _cat)
  :AbstractReport(_db, _name), cat(_cat), rawReport(nullptr)
{
  // make sure the requested category has bracket visualization data
  auto bvd = BracketVisData::getExisting(cat);
  if (bvd == nullptr)
  {
    throw std::runtime_error("Requested bracket report for a category without bracket visualization data!");
  }
}

//----------------------------------------------------------------------------

upSimpleReport BracketSheet::regenerateReport()
{
  // get the handle of the overall bracket visualization data
  auto bvd = BracketVisData::getExisting(cat);
  if (bvd == nullptr)
  {
    auto result = createEmptyReport_Portrait();
    setHeaderAndHeadline(result.get(), "Tournament Bracket");
    result->writeLine(tr("No visualization data available for this category."));
    return result;
  }

  // update/fill potentially missing names in the bracket
  bvd->fillMissingPlayerNames();


  BRACKET_PAGE_ORIENTATION pgOrientation;
  BRACKET_LABEL_POS labelPos;

  // get the orientation of the first page
  //
  // FIX: as of today, all pages in a report must have the same orientation!
  // Even if the orientation information in the database changes from page to page,
  // the report will still use the same orientation as for the first page.
  tie(pgOrientation, labelPos) = bvd->getPageInfo(0);

  // initialize the report's first page
  upSimpleReport rep = (pgOrientation == BRACKET_PAGE_ORIENTATION::LANDSCAPE) ? createEmptyReport_Landscape() : createEmptyReport_Portrait();
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
      BRACKET_ORIENTATION orientation = el.getOrientation();
      BRACKET_TERMINATOR terminator = el.getTerminator();

      int xLen = -1;
      if (orientation == BRACKET_ORIENTATION::RIGHT)
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
      if (terminator == BRACKET_TERMINATOR::OUTWARDS)
      {
        drawHorLine(x0 + xLen, y0 + spanY / 2 + termOffset, xLen);
      }
      if (terminator == BRACKET_TERMINATOR::INWARDS)
      {
        drawHorLine(x0 + xLen, y0 + spanY / 2 + termOffset, -xLen);
      }

      // draw the initial rank, if any
      int iniRank = el.getInitialRank1();
      if (iniRank > 0)
      {
        drawBracketTextItem(x0, y0, spanY, orientation,
                            QString::number(iniRank) + ".",
                            BRACKET_TEXT_ELEMENT::INITIAL_RANK1);
      }
      iniRank = el.getInitialRank2();
      if (iniRank > 0)
      {
        drawBracketTextItem(x0, y0, spanY, orientation,
                            QString::number(iniRank) + ".",
                            BRACKET_TEXT_ELEMENT::INITIAL_RANK2);
      }

      //
      // print the actual or symbolic player names, if any
      //
      int ppId = determineEffectivePlayerPairId(el, 1);
      if (ppId < 0)
      {
        QString symbName = determineSymbolicPlayerPairDisplayText(el, 1);
        drawBracketTextItem(x0, y0, spanY, orientation, symbName, BRACKET_TEXT_ELEMENT::PAIR1,
                            BRACKET_STYLE_ITALICS);
      } else {
        PlayerPair pp = Tournament::getPlayerMngr()->getPlayerPair(ppId);
        drawTruncatedPlayerNameOnBracketLine(x0, y0, orientation, pp);
      }
      ppId = determineEffectivePlayerPairId(el, 2);
      if (ppId < 0)
      {
        QString symbName = determineSymbolicPlayerPairDisplayText(el, 2);
        drawBracketTextItem(x0, y0, spanY, orientation, symbName, BRACKET_TEXT_ELEMENT::PAIR2,
                            BRACKET_STYLE_ITALICS);
      } else {
        PlayerPair pp = Tournament::getPlayerMngr()->getPlayerPair(ppId);
        drawTruncatedPlayerNameOnBracketLine(x0, y0 + spanY, orientation, pp);
      }

      //
      // Decorate the bracket with match data, if existing
      //

      // is there a match connected to this bracket element?
      auto ma = el.getLinkedMatch();
      if (ma != nullptr)
      {
        // print match number or result, if any
        OBJ_STATE stat = ma->getState();
        int matchNum = ma->getMatchNumber();
        if (stat == STAT_MA_FINISHED)
        {
          auto score = ma->getScore();
          drawBracketTextItem(x0, y0, spanY, orientation, score->toString(), BRACKET_TEXT_ELEMENT::SCORE);
        }
        else if (matchNum > 0)
        {
          QString s = "#" + QString::number(matchNum);
          drawBracketTextItem(x0, y0, spanY, orientation, s, BRACKET_TEXT_ELEMENT::MATCH_NUM);
        }

        // print final rank for winner, if any
        int winRank = ma->getWinnerRank();
        if ((terminator != BRACKET_TERMINATOR::NONE) && (winRank > 0))
        {
          // add an offset to x0 in case we have inwards offsets
          int tmpX0 = x0;
          if (terminator == BRACKET_TERMINATOR::INWARDS)
          {
            if (orientation == BRACKET_ORIENTATION::LEFT)
            {
              ++x0;
            } else {
              --x0;
            }
          }

          QString txt = QString::number(winRank) + ". " + tr("Place");
          drawBracketTextItem(x0, y0 + termOffset, spanY, orientation, txt, BRACKET_TEXT_ELEMENT::WINNER_RANK);

          // if the match is finished, print the winner's name as well
          if (stat == STAT_MA_FINISHED)
          {
            drawBracketTextItem(x0, y0 + termOffset, spanY, orientation, ma->getWinner()->getDisplayName(), BRACKET_TEXT_ELEMENT::TERMINATOR_NAME);
          }

          // restore the original x0
          x0 = tmpX0;
        }
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
  assert(bvd != nullptr);
  int maxX = -1;
  int maxY = -1;
  for (const BracketVisElement& el : bvd->getVisElements())
  {
    int x = el.getGridX0();
    int y = el.getGridY0();
    int spanY = el.getSpanY();
    BRACKET_ORIENTATION orientation = el.getOrientation();
    BRACKET_TERMINATOR terminator = el.getTerminator();

    if (orientation == BRACKET_ORIENTATION::RIGHT)
    {
      ++x;  // reserve space to the right for the bracket itself
    }

    if (terminator == BRACKET_TERMINATOR::OUTWARDS)
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
  SimpleReportLib::TextStyle* baseStyle = rawReport->getTextStyle(BRACKET_STYLE);
  if (baseStyle == nullptr)
  {
    baseStyle = rawReport->createChildTextStyle(BRACKET_STYLE);
  }

  // set the text size as 40% of the y grid size
  baseStyle->setFontSize_MM(yFac * 0.4);

  // create childs in italics and bold as well
  auto childStyle = rawReport->getTextStyle(BRACKET_STYLE_ITALICS);
  if (childStyle == nullptr)
  {
    childStyle = rawReport->createChildTextStyle(BRACKET_STYLE_ITALICS, BRACKET_STYLE);
  }
  childStyle->setItalicsState(true);
  childStyle = rawReport->getTextStyle(BRACKET_STYLE_BOLD);
  if (childStyle == nullptr)
  {
    childStyle = rawReport->createChildTextStyle(BRACKET_STYLE_BOLD, BRACKET_STYLE);
  }
  childStyle->setBoldState(true);
}

//----------------------------------------------------------------------------

tuple<double, double> BracketSheet::grid2MM(int gridX, int gridY) const
{
  double x = gridX * xFac + DEFAULT_MARGIN__MM;
  double y = gridY * yFac + DEFAULT_MARGIN__MM;

  return make_tuple(x, y);
}

//----------------------------------------------------------------------------

void BracketSheet::drawBracketTextItem(int bracketX0, int bracketY0, int ySpan, BRACKET_ORIENTATION orientation, QString txt, BracketSheet::BRACKET_TEXT_ELEMENT item, const QString& styleNameOverride) const
{
  // get the default text style for bracket text
  auto style = rawReport->getTextStyle(BRACKET_STYLE);

  // apply a different text style, if requested by the user
  if (!(styleNameOverride.isEmpty()))
  {
    style = rawReport->getTextStyle(styleNameOverride);
  }

  assert(style != nullptr);  // the style must have been created before!
  double txtHeight = style->getFontSize_MM();

  // prepare common values for all text elements
  double x0;
  double y0;
  SimpleReportLib::HOR_TXT_ALIGNMENT align = SimpleReportLib::LEFT;

  // adjust the top-left corner of the bracket, if necessary
  if (orientation == BRACKET_ORIENTATION::LEFT)
  {
    --bracketX0;
  }

  // pre-calculate the y-positions of text elements
  tie(x0, y0) = grid2MM(bracketX0, bracketY0);
  double yTextTop = y0;
  yTextTop -= txtHeight * 1.1 + GAP_LINE_TXT__MM;
  double yTextBottom = yTextTop + yFac * ySpan;
  double yTextCenter = GAP_LINE_TXT__MM + (yTextTop + yTextBottom + txtHeight) / 2.0;

  //
  // adjust x0,y0 depending on the text item
  //

  if (item == BRACKET_TEXT_ELEMENT::PAIR1)
  {
    y0 = yTextTop;

    // separate the text a bit from other lines or elements on the left
    x0 += GAP_LINE_TXT__MM;
  }

  if (item == BRACKET_TEXT_ELEMENT::PAIR2)
  {
    y0 = yTextBottom;

    // separate the text a bit from other lines or elements on the left
    x0 += GAP_LINE_TXT__MM;
  }

  if ((item == BRACKET_TEXT_ELEMENT::SCORE) || (item == BRACKET_TEXT_ELEMENT::MATCH_NUM))
  {
    x0 += xFac / 2.0;  // center horizontaly
    y0 = yTextCenter;
    align = SimpleReportLib::CENTER;
  }

  if (item == BRACKET_TEXT_ELEMENT::INITIAL_RANK1)
  {
    // undo the x0-adjustment, because the initial rank is always
    // at the "open end" of the bracket, and the "open end" is the
    // original x0,y0-pair
    if (orientation == BRACKET_ORIENTATION::LEFT)
    {
      x0 += xFac;
    }

    // add a little gap between the line and the text. Depending
    // on the bracket orientation we have to add or subtract
    // from the x0-value
    x0 += (orientation == BRACKET_ORIENTATION::LEFT) ? GAP_LINE_TXT__MM : -GAP_LINE_TXT__MM;

    y0 = yTextTop;
    align = SimpleReportLib::RIGHT;
  }
  if (item == BRACKET_TEXT_ELEMENT::INITIAL_RANK2)
  {
    if (orientation == BRACKET_ORIENTATION::LEFT)
    {
      x0 += xFac;
    }

    x0 += (orientation == BRACKET_ORIENTATION::LEFT) ? GAP_LINE_TXT__MM : -GAP_LINE_TXT__MM;

    y0 = yTextBottom;
    align = SimpleReportLib::RIGHT;
  }

  if ((item == BRACKET_TEXT_ELEMENT::WINNER_RANK) || (item == BRACKET_TEXT_ELEMENT::TERMINATOR_NAME))
  {
    // default: calculate position for winner rank
    x0 += (orientation == BRACKET_ORIENTATION::RIGHT) ? xFac : -xFac;
    y0 = yTextCenter + txtHeight + GAP_LINE_TXT__MM;

    // move up by text height plus 2 x gap if winner name
    if (item == BRACKET_TEXT_ELEMENT::TERMINATOR_NAME)
    {
      y0 -= txtHeight + 2 * GAP_LINE_TXT__MM;
    }

    // ranks are printed in bold text
    if ((item == BRACKET_TEXT_ELEMENT::WINNER_RANK) && (styleNameOverride.isEmpty()))
    {
      style = rawReport->getTextStyle(BRACKET_STYLE_BOLD);
    }

    x0 += xFac / 2.0;
    align = SimpleReportLib::CENTER;
  }

  // actually draw the text
  rawReport->drawText(x0, y0, txt, style, align);
}

//----------------------------------------------------------------------------

void BracketSheet::drawTruncatedPlayerNameOnBracketLine(int bracketLineX0, int bracketLineY0, BRACKET_ORIENTATION orientation, const PlayerPair& pp) const
{
  // get the default text style for bracket text
  auto style = rawReport->getTextStyle(BRACKET_STYLE);
  double txtHeight = style->getFontSize_MM();

  // adjust the top-left corner of the bracket, if necessary
  if (orientation == BRACKET_ORIENTATION::LEFT)
  {
    --bracketLineX0;
  }

  // calc the base point of the bracket line
  double lineX0;
  double lineY0;
  tie(lineX0, lineY0) = grid2MM(bracketLineX0, bracketLineY0);

  // prepare a possible postfix for player1's name in
  // case we are in doubles or mixed
  QString p1Postfix = (pp.hasPlayer2()) ? " / " : QString();

  // a little helper function that truncates a player name
  // until it fits on a bracket line
  auto truncPlayerName = [&](const Player& _p, const QString& postfix) {
    int fullLen = _p.getDisplayName().length();
    QString truncName;
    double maxWidth = xFac - 2 * GAP_LINE_TXT__MM;
    for (int len = fullLen; len > 3; --len)
    {
      truncName = _p.getDisplayName(len) + postfix;
      double width = rawReport->getTextDimensions_MM(truncName, style).width();
      if (width <= maxWidth) break;    // xFac equals the line length
    }

    return truncName;
  };

  // truncate player1's name until it fits on
  // a bracket line and actually draw it
  QString p1TruncName = truncPlayerName(pp.getPlayer1(), p1Postfix);
  double textY0 = lineY0 - txtHeight * 1.1 - GAP_LINE_TXT__MM;
  rawReport->drawText(lineX0 + GAP_LINE_TXT__MM, textY0, p1TruncName, style);

  // truncate player2's name until it fits on
  // a bracket line and actually draw it
  if (pp.hasPlayer2())
  {
    QString p2TruncName = truncPlayerName(pp.getPlayer2(), QString());
    textY0 = lineY0 + txtHeight * 0.1;
    rawReport->drawText(lineX0 + GAP_LINE_TXT__MM, textY0, p2TruncName, style);
  }

}

//----------------------------------------------------------------------------

QString BracketSheet::determineSymbolicPlayerPairDisplayText(const BracketVisElement& el, int pos) const
{
  assert(el.getCategoryId() == cat.getId());
  assert((pos > 0) && (pos < 3));

  // is there a symbolic name?
  auto ma = el.getLinkedMatch();
  if (ma != nullptr)
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
  if (ma != nullptr)
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
  if (pp != nullptr)
  {
    return pp->getPairId();
  }

  return -1;   // no player pair at all
}

//----------------------------------------------------------------------------

void BracketSheet::printHeaderAndFooterOnAllPages() const
{
  // get the handle of the overall bracket visualization data
  auto bvd = BracketVisData::getExisting(cat);
  if (bvd == nullptr) return;

  assert(bvd->getNumPages() == rawReport->getPageCount());

  // prepare the elements of the label: headline, organizer name, date
  QString headline = cat.getName() + " -- " + tr("Bracket");
  QString org = cfg[CFG_KEY_TNMT_ORGA].toString() + " -- " + cfg[CFG_KEY_TNMT_NAME].toString();
  QString dat = tr("As of %1, %2");
  dat = dat.arg(SimpleReportLib::HeaderFooterStrings::TOKEN_CURDATE);
  dat = dat.arg(SimpleReportLib::HeaderFooterStrings::TOKEN_CURTIME);
  SimpleReportLib::HeaderFooterStrings::substTokensInPlace(dat, -1, -1);

  // prepare the styles for the text elements
  auto headlineStyle = rawReport->getTextStyle(HEADLINE_STYLE);
  assert(headlineStyle != nullptr);
  auto orgStyle = rawReport->getTextStyle(SimpleReportLib::SimpleReportGenerator::DEFAULT_HEADER_STYLE_NAME);
  assert(orgStyle != nullptr);

  // loop over all pages to check if and where to print a label
  for (int pg=0; pg < rawReport->getPageCount(); ++pg)
  {
    BRACKET_PAGE_ORIENTATION orientation;
    BRACKET_LABEL_POS labelPos;
    tie(orientation, labelPos) = bvd->getPageInfo(pg);

    if (labelPos == BRACKET_LABEL_POS::NONE) continue;

    rawReport->setActivePage(pg);

    // determine text alignment and base point
    double x0 = DEFAULT_MARGIN__MM;
    double y0 = DEFAULT_MARGIN__MM;
    SimpleReportLib::HOR_TXT_ALIGNMENT align = SimpleReportLib::LEFT;
    if ((labelPos == BRACKET_LABEL_POS::TOP_RIGHT) || (labelPos == BRACKET_LABEL_POS::BOTTOM_RIGHT))
    {
      align = SimpleReportLib::RIGHT;
      x0 = rawReport->getPageWidth() - DEFAULT_MARGIN__MM;
    }
    if ((labelPos == BRACKET_LABEL_POS::BOTTOM_LEFT) || (labelPos == BRACKET_LABEL_POS::BOTTOM_RIGHT))
    {
      y0 = rawReport->getPageHeight() - DEFAULT_MARGIN__MM;

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

    if ((labelPos == BRACKET_LABEL_POS::TOP_LEFT) || (labelPos == BRACKET_LABEL_POS::TOP_RIGHT))
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

    if ((labelPos == BRACKET_LABEL_POS::BOTTOM_LEFT) || (labelPos == BRACKET_LABEL_POS::BOTTOM_RIGHT))
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
