/* 
 * File:   PlayerTableModel.cpp
 * Author: volker
 * 
 * Created on March 17, 2014, 7:51 PM
 */

#include "MatchGroupTabModel.h"

#include "Category.h"
#include "Tournament.h"
#include "../ui/GuiHelpers.h"
#include "CentralSignalEmitter.h"

#include <QDebug>

using namespace QTournament;
using namespace SqliteOverlay;

MatchGroupTableModel::MatchGroupTableModel(Tournament* tnmt)
:QAbstractTableModel(0), db(tnmt->getDatabaseHandle()), mgTab((db->getTab(TAB_MATCH_GROUP)))
{
  CentralSignalEmitter* cse = CentralSignalEmitter::getInstance();
  connect(cse, SIGNAL(beginCreateMatchGroup()), this, SLOT(onBeginCreateMatchGroup()), Qt::DirectConnection);
  connect(cse, SIGNAL(endCreateMatchGroup(int)), this, SLOT(onEndCreateMatchGroup(int)), Qt::DirectConnection);
  connect(cse, SIGNAL(matchGroupStatusChanged(int,int,OBJ_STATE,OBJ_STATE)), this, SLOT(onMatchGroupStatusChanged(int,int)), Qt::DirectConnection);
  connect(cse, SIGNAL(beginResetAllModels()), this, SLOT(onBeginResetModel()), Qt::DirectConnection);
  connect(cse, SIGNAL(endResetAllModels()), this, SLOT(onEndResetModel()), Qt::DirectConnection);
}

//----------------------------------------------------------------------------

int MatchGroupTableModel::rowCount(const QModelIndex& parent) const
{
  if (parent.isValid()) return 0;
  return mgTab->length();
}

//----------------------------------------------------------------------------

int MatchGroupTableModel::columnCount(const QModelIndex& parent) const
{
  if (parent.isValid()) return 0;
  return COLUMN_COUNT;
}

//----------------------------------------------------------------------------

QVariant MatchGroupTableModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
      //return QVariant();
      return QString("Invalid index");

    if (index.row() >= mgTab->length())
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
    
    auto tnmt = Tournament::getActiveTournament();
    auto mg = tnmt->getMatchMngr()->getMatchGroupBySeqNum(index.row());
    
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
    if (index.column() == STATE_COL_ID)
    {
      OBJ_STATE stat = mg->getState();
      return static_cast<int>(stat);
    }

    // sixth column: stage sequence number
    // Used for sorting in the "Staged Match Groups View" only and needs to be hidden in the associated view
    if (index.column() == STAGE_SEQ_COL_ID)
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
    if (section == STATE_COL_ID) {
      return tr("State");
    }
    if (section == STAGE_SEQ_COL_ID) {
      return tr("StageSeqNum");
    }

    return QString("Not implemented, section=" + QString::number(section));
  }
  
  return QString::number(section + 1);
}

//----------------------------------------------------------------------------

void MatchGroupTableModel::onBeginCreateMatchGroup()
{
  int newPos = mgTab->length();
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
  QModelIndex endIdx = createIndex(matchGroupSeqNum, COLUMN_COUNT-1);
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


