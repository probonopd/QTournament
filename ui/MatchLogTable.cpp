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

#include <QHeaderView>
#include <QResizeEvent>
#include <QScrollBar>
#include <QTableWidgetItem>

#include "MatchLogTable.h"
#include "MatchMngr.h"
#include "CentralSignalEmitter.h"

MatchLogTable::MatchLogTable(QWidget* parent)
  :QTableWidget(parent), db{nullptr}
{
  setDatabase(nullptr);

  // prepare the table layout (columns, headers)
  QStringList horHeaders{tr("#"), tr("Match Info"), tr("Start"), tr("Finish"), tr("Duration"), tr("Court")};
  setColumnCount(horHeaders.length());
  setHorizontalHeaderLabels(horHeaders);
  verticalHeader()->hide();

  // disable the horizontal scrollbar
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  // set selection mode to "row" and "single"
  setSelectionMode(QAbstractItemView::SingleSelection);
  setSelectionBehavior(QAbstractItemView::SelectRows);

  // store the default delegate for later
  defaultDelegate = itemDelegate();

  // react on selection changes
  connect(selectionModel(),
    SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
    SLOT(onSelectionChanged(const QItemSelection&, const QItemSelection&)));

  // connect to match status changes
  connect(CentralSignalEmitter::getInstance(), SIGNAL(matchStatusChanged(int,int,OBJ_STATE,OBJ_STATE)), this,
          SLOT(onMatchStatusChanged(int,int,OBJ_STATE,OBJ_STATE)), Qt::DirectConnection);
}

//----------------------------------------------------------------------------

MatchLogTable::~MatchLogTable()
{
  if (defaultDelegate != nullptr) delete defaultDelegate;
}

//----------------------------------------------------------------------------

void MatchLogTable::setDatabase(TournamentDB* _db)
{
  if (_db == db) return;
  db = _db;

  clearContents();
  setRowCount(0);

  if (db != nullptr)
  {
    // update the delegate
    logItemDelegate = make_unique<MatchLogItemDelegate>(db, this);
    setItemDelegate(logItemDelegate.get());

    // add already finished matches to the table
    fillFromDatabase();

  } else {
    setItemDelegate(defaultDelegate);
  }

  setEnabled(db != nullptr);

  // initialize column widths
  autosizeColumns();
}

//----------------------------------------------------------------------------

void MatchLogTable::onMatchStatusChanged(int maId, int maSeqNum, OBJ_STATE oldStat, OBJ_STATE newStat)
{
  if (newStat != STAT_MA_FINISHED) return;
  if (db == nullptr) return;

  MatchMngr mm{db};
  auto ma = mm.getMatch(maId);
  if (ma != nullptr) prependMatch(*ma);
}

//----------------------------------------------------------------------------

void MatchLogTable::onSelectionChanged(const QItemSelection& selectedItem, const QItemSelection& deselectedItem)
{
  resizeRowsToContents();
  for (auto item : selectedItem)
  {
    logItemDelegate->setSelectedRow(item.top());
    resizeRowToContents(item.top());
  }
  for (auto item : deselectedItem)
  {
    resizeRowToContents(item.top());
  }
}

//----------------------------------------------------------------------------

void MatchLogTable::resizeEvent(QResizeEvent* _event)
{
  // call parent handler
  QTableWidget::resizeEvent(_event);

  // resize all columns
  autosizeColumns();

  // finish event processing
  _event->accept();
}

//----------------------------------------------------------------------------

