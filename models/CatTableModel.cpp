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


#include "CatTableModel.h"

#include "Category.h"
#include "CatRoundStatus.h"
#include "CentralSignalEmitter.h"
#include "CatMngr.h"

using namespace QTournament;
using namespace SqliteOverlay;

CategoryTableModel::CategoryTableModel(const TournamentDB& _db)
  :QAbstractTableModel{nullptr}, db{_db}, catTab{DbTab{db, TabCategory, false}}
{
  CentralSignalEmitter* cse = CentralSignalEmitter::getInstance();
  connect(cse, SIGNAL(beginCreateCategory()), this, SLOT(onBeginCreateCategory()), Qt::DirectConnection);
  connect(cse, SIGNAL(endCreateCategory(int)), this, SLOT(onEndCreateCategory(int)), Qt::DirectConnection);
  connect(cse, SIGNAL(beginDeleteCategory(int)), this, SLOT(onBeginDeleteCategory(int)), Qt::DirectConnection);
  connect(cse, SIGNAL(endDeleteCategory()), this, SLOT(onEndDeleteCategory()), Qt::DirectConnection);
  connect(cse, SIGNAL(beginResetAllModels()), this, SLOT(onBeginResetModel()), Qt::DirectConnection);
  connect(cse, SIGNAL(endResetAllModels()), this, SLOT(onEndResetModel()), Qt::DirectConnection);
}

//----------------------------------------------------------------------------

int CategoryTableModel::rowCount(const QModelIndex& parent) const
{
  return catTab.length();
}

//----------------------------------------------------------------------------

int CategoryTableModel::columnCount(const QModelIndex& parent) const
{
  return ColumnCount;
}

//----------------------------------------------------------------------------

QVariant CategoryTableModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
      return QVariant();

    if (index.row() >= catTab.length())
      return QVariant();

    if (role != Qt::DisplayRole)
      return QVariant();
    
    CatMngr cm{db};
    Category c = cm.getCategoryBySeqNum(index.row());

    const int roundOffset = c.getParameter_int(CatParameter::FirstRoundOffset);
    
    // name
    if (index.column() == ColName)
    {
      return c.getName();
    }

    // number of finished rounds
    if (index.column() == ColFinishedRounds)
    {
      CatRoundStatus crs = c.getRoundStatus();
      int finishedRounds = crs.getFinishedRoundsCount();
      if (finishedRounds == CatRoundStatus::NoRoundsFinishedYet)
      {
        return "--";
      }

      return finishedRounds;
    }

    // number of currently played round
    if (index.column() == ColCurrentRound)
    {
      CatRoundStatus crs = c.getRoundStatus();
      int currentRoundNum = crs.getCurrentlyRunningRoundNumber();

      if (currentRoundNum == CatRoundStatus::NoCurrentlyRunningRounds)
      {
        return "--";
      }

      if (currentRoundNum == CatRoundStatus::MultipleRoundsRunning)
      {
        // generate a comma-separated list of currently running rounds
        QList<int> runningRounds = crs.getCurrentlyRunningRoundNumbers();
        QString result = "";
        for (int round : runningRounds)
        {
          result += QString::number(round + roundOffset) + ", ";
        }
        return result.left(result.length() - 2);
      }

      return currentRoundNum + roundOffset;
    }

    // total number of matches in the current round
    if (index.column() == ColTotalMatches)
    {
      CatRoundStatus crs = c.getRoundStatus();
      auto matchStat = crs.getMatchCountForCurrentRound();
      int total = std::get<0>(matchStat);
      if (total == CatRoundStatus::NoCurrentlyRunningRounds)
      {
        return "--";
      }

      return total;
    }

    // number of unfinished matches in the current round
    if (index.column() == ColUnfinishedMatches)
    {
      CatRoundStatus crs = c.getRoundStatus();
      auto matchStat = crs.getMatchCountForCurrentRound();
      int unfinished = std::get<1>(matchStat);
      if (unfinished == CatRoundStatus::NoCurrentlyRunningRounds)
      {
        return "--";
      }

      return unfinished;
    }

    // number of running matches in the current round
    if (index.column() == ColRunningMatches)
    {
      CatRoundStatus crs = c.getRoundStatus();
      auto matchStat = crs.getMatchCountForCurrentRound();
      int runningMatches = std::get<2>(matchStat);
      if (runningMatches == CatRoundStatus::NoCurrentlyRunningRounds)
      {
        return "--";
      }

      return runningMatches;
    }

    // number of waiting matches in the current round
    if (index.column() == ColWaitingMatches)
    {
      CatRoundStatus crs = c.getRoundStatus();
      auto matchStat = crs.getMatchCountForCurrentRound();

      // Waiting = unfinished - running
      int waitingMatches = std::get<1>(matchStat) - std::get<2>(matchStat);

      if (waitingMatches <= 0)
      {
        return "--";
      }

      return waitingMatches;
    }

    // total number of rounds
    if (index.column() == ColTotalRounds)
    {
      auto specialCat = c.convertToSpecializedObject();
      int result = specialCat->calcTotalRoundsCount();
      if (result > 0) return result;
      return "--";
    }

    return QString("Not Implemented");
}

//----------------------------------------------------------------------------

QVariant CategoryTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (role != Qt::DisplayRole)
  {
    return QVariant();
  }
  
  if (orientation == Qt::Horizontal)
  {
    if (section == ColName) {
      return tr("Name");
    }
    
    if (section == ColFinishedRounds) {
      return tr("Finished rounds");
    }

    if (section == ColCurrentRound) {
      return tr("Current round");
    }

    if (section == ColTotalMatches) {
      return tr("Total matches\nin current round");
    }

    if (section == ColUnfinishedMatches) {
      return tr("Unfinished matches\nin current round");
    }

    if (section == ColRunningMatches) {
      return tr("Running matches\nin current round");
    }

    if (section == ColWaitingMatches) {
      return tr("Waiting matches\nin current round");
    }

    if (section == ColTotalRounds) {
      return tr("Total rounds");
    }

    return QString("Not implemented");
  }
  
  return QString::number(section + 1);
}

//----------------------------------------------------------------------------

void CategoryTableModel::onBeginCreateCategory()
{
  int newPos = catTab.length();
  beginInsertRows(QModelIndex(), newPos, newPos);
}
//----------------------------------------------------------------------------

void CategoryTableModel::onEndCreateCategory(int newCatSeqNum)
{
  endInsertRows();
}

//----------------------------------------------------------------------------

void CategoryTableModel::onBeginDeleteCategory(int catSeqNum)
{
  beginRemoveRows(QModelIndex(), catSeqNum, catSeqNum);
}

//----------------------------------------------------------------------------

void CategoryTableModel::onEndDeleteCategory()
{
  endRemoveRows();
}

//----------------------------------------------------------------------------

void CategoryTableModel::onBeginResetModel()
{
  beginResetModel();
}

//----------------------------------------------------------------------------

void CategoryTableModel::onEndResetModel()
{
  endResetModel();
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


