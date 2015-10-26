#include <tuple>

#include <QList>

#include "BracketSheet.h"
#include "SimpleReportGenerator.h"

#include "ui/GuiHelpers.h"
#include "Match.h"
#include "MatchMngr.h"
#include "BracketGenerator.h"

namespace QTournament
{

  constexpr char BracketSheet::BRACKET_STYLE[];


BracketSheet::BracketSheet(TournamentDB* _db, const QString& _name, const Category& _cat)
  :AbstractReport(_db, _name), cat(_cat), tabVis(_db->getTab(TAB_BRACKET_VIS)), rawReport(nullptr)
{
  // make sure the requested category has bracket visualization data
  if (tabVis.getMatchCountForColumnValue(BV_CAT_REF, cat.getId()) == 0)
  {
    throw std::runtime_error("Requested bracket report for a category without bracket visualization data!");
  }
}

//----------------------------------------------------------------------------

upSimpleReport BracketSheet::regenerateReport()
{
  upSimpleReport rep = createEmptyReport_Landscape();
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

  // loop over all bracket elements and draw them one by one
  DbTab::CachingRowIterator it = tabVis.getRowsByColumnValue(BV_CAT_REF, cat.getId());
  while (!(it.isEnd()))
  {
    TabRow r = *it;
    int x0 = r[BV_GRID_X0].toInt();
    int y0 = r[BV_GRID_Y0].toInt();
    int spanY = r[BV_SPAN_Y].toInt();
    int orientation = r[BV_ORIENTATION].toInt();
    int terminator = r[BV_TERMINATOR].toInt();

    int xLen = -1;
    if (orientation == BracketMatchData::VIS_ORIENTATION_RIGHT)
    {
      xLen = 1;
    }

    // draw the "open rectangle"
    drawHorLine(x0, y0, xLen);
    drawHorLine(x0, y0 + spanY, xLen);
    drawVertLine(x0 + xLen, y0, spanY);

    // draw the terminator, if any
    if (terminator == BracketMatchData::VIS_TERMINATOR_OUTWARDS)
    {
      drawHorLine(x0 + xLen, y0 + spanY / 2, xLen);
    }
    if (terminator == BracketMatchData::VIS_TERMINATOR_INWARDS)
    {
      drawHorLine(x0 + xLen, y0 + spanY / 2, -xLen);
    }

    // draw the initial rank, if any
    int iniRank = r[BV_INITIAL_RANK1].toInt();
    if (iniRank > 0)
    {
      drawBracketTextItem(x0, y0, spanY, orientation,
                          QString::number(iniRank) + ".",
                          BRACKET_TEXT_ELEMENT::INITIAL_RANK1);
    }
    iniRank = r[BV_INITIAL_RANK2].toInt();
    if (iniRank > 0)
    {
      drawBracketTextItem(x0, y0, spanY, orientation,
                          QString::number(iniRank) + ".",
                          BRACKET_TEXT_ELEMENT::INITIAL_RANK2);
    }

    //
    // Decorate the bracket with match data, if existing
    //

    // is there a match connected to this bracket element?
    QVariant _matchId = r[BV_MATCH_REF];
    if (_matchId.isNull()) {
      ++it;
      continue;
    }
    auto ma = Tournament::getMatchMngr()->getMatch(_matchId.toInt());

    // print the names of the first player pair
    if (ma->hasPlayerPair1())
    {
      PlayerPair pp = ma->getPlayerPair1();
      drawBracketTextItem(x0, y0, spanY, orientation, pp.getDisplayName(), BRACKET_TEXT_ELEMENT::PAIR1);
    }

    // print the names of the second player pair
    if (ma->hasPlayerPair2())
    {
      PlayerPair pp = ma->getPlayerPair2();
      drawBracketTextItem(x0, y0, spanY, orientation, pp.getDisplayName(), BRACKET_TEXT_ELEMENT::PAIR2);
    }

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
      QString s = QString::number(matchNum);
      drawBracketTextItem(x0, y0, spanY, orientation, s, BRACKET_TEXT_ELEMENT::MATCH_NUM);
    }

    ++it;
  }

