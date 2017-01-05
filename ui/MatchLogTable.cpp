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
  :CommonMatchTableWidget{parent}
{
  connect(CentralSignalEmitter::getInstance(), SIGNAL(matchStatusChanged(int,int,OBJ_STATE,OBJ_STATE)),
          this, SLOT(onMatchStatusChanged(int,int,OBJ_STATE,OBJ_STATE)), Qt::DirectConnection);
}

//----------------------------------------------------------------------------

void MatchLogTable::onMatchStatusChanged(int maId, int maSeqNum, OBJ_STATE oldStat, OBJ_STATE newStat)
{
  if (newStat != STAT_MA_FINISHED) return;
  if (db == nullptr) return;

  MatchMngr mm{db};
  auto ma = mm.getMatch(maId);
  if (ma != nullptr) prependMatch(*ma);
  resizeRowToContents(0);
}

//----------------------------------------------------------------------------

void MatchLogTable::hook_onTournamentOpened()
{
  fillFromDatabase();
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

  // the umpire
  auto ump = ma.getAssignedReferee();
  if (ump != nullptr)
  {
    setCellItem(IDX_UMPIRE_COL, ump->getDisplayName_FirstNameFirst(), matchId);
  } else {
    setCellItem(IDX_UMPIRE_COL, "--", matchId);
  }
}
