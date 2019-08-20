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

using namespace QTournament;

CommonMatchTableWidget::CommonMatchTableWidget(QWidget* parent)
  :GuiHelpers::AutoSizingTableWidget_WithDatabase{GuiHelpers::AutosizeColumnDescrList{
{tr("Number"), RelNumericColWidth, -1, MaxNumericColWidth},
{tr("Category"), RelNumericColWidth, -1, MaxNumericColWidth},
{tr("Round"), RelNumericColWidth, -1, MaxNumericColWidth},
{tr("Group"), RelNumericColWidth, -1, MaxNumericColWidth},
{tr("Match Info"), RelMatchInfoColWidth, -1, -1},
{tr("Start"), RelNumericColWidth, -1, MaxNumericColWidth},
{tr("Finish"), RelNumericColWidth, -1, MaxNumericColWidth},
{tr("Duration"), RelNumericColWidth, -1, MaxNumericColWidth},
{tr("Court"), RelNumericColWidth, -1, MaxNumericColWidth},
{tr("Umpire"), RelUmpireColWidth, -1, -1}
     }}
{
  setRubberBandCol(IdxMatchInfoCol);

  // react on selection changes
  connect(selectionModel(),
    SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
    SLOT(onSelectionChanged(const QItemSelection&, const QItemSelection&)));

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

void CommonMatchTableWidget::hook_onDatabaseOpened()
{
  // call the parent
  AutoSizingTableWidget_WithDatabase::hook_onDatabaseOpened();

  // update the delegate
  logItemDelegate = new MatchLogItemDelegate(*db, this);
  setCustomDelegate(logItemDelegate);  // the base class takes ownership of the pointer
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
  if (maNum != MatchNumNotAssigned)
  {
    setCellItem(IdxMatchNumCol, QString::number(maNum), matchId);
  } else {
    setCellItem(IdxMatchNumCol, "--", matchId);
  }


  // add category and round
  setCellItem(IdxConfigCol, ma.getCategory().getName(), matchId);
  auto grp = ma.getMatchGroup();
  setCellItem(IdxRoundCol, QString::number(grp.getRound()), matchId);

  // add the group number, if any
  int grpNum = grp.getGroupNumber();
  if (grpNum > 0)
  {
    setCellItem(IdxGrpCol, QString::number(grpNum), matchId);
  } else {
    setCellItem(IdxGrpCol, "--", matchId);
  }

  // add only empty text for the match info. the content will be displayed
  // by the delegate
  setCellItem(IdxMatchInfoCol, "", matchId);

  // start and finish time
  QDateTime startTime = ma.getStartTime();
  if (startTime.isValid())
  {
    setCellItem(IdxStartTimeCol, ma.getStartTime().toString("HH:mm"), matchId);
    setCellItem(IdxFinishTimeCol, ma.getFinishTime().toString("HH:mm"), matchId);
  } else {
    // walkover
    setCellItem(IdxStartTimeCol, "--", matchId);
    setCellItem(IdxFinishTimeCol, "--", matchId);
  }

  // the duration
  int duration = ma.getMatchDuration();
  if (duration >= 0)
  {
    int hours = duration / 3600;
    int minutes = (duration % 3600) / 60;
    QString sDuration = "%1:%2";
    sDuration = sDuration.arg(hours).arg(minutes, 2, 10, QLatin1Char('0'));
    setCellItem(IdxDurationCol, sDuration, matchId);
  } else {
    if (ma.isWonByWalkover())
    {
      setCellItem(IdxDurationCol, tr("walkover"), matchId);
    } else {
      setCellItem(IdxDurationCol, "--", matchId);
    }
  }

  // the court
  auto co = ma.getCourt(nullptr);
  if (co)
  {
    QString cn = QString::number(co->getNumber());
    setCellItem(IdxCourtCol, cn, matchId);
  } else {
    setCellItem(IdxCourtCol, "--", matchId);
  }

  // the umpire
  auto ump = ma.getAssignedReferee();
  if (ump)
  {
    setCellItem(IdxUmpireCol, ump->getDisplayName_FirstNameFirst(), matchId);
  } else {
    setCellItem(IdxUmpireCol, "--", matchId);
  }

  resizeRowToContents(beforeRowIdx);
}

//----------------------------------------------------------------------------

void CommonMatchTableWidget::appendMatch(const Match& ma)
{
  insertMatch(rowCount(), ma);
}

//----------------------------------------------------------------------------