  rawReport == nullptr;
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
  int catId = cat.getId();
  int maxX = -1;
  int maxY = -1;
  DbTab::CachingRowIterator it = tabVis.getRowsByColumnValue(BV_CAT_REF, catId);
  while (!(it.isEnd()))
  {
    TabRow r = *it;
    int x = r[BV_GRID_X0].toInt();
    int y = r[BV_GRID_Y0].toInt();
    int spanY = r[BV_SPAN_Y].toInt();
    int orientation = r[BV_ORIENTATION].toInt();
    int terminator = r[BV_TERMINATOR].toInt();

    if (orientation == BracketMatchData::VIS_ORIENTATION_RIGHT)
    {
      ++x;  // reserve space to the right for the bracket itself
    }

    if (terminator == BracketMatchData::VIS_TERMINATOR_OUTWARDS)
    {
      ++x;  // reserve space for the terminator-line
    }

    if (x > maxX) maxX = x;
    if ((y + spanY) > maxY) maxY = y + spanY;

    ++it;
  }

  xFac = rawReport->getUsablePageWidth() / maxX;
  yFac = rawReport->getUsablePageHeight() / maxY;
}

//----------------------------------------------------------------------------

void BracketSheet::setupTextStyle()
{
  // check if the text style for bracket text already exists
  auto style = rawReport->getTextStyle(BRACKET_STYLE);
  if (style == nullptr)
  {
    style = rawReport->createChildTextStyle(BRACKET_STYLE);
  }

  // set the text size as 40% of the y grid size
  style->setFontSize_MM(yFac * 0.4);
}

//----------------------------------------------------------------------------

tuple<double, double> BracketSheet::grid2MM(int gridX, int gridY) const
{
  double x = gridX * xFac + DEFAULT_MARGIN__MM;
  double y = gridY * yFac + DEFAULT_MARGIN__MM;

  return make_tuple(x, y);
}

//----------------------------------------------------------------------------

void BracketSheet::drawBracketTextItem(int bracketX0, int bracketY0, int ySpan, int orientation, QString txt, BracketSheet::BRACKET_TEXT_ELEMENT item)
{
  // get the text style for bracket text
  auto style = rawReport->getTextStyle(BRACKET_STYLE);
  assert(style != nullptr);  // the style must have been created before!
  double txtHeight = style->getFontSize_MM();

  // prepare common values for all text elements
  double x0;
  double y0;
  SimpleReportLib::HOR_TXT_ALIGNMENT align = SimpleReportLib::LEFT;

  // adjust the top-left corner of the bracket, if necessary
  if (orientation == BracketMatchData::VIS_ORIENTATION_LEFT)
  {
    --bracketX0;
  }

  // pre-calculate the y-positions of text elements
  double yTextTop;
  tie(x0, yTextTop) = grid2MM(bracketX0, bracketY0);
  yTextTop -= txtHeight * 1.1 + GAP_LINE_TXT__MM;
  double yTextBottom = yTextTop + yFac * ySpan;
  double yTextCenter = (yTextTop + yTextBottom) / 2.0;

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
    if (orientation == BracketMatchData::VIS_ORIENTATION_LEFT)
    {
      x0 += xFac;
    }

    // add a little gap between the line and the text. Depending
    // on the bracket orientation we have to add or subtract
    // from the x0-value
    x0 += (orientation == BracketMatchData::VIS_ORIENTATION_LEFT) ? GAP_LINE_TXT__MM : -GAP_LINE_TXT__MM;

    y0 = yTextTop;
    align = SimpleReportLib::RIGHT;
  }
  if (item == BRACKET_TEXT_ELEMENT::INITIAL_RANK2)
  {
    if (orientation == BracketMatchData::VIS_ORIENTATION_LEFT)
    {
      x0 += xFac;
    }

    x0 += (orientation == BracketMatchData::VIS_ORIENTATION_LEFT) ? GAP_LINE_TXT__MM : -GAP_LINE_TXT__MM;

    y0 = yTextBottom;
    align = SimpleReportLib::RIGHT;
  }

  // actually draw the text
  rawReport->drawText(x0, y0, txt, style, align);
}

//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


}
