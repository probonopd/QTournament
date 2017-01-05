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
  if (ma != nullptr) insertMatch(0, *ma);
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
    insertMatch(0, ma);
  }
}

//----------------------------------------------------------------------------

