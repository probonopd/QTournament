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

#include "MatchGroupTabModel.h"

#include "Category.h"
#include "MatchMngr.h"
#include "../ui/GuiHelpers.h"
#include "CentralSignalEmitter.h"

#include <QDebug>

using namespace QTournament;
using namespace SqliteOverlay;

MatchGroupTableModel::MatchGroupTableModel(const TournamentDB& _db)
  :QAbstractTableModel{nullptr}, db{_db}, mgTab{DbTab{db, TabMatchGroup, false}}
{
  CentralSignalEmitter* cse = CentralSignalEmitter::getInstance();
  connect(cse, SIGNAL(beginCreateMatchGroup()), this, SLOT(onBeginCreateMatchGroup()), Qt::DirectConnection);
  connect(cse, SIGNAL(endCreateMatchGroup(int)), this, SLOT(onEndCreateMatchGroup(int)), Qt::DirectConnection);
  connect(cse, SIGNAL(matchGroupStatusChanged(int,int,ObjState,ObjState)), this, SLOT(onMatchGroupStatusChanged(int,int)), Qt::DirectConnection);
  connect(cse, SIGNAL(beginResetAllModels()), this, SLOT(onBeginResetModel()), Qt::DirectConnection);
  connect(cse, SIGNAL(endResetAllModels()), this, SLOT(onEndResetModel()), Qt::DirectConnection);
}

//----------------------------------------------------------------------------

int MatchGroupTableModel::rowCount(const QModelIndex& parent) const
{
  if (parent.isValid()) return 0;
  return mgTab.length();
}

//----------------------------------------------------------------------------

int MatchGroupTableModel::columnCount(const QModelIndex& parent) const
{
  if (parent.isValid()) return 0;
  return ColumnCount;
}

//----------------------------------------------------------------------------

QVariant MatchGroupTableModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
      //return QVariant();
      return QString("Invalid index");

    if (index.row() >= mgTab.length())
      //return QVariant();
      return QString("Invalid row: " + QString::number(index.row()));

    // center content
    if (role == Qt::TextAlignmentRole)
    {
      return Qt::AlignCenter;
    }

    // skip all other requests except for the actual content
    if (role != Qt::DisplayRole)
      return QVariant();
    
    MatchMngr mm{db};
    auto mg = mm.getMatchGroupBySeqNum(index.row());
    
    // first column: category
    if (index.column() == 0)
    {
      Category c = mg->getCategory();
      return c.getName();
    }

    // second column: round
    if (index.column() == 1)
    {
      return mg->getRound();
    }

    // third column: group number
    if (index.column() == 2)
    {
      return GuiHelpers::groupNumToString(mg->getGroupNumber());
    }

    // fourth column: number of matches in this match group
    if (index.column() == 3)
    {
      return mg->getMatchCount();
    }

    // fifth column: current status
    // Used for filtering only and needs to be hidden in the associated view
    if (index.column() == StateColId)
    {
      ObjState stat = mg->getState();
      return static_cast<int>(stat);
    }

    // sixth column: stage sequence number
    // Used for sorting in the "Staged Match Groups View" only and needs to be hidden in the associated view
    if (index.column() == StageSeqColId)
    {
      return mg->getStageSequenceNumber();
    }

    return QString("Not Implemented, row=" + QString::number(index.row()) + ", col=" + QString::number(index.row()));
}

//----------------------------------------------------------------------------

QVariant MatchGroupTableModel::headerData(int section, Qt::Orientation orientation, int role) const
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
    if (section == 0) {
      return tr("Category");
    }
    if (section == 1) {
      return tr("Round");
    }
    if (section == 2) {
      return tr("Group");
    }
    if (section == 3) {
      return tr("Matches");
    }
    if (section == StateColId) {
      return tr("State");
    }
    if (section == StageSeqColId) {
      return tr("StageSeqNum");
    }

    return QString("Not implemented, section=" + QString::number(section));
  }
  
  return QString::number(section + 1);
}

//----------------------------------------------------------------------------

void MatchGroupTableModel::onBeginCreateMatchGroup()
{
  int newPos = mgTab.length();
  beginInsertRows(QModelIndex(), newPos, newPos);
}
//----------------------------------------------------------------------------

void MatchGroupTableModel::onEndCreateMatchGroup(int newMatchGroupSeqNum)
{
  endInsertRows();

  // For some weird reason, the filter is not updated when
  // we do a (mass-)insertion of match groups. Thus, we manually
  // ask the associated views to update their filters
  //
  // This is only necessary for the MatchGroupTableModel, not for the MatchTableModel.
  // Weird...
  emit triggerFilterUpdate();
}

//----------------------------------------------------------------------------

void MatchGroupTableModel::onMatchGroupStatusChanged(int matchGroupId, int matchGroupSeqNum)
{
  QModelIndex startIdx = createIndex(matchGroupSeqNum, 0);
  QModelIndex endIdx = createIndex(matchGroupSeqNum, ColumnCount-1);
  emit dataChanged(startIdx, endIdx);
}

//----------------------------------------------------------------------------

void MatchGroupTableModel::onBeginResetModel()
{
  beginResetModel();
}

//----------------------------------------------------------------------------

void MatchGroupTableModel::onEndResetModel()
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


