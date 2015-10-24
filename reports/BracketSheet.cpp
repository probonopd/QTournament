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
  :AbstractReport(_db, _name), cat(_cat), tabVis(_db->getTab(TAB_BRACKET_VIS))
{
  // make sure the requested category has bracket visualization data
  if (tabVis.getMatchCountForColumnValue(BV_CAT_REF, cat.getId()) == 0)
  {
    throw std::runtime_error("Requested bracket report for a category without bracket visualization data!");
  }
}

//----------------------------------------------------------------------------

upSimpleReport BracketSheet::regenerateReport() const
{
  upSimpleReport result = createEmptyReport_Landscape();

  setHeaderAndHeadline(result.get(), "Bracket");

  // determine the conversion factor between "grid units" and "paper units" (millimeter)
  double xFac;
  double yFac;
  tie(xFac, yFac) = determineGridSize(result);

  // a helper lambda to convert between grid units and millimeters
  auto grid2MM = [&](int gridX, int gridY) {
    double x = gridX * xFac + DEFAULT_MARGIN__MM;
    double y = gridY * yFac + DEFAULT_MARGIN__MM;
    return make_tuple(x, y);
  };

  // two helper-lambdas for drawing horizontal and vertical lines
  // using grid units instead of millimeters
  auto drawHorLine = [&](int gridX0, int gridY0, int gridXLen) {
    double x0;
    double y0;
    tie(x0, y0) = grid2MM(gridX0, gridY0);
    result->drawHorLine(x0, y0, gridXLen * xFac);
  };
  auto drawVertLine = [&](int gridX0, int gridY0, int gridYLen) {
    double x0;
    double y0;
    tie(x0, y0) = grid2MM(gridX0, gridY0);
    result->drawVertLine(x0, y0, gridYLen * yFac);
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

    ++it;
  }

  return result;
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
 * @return a tupple of <xWidth, yWidth> containing the size of one grid unit in millimeters
 */
tuple<double, double> BracketSheet::determineGridSize(const upSimpleReport& rep) const
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

  double xGridUnit = rep->getUsablePageWidth() / maxX;
  double yGridUnit = rep->getUsablePageHeight() / maxY;

  return make_tuple(xGridUnit, yGridUnit);
}

//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


}
