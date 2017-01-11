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

#include <QDebug>

#include "MatchTabModel.h"
#include "Category.h"
#include "../ui/GuiHelpers.h"
#include "CentralSignalEmitter.h"
#include "MatchMngr.h"
#include <SqliteOverlay/KeyValueTab.h>

using namespace QTournament;
using namespace SqliteOverlay;

MatchTableModel::MatchTableModel(TournamentDB* _db)
:QAbstractTableModel(0), db(_db), matchTab((db->getTab(TAB_MATCH))), matchTimePredictor(nullptr)
{
  CentralSignalEmitter* cse = CentralSignalEmitter::getInstance();
  connect(cse, SIGNAL(beginCreateMatch()), this, SLOT(onBeginCreateMatch()), Qt::DirectConnection);
  connect(cse, SIGNAL(endCreateMatch(int)), this, SLOT(onEndCreateMatch(int)), Qt::DirectConnection);
  connect(cse, SIGNAL(matchStatusChanged(int,int,OBJ_STATE,OBJ_STATE)), this, SLOT(onMatchStatusChanged(int,int,OBJ_STATE,OBJ_STATE)), Qt::DirectConnection);
  connect(cse, SIGNAL(beginResetAllModels()), this, SLOT(onBeginResetModel()), Qt::DirectConnection);
  connect(cse, SIGNAL(endResetAllModels()), this, SLOT(onEndResetModel()), Qt::DirectConnection);
  connect(cse, SIGNAL(endCreateCourt(int)), this, SLOT(recalcPrediction()), Qt::DirectConnection);
  connect(cse, SIGNAL(endDeleteCourt()), this, SLOT(recalcPrediction()), Qt::DirectConnection);
  connect(cse, SIGNAL(courtStatusChanged(int,int,OBJ_STATE,OBJ_STATE)), this, SLOT(recalcPrediction()), Qt::DirectConnection);

  // create and initialize a new match time predictor
  matchTimePredictor = make_unique<MatchTimePredictor>(db);
}

//----------------------------------------------------------------------------

int MatchTableModel::rowCount(const QModelIndex& parent) const
{
  if (parent.isValid()) return 0;
  return matchTab->length();
}

//----------------------------------------------------------------------------

int MatchTableModel::columnCount(const QModelIndex& parent) const
{
  if (parent.isValid()) return 0;
  return COLUMN_COUNT;
}

//----------------------------------------------------------------------------

QVariant MatchTableModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
      //return QVariant();
      return QString("Invalid index");

    if (index.row() >= matchTab->length())
      //return QVariant();
      return QString("Invalid row: " + QString::number(index.row()));

    if (role != Qt::DisplayRole)
      return QVariant();
    
    MatchMngr mm{db};
    auto ma = mm.getMatchBySeqNum(index.row());
    auto mg = ma->getMatchGroup();
    
    // first column: match num
    if (index.column() == MATCH_NUM_COL_ID)
    {
      return ma->getMatchNumber();
    }

    // second column: match name
    if (index.column() == 1)
    {
      return ma->getDisplayName(tr("Winner"), tr("Loser"));
    }

    // third column: category name
    if (index.column() == 2)
    {
      Category c = mg.getCategory();
      return c.getName();
    }

    // fourth column: round
    if (index.column() == 3)
    {
      return mg.getRound();
    }

    // fifth column: players group, if applicable
    if (index.column() == 4)
    {
      // if this is a match that has a winner rank assigned,
      // we abuse this column to print the target rank
      int winnerRank = ma->getWinnerRank();
      if (winnerRank > 0)
      {
        QString txt = tr("Pl. %1");
        txt = txt.arg(winnerRank);
        return txt;
      }

      // if we have a ranking bracket, labels like "QF", "SF"
      // or "FI" do not really make sense. So we display
      // nothing instead
      Category c = mg.getCategory();
      if (c.getMatchSystem() == RANKING) return "--";

      // in all other cases, try to print a group number
      return GuiHelpers::groupNumToString(mg.getGroupNumber());
    }

    // sixth column: the match state; this column is used for filtering and
    // needs to be hidden in the view
    if (index.column() == STATE_COL_ID)
    {
      return static_cast<int>(ma->getState());
    }

    // seventh column: the referee mode for the match
    if (index.column() == REFEREE_MODE_COL_ID)
    {
      REFEREE_MODE mode = ma->get_EFFECTIVE_RefereeMode();

      // if there is already a referee assigned, display
      // the referee name
      if ((mode == REFEREE_MODE::ALL_PLAYERS) ||
          (mode == REFEREE_MODE::RECENT_FINISHERS) ||
          (mode == REFEREE_MODE::SPECIAL_TEAM))
      {
        upPlayer referee = ma->getAssignedReferee();
        if (referee != nullptr)
        {
          return referee->getDisplayName();
        }
      }

      // in all other cases, display the referee selection mode
      switch (mode)
      {
      case REFEREE_MODE::NONE:
        return tr("None");

      case REFEREE_MODE::HANDWRITTEN:
        return tr("Manual");

      case REFEREE_MODE::ALL_PLAYERS:
        return tr("Pick from all players");

      case REFEREE_MODE::RECENT_FINISHERS:
        return tr("Pick from finishers");

      case REFEREE_MODE::SPECIAL_TEAM:
        return tr("Pick from team");
      }

      return tr("unknown");
    }

    // for all following columns, we need the
    // estimated start/finish time for the match
    MatchTimePrediction mtp = matchTimePredictor->getPredictionForMatch(*ma);

    // the estimated start time
    if (index.column() == EST_START_COL_ID)
    {
      time_t t = mtp.estStartTime__UTC;
      if (t == 0) return "??";
      QDateTime start = QDateTime::fromTime_t(t);
      return start.toString("HH:mm");
    }

    // the estimated finish time
    if (index.column() == EST_END_COL_ID)
    {
      time_t t = mtp.estFinishTime__UTC;
      if (t == 0) return "??";
      QDateTime start = QDateTime::fromTime_t(t);
      return start.toString("HH:mm");
    }

    // the estimated court
    if (index.column() == EST_COURT_COL_ID)
    {
      if (mtp.estCourtNum < 1) return "??";
      return mtp.estCourtNum;
    }

    return QString("Not Implemented, row=" + QString::number(index.row()) + ", col=" + QString::number(index.row()));
}

