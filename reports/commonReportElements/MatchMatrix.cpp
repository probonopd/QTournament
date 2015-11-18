
#include "TableWriter.h"

#include "MatchMatrix.h"
#include "MatchMngr.h"
#include "CatRoundStatus.h"
#include "TournamentDB.h"


MatchMatrix::MatchMatrix(SimpleReportGenerator* _rep, const QString& tabName, const Category& _cat, int _round, int _grpNum)
  :AbstractReportElement(_rep), tableName(tabName), cat(_cat), round(_round), grpNum(_grpNum)
{
  if (round < 1)
  {
    throw invalid_argument("Requested match matrix for invalid round number (too low)");
  }

  MATCH_SYSTEM msys = cat.getMatchSystem();
  if ((msys != ROUND_ROBIN) && (msys != GROUPS_WITH_KO))
  {
    throw invalid_argument("Requested match matrix for invalid category (wrong match system)");
  }

  if (msys == GROUPS_WITH_KO)
  {
    KO_Config cfg{cat.getParameter_string(GROUP_CONFIG)};
    if (round > cfg.getNumRounds())
    {
      throw invalid_argument("Requested match matrix a non-round-robin round!");
    }

    if ((grpNum < 1) || (grpNum > cfg.getNumGroups()))
    {
      throw invalid_argument("Requested match matrix an invalid group number");
    }
  } else {
    if (grpNum != -1)   // grpNum must be -1 for round robin categories
    {
      throw invalid_argument("Requested match matrix an invalid group number");
    }
  }

  CatRoundStatus crs = cat.getRoundStatus();
  if (round > crs.getFinishedRoundsCount())
  {
    throw invalid_argument("Requested match matrix for invalid round number (too high)");
  }
}

//----------------------------------------------------------------------------

QRectF MatchMatrix::plot(const QPointF& topLeft)
{
  QPointF origin = topLeft;

  if ((topLeft.x() < 0) && (topLeft.y() < 0))
  {
    origin = rep->getAbsCursorPos();
  }

  // get the player names for this group
  PlayerPairList ppList;
  ppList = cat.getPlayerPairs(grpNum);

  // get a list of all matches up to and including
  // the requested round
  MatchMngr* mm = Tournament::getMatchMngr();
  MatchList ml;
  MATCH_SYSTEM msys = cat.getMatchSystem();
  for (int r = 1; r <= round; ++r)
  {
    if (msys == GROUPS_WITH_KO)
    {
      ERR err;
      auto mGrp = mm->getMatchGroup(cat, r, grpNum, &err);
      assert(err == OK);
      assert(mGrp != nullptr);

      auto mList = mm->getMatchesForMatchGroup(*mGrp);
      ml.append(mList);
    } else {
      auto mgl = mm->getMatchGroupsForCat(cat, r);
      assert(mgl.length() == 1);
      auto mGrp = mgl.at(0);
      ml.append(mm->getMatchesForMatchGroup(mGrp));
    }
  }

  // get the textstyle for the table contents
  TextStyle* style = rep->getTextStyle();
  assert(style != nullptr);

  // prepare the grid
  int nPlayers = ppList.length();
  MatrixGrid grid{origin, rep->getUsablePageWidth(), nPlayers + 1, 6 * style->getFontSize_MM()};

  // plot the grid
  for (int r=0; r < (nPlayers+1); ++r)
  {
    for (int c=0; c < (nPlayers+1); ++c)
    {
      auto cell = grid.getCell(r, c);

      if ((r == c) && (r != 0))
      {
        rep->drawRect(cell, MED, QColor(Qt::lightGray));
      } else {
        rep->drawRect(cell);
      }
    }
  }

  // plot the grid contents
  for (int r=0; r < (nPlayers+1); ++r)
  {
    for (int c=0; c < (nPlayers+1); ++c)
    {
      auto cell = grid.getCell(r, c);
      QString txt;
      bool hasMatchScore = false;

      if ((r == 0) && (c == 0))
      {
        txt = tableName;
      } else if ((r == 0) || (c == 0)) {
        txt = ppList.at(r + c - 1).getDisplayName();
      } else if ((r > 0) && (c > r)){
        auto ma = getMatchForCell(ppList, r, c);
        if (ma != nullptr)
        {
          PlayerPair ppRow = ppList.at(r - 1);
          PlayerPair ppCol = ppList.at(c - 1);
          QStringList scList = getSortedMatchScoreStrings(*ma, ppRow, ppCol);
          if (scList.isEmpty()) continue;

          for (const QString& l : scList)
          {
            txt += l + ",\n";
          }
          txt.chop(2);
          hasMatchScore = true;
        }
      }

      rep->drawText(cell, RECT_CORNER::CENTER, RECT_CORNER::CENTER, txt, nullptr);

      // draw the "mirrored" match result, if applicable
      //
      // this string-based approach is faster than calling again
      // getMatchForCell() with swapped row/column values
      if (hasMatchScore)
      {
        QString swappedScore;
        for (QString gameScore : txt.split("\n"))
        {
          gameScore = gameScore.trimmed();
          QString sc1 = gameScore.split(" : ").at(0);
          QString sc2 = gameScore.split(" : ").at(1);
          if (sc2.endsWith(",")) sc2.chop(1);

          QString s = "%1 : %2,\n";
          swappedScore += s.arg(sc2).arg(sc1);
        }
        swappedScore.chop(2);
        auto mirroredCell = grid.getCell(c, r);
        rep->drawText(mirroredCell, RECT_CORNER::CENTER, RECT_CORNER::CENTER, swappedScore, nullptr);
      }
    }
  }

  return grid.getGridRect(6);
}

