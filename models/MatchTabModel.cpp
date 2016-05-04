/* 
 * File:   PlayerTableModel.cpp
 * Author: volker
 * 
 * Created on March 17, 2014, 7:51 PM
 */

#include <QDebug>

#include "MatchTabModel.h"
#include "Category.h"
#include "../ui/GuiHelpers.h"
#include "CentralSignalEmitter.h"
#include "MatchMngr.h"

using namespace QTournament;
using namespace SqliteOverlay;

MatchTableModel::MatchTableModel(TournamentDB* _db)
:QAbstractTableModel(0), db(_db), matchTab((db->getTab(TAB_MATCH)))
{
  CentralSignalEmitter* cse = CentralSignalEmitter::getInstance();
  connect(cse, SIGNAL(beginCreateMatch()), this, SLOT(onBeginCreateMatch()), Qt::DirectConnection);
  connect(cse, SIGNAL(endCreateMatch(int)), this, SLOT(onEndCreateMatch(int)), Qt::DirectConnection);
  connect(cse, SIGNAL(matchStatusChanged(int,int,OBJ_STATE,OBJ_STATE)), this, SLOT(onMatchStatusChanged(int,int)), Qt::DirectConnection);
  connect(cse, SIGNAL(beginResetAllModels()), this, SLOT(onBeginResetModel()), Qt::DirectConnection);
  connect(cse, SIGNAL(endResetAllModels()), this, SLOT(onEndResetModel()), Qt::DirectConnection);
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
      return GuiHelpers::groupNumToString(mg.getGroupNumber());
    }

    // sixth column: the match state; this column is used for filtering and
    // needs to be hidden in the view
    if (index.column() == STATE_COL_ID)
    {
      return static_cast<int>(ma->getState());
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

    return QString("Not implemented, section=" + QString::number(section));
  }
  
  return QString::number(section + 1);
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
}

//----------------------------------------------------------------------------

void MatchTableModel::onMatchStatusChanged(int matchId, int matchSeqNum)
{
  QModelIndex startIdx = createIndex(matchSeqNum, 0);
  QModelIndex endIdx = createIndex(matchSeqNum, COLUMN_COUNT-1);
  emit dataChanged(startIdx, endIdx);
}

//----------------------------------------------------------------------------

void MatchTableModel::onBeginResetModel()
{
  beginResetModel();
}

//----------------------------------------------------------------------------

void MatchTableModel::onEndResetModel()
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


