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

#include "CommonMatchTableWidget.h"
#include "MatchMngr.h"
#include "CentralSignalEmitter.h"

CommonMatchTableWidget::CommonMatchTableWidget(QWidget* parent)
  :QTableWidget(parent), db{nullptr}
{
  setDatabase(nullptr);

  // prepare the table layout (columns, headers)
  QStringList horHeaders{tr("Number"), tr("Category"), tr("Round"), tr("Group"), tr("Match Info"),
                         tr("Start"), tr("Finish"), tr("Duration"), tr("Court"), tr("Umpire")};
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

}

//----------------------------------------------------------------------------

CommonMatchTableWidget::~CommonMatchTableWidget()
{
  if (defaultDelegate != nullptr) delete defaultDelegate;
}

//----------------------------------------------------------------------------

void CommonMatchTableWidget::setDatabase(TournamentDB* _db)
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

    // call custom initialization function
    // for derived classes
    hook_onTournamentOpened();

    // resize columns and rows to content once (we do not want permanent automatic resizing)
    horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
    verticalHeader()->resizeSections(QHeaderView::ResizeToContents);

  } else {
    setItemDelegate(defaultDelegate);

    // call custom initialization function
    // for derived classes
    hook_onTournamentClosed();
  }

  setEnabled(db != nullptr);

  // initialize column widths
  autosizeColumns();
}

//----------------------------------------------------------------------------

void CommonMatchTableWidget::onSelectionChanged(const QItemSelection& selectedItem, const QItemSelection& deselectedItem)
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

void CommonMatchTableWidget::resizeEvent(QResizeEvent* _event)
{
  // call parent handler
  QTableWidget::resizeEvent(_event);

  // resize all columns
  autosizeColumns();

  // finish event processing
  _event->accept();
}

//----------------------------------------------------------------------------

void CommonMatchTableWidget::autosizeColumns()
{
  // distribute the available space according to relative column widths
  int totalUnits = (REL_WIDTH_NUMERIC_COL + // Match number
                    REL_WIDTH_NUMERIC_COL + // category
                    REL_WIDTH_NUMERIC_COL + // round
                    REL_WIDTH_NUMERIC_COL + // group
                    REL_WIDTH_MATCH_INFO + // Match details
                    REL_WIDTH_NUMERIC_COL + // start
                    REL_WIDTH_NUMERIC_COL + // finish
                    REL_WIDTH_NUMERIC_COL + // duration
                    REL_WIDTH_NUMERIC_COL + // court
                    REL_WIDTH_UMPIRE_COL);  // umpire

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
  myWidthSetter(IDX_CAT_COL, numericColWidth);
  myWidthSetter(IDX_ROUND_COL, numericColWidth);
  myWidthSetter(IDX_GRP_COL, numericColWidth);
  myWidthSetter(IDX_START_TIME_COL, numericColWidth);
  myWidthSetter(IDX_FINISH_TIME_COL, numericColWidth);
  myWidthSetter(IDX_DURATION_COL, numericColWidth);
  myWidthSetter(IDX_COURT_COL, numericColWidth);
  myWidthSetter(IDX_UMPIRE_COL, REL_WIDTH_UMPIRE_COL * unitWidth);

  // assign the remaining width to the match info. This accounts for
  // rounding errors when dividing / multiplying pixel widths and makes
  // that we always used the full width of the widget
  myWidthSetter(IDX_MATCH_INFO_COL, widthAvail - usedWidth);
}

//----------------------------------------------------------------------------

void CommonMatchTableWidget::insertMatch(int beforeRowIdx, const Match& ma)
{
  insertRow(beforeRowIdx);
  int matchId = ma.getId();

  // a helper lamba for setting the content of a cell
  auto setCellItem = [&](int col, const QString& txt, int userData)
  {
    QTableWidgetItem* newItem = new QTableWidgetItem(txt);
    newItem->setData(Qt::UserRole, userData);
    newItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    setItem(beforeRowIdx, col, newItem);
  };

  // add the match number
  int maNum = ma.getMatchNumber();
  if (maNum != MATCH_NUM_NOT_ASSIGNED)
  {
    setCellItem(IDX_MATCH_NUM_COL, QString::number(maNum), matchId);
  } else {
    setCellItem(IDX_MATCH_NUM_COL, "--", matchId);
  }


  // add category and round
  setCellItem(IDX_CAT_COL, ma.getCategory().getName(), matchId);
  auto grp = ma.getMatchGroup();
  setCellItem(IDX_ROUND_COL, QString::number(grp.getRound()), matchId);

  // add the group number, if any
  int grpNum = grp.getGroupNumber();
  if (grpNum > 0)
  {
    setCellItem(IDX_GRP_COL, QString::number(grpNum), matchId);
  } else {
    setCellItem(IDX_GRP_COL, "--", matchId);
  }

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
    if (ma.isWonByWalkover())
    {
      setCellItem(IDX_DURATION_COL, tr("walkover"), matchId);
    } else {
      setCellItem(IDX_DURATION_COL, "--", matchId);
    }
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

  // the umpire
  auto ump = ma.getAssignedReferee();
  if (ump != nullptr)
  {
    setCellItem(IDX_UMPIRE_COL, ump->getDisplayName_FirstNameFirst(), matchId);
  } else {
    setCellItem(IDX_UMPIRE_COL, "--", matchId);
  }
}

//----------------------------------------------------------------------------

void CommonMatchTableWidget::appendMatch(const Match& ma)
{
  insertMatch(rowCount(), ma);
}

//----------------------------------------------------------------------------
