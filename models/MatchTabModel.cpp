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

MatchTableModel::MatchTableModel(const TournamentDB& _db)
  :QAbstractTableModel{nullptr}, db{_db}, matchTab{DbTab{db, TabMatch, false}}
{
  CentralSignalEmitter* cse = CentralSignalEmitter::getInstance();
  connect(cse, SIGNAL(beginCreateMatch()), this, SLOT(onBeginCreateMatch()), Qt::DirectConnection);
  connect(cse, SIGNAL(endCreateMatch(int)), this, SLOT(onEndCreateMatch(int)), Qt::DirectConnection);
  connect(cse, SIGNAL(matchStatusChanged(int,int,ObjState,ObjState)), this, SLOT(onMatchStatusChanged(int,int,ObjState,ObjState)), Qt::DirectConnection);
  connect(cse, SIGNAL(beginResetAllModels()), this, SLOT(onBeginResetModel()), Qt::DirectConnection);
  connect(cse, SIGNAL(endResetAllModels()), this, SLOT(onEndResetModel()), Qt::DirectConnection);
  connect(cse, SIGNAL(endCreateCourt(int)), this, SLOT(recalcPrediction()), Qt::DirectConnection);
  connect(cse, SIGNAL(endDeleteCourt()), this, SLOT(recalcPrediction()), Qt::DirectConnection);
  connect(cse, SIGNAL(courtStatusChanged(int,int,ObjState,ObjState)), this, SLOT(recalcPrediction()), Qt::DirectConnection);

  // create and initialize a new match time predictor
  matchTimePredictor = std::make_unique<MatchTimePredictor>(db);
}

//----------------------------------------------------------------------------

int MatchTableModel::rowCount(const QModelIndex& parent) const
{
  if (parent.isValid()) return 0;
  return matchTab.length();
}

//----------------------------------------------------------------------------

int MatchTableModel::columnCount(const QModelIndex& parent) const
{
  if (parent.isValid()) return 0;
  return ColumnCount;
}

//----------------------------------------------------------------------------

QVariant MatchTableModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
      //return QVariant();
      return QString("Invalid index");

    if (index.row() >= matchTab.length())
      //return QVariant();
      return QString("Invalid row: " + QString::number(index.row()));

    if (role != Qt::DisplayRole)
      return QVariant();
    
    MatchMngr mm{db};
    auto ma = mm.getMatchBySeqNum(index.row());
    auto mg = ma->getMatchGroup();
    
    // first column: match num
    if (index.column() == MatchNumColId)
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
      if (c.getMatchSystem() == MatchSystem::Ranking) return "--";

      // in all other cases, try to print a group number
      return GuiHelpers::groupNumToString(mg.getGroupNumber());
    }

    // sixth column: the match state; this column is used for filtering and
    // needs to be hidden in the view
    if (index.column() == StateColId)
    {
      return static_cast<int>(ma->getState());
    }

    // seventh column: the referee mode for the match
    if (index.column() == RefereeModeColId)
    {
      RefereeMode mode = ma->get_EFFECTIVE_RefereeMode();

      // if there is already a referee assigned, display
      // the referee name
      if ((mode == RefereeMode::AllPlayers) ||
          (mode == RefereeMode::RecentFinishers) ||
          (mode == RefereeMode::SpecialTeam))
      {
        auto referee = ma->getAssignedReferee();
        if (referee)
        {
          return referee->getDisplayName();
        }
      }

      // in all other cases, display the referee selection mode
      switch (mode)
      {
      case RefereeMode::None:
        return tr("None");

      case RefereeMode::HandWritten:
        return tr("Manual");

      case RefereeMode::AllPlayers:
        return tr("Pick from all players");

      case RefereeMode::RecentFinishers:
        return tr("Pick from finishers");

      case RefereeMode::SpecialTeam:
        return tr("Pick from team");
      }

      return tr("unknown");
    }

    // for all following columns, we need the
    // estimated start/finish time for the match
    MatchTimePrediction mtp = matchTimePredictor->getPredictionForMatch(*ma);

    // the estimated start time
    if (index.column() == EstStartColId)
    {
      time_t t = mtp.estStartTime__UTC;
      if (t == 0) return "??";
      QDateTime start = QDateTime::fromTime_t(t);
      return start.toString("HH:mm");
    }

    // the estimated finish time
    if (index.column() == EstEndColId)
    {
      time_t t = mtp.estFinishTime__UTC;
      if (t == 0) return "??";
      QDateTime start = QDateTime::fromTime_t(t);
      return start.toString("HH:mm");
    }

    // the estimated court
    if (index.column() == EstCourtColId)
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
    if (section == MatchNumColId) {
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
    if (section == StateColId) {
      return tr("State");
    }
    if (section == RefereeModeColId) {
      return tr("Umpire");
    }
    if (section == EstStartColId) {
      return tr("Start");
    }
    if (section == EstEndColId) {
      return tr("Finish");
    }
    if (section == EstCourtColId) {
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
  int newPos = matchTab.length();
  beginInsertRows(QModelIndex(), newPos, newPos);
}
//----------------------------------------------------------------------------

void MatchTableModel::onEndCreateMatch(int newMatchSeqNum)
{
  endInsertRows();
  //recalcPrediction();   // matches are created as INCOMPLETE and do not affect the schedule
}

//----------------------------------------------------------------------------

void MatchTableModel::onMatchStatusChanged(int matchId, int matchSeqNum, ObjState fromState, ObjState toState)
{
  std::cerr << "MatchTabModel: onMatchStatusChanged!" << std::endl;

  QModelIndex startIdx = createIndex(matchSeqNum, 0);
  QModelIndex endIdx = createIndex(matchSeqNum, ColumnCount-1);
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
  QModelIndex startIdx = createIndex(0, EstStartColId);
  QModelIndex endIdx = createIndex(rowCount(), EstCourtColId);
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