void MatchLogTable::autosizeColumns()
{
  // distribute the available space according to relative column widths
  int totalUnits = (REL_WIDTH_NUMERIC_COL + // Match number
                    REL_WIDTH_MATCH_INFO + // Match details
                    REL_WIDTH_NUMERIC_COL + // start
                    REL_WIDTH_NUMERIC_COL + // finish
                    REL_WIDTH_NUMERIC_COL + // duration
                    REL_WIDTH_NUMERIC_COL); // court

  int widthAvail = width();
  if ((verticalScrollBar() != nullptr) && (verticalScrollBar()->isVisible()))
  {
    widthAvail -= verticalScrollBar()->width();
  }
  double unitWidth = widthAvail / (1.0 * totalUnits);

  // determine a max width for numeric columns
  int numericColWidth = REL_WIDTH_NUMERIC_COL * unitWidth;
  if (numericColWidth > MAX_NUMERIC_COL_WIDTH) numericColWidth = MAX_NUMERIC_COL_WIDTH;

  int usedWidth = 0;

  // a little lambda that sets the column width and
  // aggregates it in a dedicated local variable
  auto myWidthSetter = [&](int colId, int newColWidth) {
    setColumnWidth(colId, newColWidth);
    usedWidth += newColWidth;
  };

  myWidthSetter(IDX_MATCH_NUM_COL, numericColWidth);
  myWidthSetter(IDX_START_TIME_COL, numericColWidth);
  myWidthSetter(IDX_FINISH_TIME_COL, numericColWidth);
  myWidthSetter(IDX_DURATION_COL, numericColWidth);
  myWidthSetter(IDX_COURT_COL, numericColWidth);

  // assign the remaining width to the match info. This accounts for
  // rounding errors when dividing / multiplying pixel widths and makes
  // that we always used the full width of the widget
  myWidthSetter(IDX_MATCH_INFO_COL, widthAvail - usedWidth);
}

//----------------------------------------------------------------------------

void MatchLogTable::fillFromDatabase()
{
  if (db == nullptr) return;

  MatchMngr mm{db};
  MatchList ml = mm.getFinishedMatches();

  // sort matches by finish time, earliest finisher first
  std::sort(ml.begin(), ml.end(), [](const Match& ma1, const Match& ma2) {
    return ma1.getFinishTime() < ma2.getFinishTime();
  });

  // add them in reverse order: each match is inserted BEFORE the previous one
  for (const Match& ma : ml)
  {
    prependMatch(ma);
  }
}

//----------------------------------------------------------------------------

void MatchLogTable::prependMatch(const Match& ma)
{
  insertRow(0);
  int matchId = ma.getId();

  // a helper lamba for setting the content of a cell
  auto setCellItem = [&](int col, const QString& txt, int userData)
  {
    QTableWidgetItem* newItem = new QTableWidgetItem(txt);
    newItem->setData(Qt::UserRole, userData);
    newItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    setItem(0, col, newItem);
  };

  // add the match number
  setCellItem(IDX_MATCH_NUM_COL, QString::number(ma.getMatchNumber()), matchId);

  // add only empty text for the match info. the content will be displayed
  // by the delegate
  setCellItem(IDX_MATCH_INFO_COL, "", matchId);

  // start and finish time
  QDateTime startTime = ma.getStartTime();
  if (startTime.isValid())
  {
    setCellItem(IDX_START_TIME_COL, ma.getStartTime().toString("HH:mm"), matchId);
    setCellItem(IDX_FINISH_TIME_COL, ma.getFinishTime().toString("HH:mm"), matchId);
  } else {
    // walkover
    setCellItem(IDX_START_TIME_COL, "--", matchId);
    setCellItem(IDX_FINISH_TIME_COL, "--", matchId);
  }

  // the duration
  int duration = ma.getMatchDuration();
  if (duration >= 0)
  {
    int hours = duration / 3600;
    int minutes = (duration % 3600) / 60;
    QString sDuration = "%1:%2";
    sDuration = sDuration.arg(hours).arg(minutes, 2, 10, QLatin1Char('0'));
    setCellItem(IDX_DURATION_COL, sDuration, matchId);
  } else {
    setCellItem(IDX_DURATION_COL, tr("walkover"), matchId);
  }

  // the court
  auto co = ma.getCourt();
  if (co != nullptr)
  {
    QString cn = QString::number(co->getNumber());
    setCellItem(IDX_COURT_COL, cn, matchId);
  } else {
    setCellItem(IDX_COURT_COL, "--", matchId);
  }
}
