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

    // draw names of the first player pair
    if (ma->hasPlayerPair1())
    {
      PlayerPair pp = ma->getPlayerPair1();
      drawBracketTextItem(x0, y0, spanY, orientation, pp.getDisplayName(), BRACKET_TEXT_ELEMENT::PAIR1);
    }

    // draw names of the second player pair
    if (ma->hasPlayerPair2())
    {
      PlayerPair pp = ma->getPlayerPair2();
      drawBracketTextItem(x0, y0, spanY, orientation, pp.getDisplayName(), BRACKET_TEXT_ELEMENT::PAIR2);
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

tuple<double, double> BracketSheet::grid2MM(int gridX, int gridY) const
{
  double x = gridX * xFac + DEFAULT_MARGIN__MM;
  double y = gridY * yFac + DEFAULT_MARGIN__MM;

  return make_tuple(x, y);
}

//----------------------------------------------------------------------------

void BracketSheet::drawBracketTextItem(int bracketX0, int bracketY0, int ySpan, int orientation, QString txt, BracketSheet::BRACKET_TEXT_ELEMENT item)
{
  double x0;
  double y0;

  if (item == BRACKET_TEXT_ELEMENT::PAIR1)
  {
    if (orientation == BracketMatchData::VIS_ORIENTATION_LEFT)
    {
      --bracketX0;
    }

    tie(x0, y0) = grid2MM(bracketX0, bracketY0);

    // shift the text above the line
    y0 -= 2 + GAP_LINE_TXT__MM;   // FIX ME: "2" is the default font height in mm

    // separate the text a bit from other lines or elements on the left
    x0 += GAP_LINE_TXT__MM;
  }

  if (item == BRACKET_TEXT_ELEMENT::PAIR2)
  {
    if (orientation == BracketMatchData::VIS_ORIENTATION_LEFT)
    {
      --bracketX0;
    }

    tie(x0, y0) = grid2MM(bracketX0, bracketY0 + ySpan);

    // shift the text above the line
    y0 -= 2 + GAP_LINE_TXT__MM;   // FIX ME: "2" is the default font height in mm
    x0 += GAP_LINE_TXT__MM;
  }

  rawReport->drawText(x0, y0, txt);
}

//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


}