//----------------------------------------------------------------------------

QVariant MatchTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (role != Qt::DisplayRole)
  {
    return QVariant();
  }

  if (section < 0)
  {
    return QVariant();
  }
  
  if (orientation == Qt::Horizontal)
  {
    if (section == MATCH_NUM_COL_ID) {
      return tr("Number");
    }
    if (section == 1) {
      return tr("Match");
    }
    if (section == 2) {
      return tr("Category");
    }
    if (section == 3) {
      return tr("Round");
    }
    if (section == 4) {
      return tr("Group");
    }
    if (section == STATE_COL_ID) {
      return tr("State");
    }
    if (section == REFEREE_MODE_COL_ID) {
      return tr("Umpire");
    }
    if (section == EST_START_COL_ID) {
      return tr("Start");
    }
    if (section == EST_END_COL_ID) {
      return tr("Finish");
    }
    if (section == EST_COURT_COL_ID) {
      return tr("Court");
    }

    return QString("Not implemented, section=" + QString::number(section));
  }
  
  return QString::number(section + 1);
}

//----------------------------------------------------------------------------

QModelIndex MatchTableModel::getIndex(int row, int col)
{
  return createIndex(row, col);
}

//----------------------------------------------------------------------------

void MatchTableModel::onBeginCreateMatch()
{
  int newPos = matchTab->length();
  beginInsertRows(QModelIndex(), newPos, newPos);
}
//----------------------------------------------------------------------------

void MatchTableModel::onEndCreateMatch(int newMatchSeqNum)
{
  endInsertRows();
  //recalcPrediction();   // matches are created as INCOMPLETE and do not affect the schedule
}

//----------------------------------------------------------------------------

void MatchTableModel::onMatchStatusChanged(int matchId, int matchSeqNum, OBJ_STATE fromState, OBJ_STATE toState)
{
  QModelIndex startIdx = createIndex(matchSeqNum, 0);
  QModelIndex endIdx = createIndex(matchSeqNum, COLUMN_COUNT-1);
  emit dataChanged(startIdx, endIdx);

  // no need for recalculation match times here:
  //
  // started / finished matches are captured via the court status change
  // and trigger an immediate update
  //
  // all other changes (e.g. adding matches to the schedule) will be
  // captured by a periodic update every 10 seconds

  return;
}

//----------------------------------------------------------------------------

void MatchTableModel::onBeginResetModel()
{
  beginResetModel();
}

//----------------------------------------------------------------------------

void MatchTableModel::onEndResetModel()
{
  matchTimePredictor->resetPrediction();
  recalcPrediction();
  endResetModel();
}

//----------------------------------------------------------------------------

void MatchTableModel::recalcPrediction()
{
  matchTimePredictor->updatePrediction();
  QModelIndex startIdx = createIndex(0, EST_START_COL_ID);
  QModelIndex endIdx = createIndex(rowCount(), EST_COURT_COL_ID);
  emit dataChanged(startIdx, endIdx);
}

//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