//----------------------------------------------------------------------------

upMatch MatchMatrix::getMatchForCell(const PlayerPairList& ppList, int row, int col) const
{
  if ((row < 1) || (col < 1) || (row > ppList.length()) || (col > ppList.length()))
  {
    return nullptr;
  }

  PlayerPair ppRow = ppList.at(row - 1);
  PlayerPair ppCol = ppList.at(col - 1);

  // create a direct, low-level database query for the
  // applicable matches
  MatchMngr* mm = Tournament::getMatchMngr();
  QString where = "(%1 = %2 AND %3 = %4) OR (%1 = %4 AND %3 = %2)";
  where = where.arg(MA_PAIR1_REF).arg(ppRow.getPairId());
  where = where.arg(MA_PAIR2_REF).arg(ppCol.getPairId());
  DbTab matchTab = Tournament::getDatabaseHandle()->getTab(TAB_MATCH);
  DbTab::CachingRowIterator it = matchTab.getRowsByWhereClause(where);
  while (!(it.isEnd()))
  {
    auto ma = mm->getMatch((*it).getId());
    assert(ma != nullptr);

    // check for right category and the right round number
    if ((ma->getCategory() == cat) && (ma->getMatchGroup().getRound() <= round))
    {
      return ma;
    }

    ++it;
  }

  return nullptr;
}

//----------------------------------------------------------------------------

QStringList MatchMatrix::getSortedMatchScoreStrings(const Match& ma, const PlayerPair& ppRow, const PlayerPair& ppCol) const
{
  PlayerPair pp1 = ma.getPlayerPair1();
  PlayerPair pp2 = ma.getPlayerPair2();
  if (!(  ((pp1 == ppRow) && (pp2 == ppCol)) || ((pp2 == ppRow) && (pp1 == ppCol))  ) )
  {
    return QStringList();
  }

  bool mustSwapScore = (ppRow == pp2);

  auto score = ma.getScore();
  if (score == nullptr) return QStringList();

  QStringList result;
  for (int g=0; g < score->getNumGames(); ++g)
  {
    auto gameScore = score->getGame(g);
    assert(gameScore != nullptr);

    int sc1;
    int sc2;
    tie(sc1, sc2) = gameScore->getScore();

    if (mustSwapScore)
    {
      int tmp = sc1;
      sc1 = sc2;
      sc2 = tmp;
    }

    QString s = "%1 : %2";
    s = s.arg(sc1).arg(sc2);
    result.append(s);
  }

  return result;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

MatrixGrid::MatrixGrid(double _width, int _nCols, double _rowHeight)
  :MatrixGrid(QPointF(0,0), _width, _nCols, _rowHeight)
{

}

//----------------------------------------------------------------------------

MatrixGrid::MatrixGrid(const QPointF& _topLeft, double _width, int _nCols, double _rowHeight)
  :topLeft(_topLeft), width(_width), nCols(_nCols), rowHeight(_rowHeight)
{
  if (width <= 0)
  {
    throw invalid_argument("Can't have negative or zero grid width");
  }

  if (nCols <= 0)
  {
    throw invalid_argument("Can't have negative or zero number of grid columns");
  }

  if (rowHeight < 0)
  {
    throw invalid_argument("Can't have negative or zero grid row height.");
  }

  colWidth = width / nCols;
  cellSize = QSizeF(colWidth, rowHeight);
}

//----------------------------------------------------------------------------

QRectF MatrixGrid::getCell(int row, int col) const
{
  if ((row < 0) || (col < 0)) return QRectF();

  QPointF cellTopLeft = topLeft + QPointF(colWidth * col, rowHeight * row);

  return QRectF(cellTopLeft, cellSize);
}

//----------------------------------------------------------------------------

QSizeF MatrixGrid::getGridSize(int nRows) const
{
  if (nRows < 1) return QSizeF();

  return QSizeF(colWidth * nCols, nRows * rowHeight);
}

//----------------------------------------------------------------------------

QRectF MatrixGrid::getGridRect(int nRows) const
{
  if (nRows < 1) return QRectF();

  return QRectF(topLeft, getGridSize(nRows));
}

//----------------------------------------------------------------------------

QRectF MatrixGrid::getGridRegion(int startRow, int startCol, int rowSpan, int colSpan) const
{
  if ((startRow < 0) || (startCol < 0) || (rowSpan < 1) || (colSpan < 1))
  {
    return QRectF();
  }

  auto startCell = getCell(startRow, startCol);

  return QRectF(startCell.topLeft(), QSize(colSpan * colWidth, rowSpan * rowHeight));
}
