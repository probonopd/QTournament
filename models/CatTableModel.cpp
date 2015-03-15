/* 
 * File:   PlayerTableModel.cpp
 * Author: volker
 * 
 * Created on March 17, 2014, 7:51 PM
 */

#include "CatTableModel.h"

#include "Category.h"
#include "Tournament.h"
#include "CatRoundStatus.h"

using namespace QTournament;
using namespace dbOverlay;

CategoryTableModel::CategoryTableModel(TournamentDB* _db)
:QAbstractTableModel(0), db(_db), catTab((_db->getTab(TAB_CATEGORY)))
{
  connect(Tournament::getCatMngr(), &CatMngr::beginCreateCategory, this, &CategoryTableModel::onBeginCreateCategory, Qt::DirectConnection);
  connect(Tournament::getCatMngr(), &CatMngr::endCreateCategory, this, &CategoryTableModel::onEndCreateCategory, Qt::DirectConnection);
}

//----------------------------------------------------------------------------

int CategoryTableModel::rowCount(const QModelIndex& parent) const
{
  return catTab.length();
}

//----------------------------------------------------------------------------

int CategoryTableModel::columnCount(const QModelIndex& parent) const
{
  return COLUMN_COUNT;
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
    
    Category c = Tournament::getCatMngr()->getCategoryBySeqNum(index.row());
    
    // first column: name
    if (index.column() == 0)
    {
      return c.getName();
    }

    // second column: number of finished rounds
    if (index.column() == 1)
    {
      CatRoundStatus crs = c.getRoundStatus();
      int finishedRounds = crs.getFinishedRoundsCount();
      if (finishedRounds == CatRoundStatus::NO_ROUNDS_FINISHED_YET)
      {
        return "--";
      }

      return finishedRounds;
    }

    // third column: number of currently played round
    if (index.column() == 2)
    {
      CatRoundStatus crs = c.getRoundStatus();
      int currentRoundNum = crs.getCurrentlyRunningRoundNumber();
      if (currentRoundNum == CatRoundStatus::NO_CURRENTLY_RUNNING_ROUND)
      {
        return "--";
      }

      return currentRoundNum;
    }

    // fourth column: total number of matches in the current round
    if (index.column() == 3)
    {
      CatRoundStatus crs = c.getRoundStatus();
      auto matchStat = crs.getMatchCountForCurrentRound();
      int total = get<0>(matchStat);
      if (total == CatRoundStatus::NO_CURRENTLY_RUNNING_ROUND)
      {
        return "--";
      }

      return total;
    }

    // fifth column: number of unfinished matches in the current round
    if (index.column() == 4)
    {
      CatRoundStatus crs = c.getRoundStatus();
      auto matchStat = crs.getMatchCountForCurrentRound();
      int unfinished = get<1>(matchStat);
      if (unfinished == CatRoundStatus::NO_CURRENTLY_RUNNING_ROUND)
      {
        return "--";
      }

      return unfinished;
    }

    // sixth column: number of running matches in the current round
    if (index.column() == 5)
    {
      CatRoundStatus crs = c.getRoundStatus();
      auto matchStat = crs.getMatchCountForCurrentRound();
      int runningMatches = get<2>(matchStat);
      if (runningMatches == CatRoundStatus::NO_CURRENTLY_RUNNING_ROUND)
      {
        return "--";
      }

      return runningMatches;
    }

    // seventh column: number of waiting matches in the current round
    if (index.column() == 6)
    {
      CatRoundStatus crs = c.getRoundStatus();
      auto matchStat = crs.getMatchCountForCurrentRound();

      // Waiting = unfinished - running
      int waitingMatches = get<1>(matchStat) - get<2>(matchStat);

      if (waitingMatches <= 0)
      {
        return "--";
      }

      return waitingMatches;
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
    if (section == 0) {
      return tr("Name");
    }
    
    if (section == 1) {
      return tr("Finished rounds");
    }

    if (section == 2) {
      return tr("Current round");
    }

    if (section == 3) {
      return tr("Total matches\nin current round");
    }

    if (section == 4) {
      return tr("Unfinished matches\nin current round");
    }

    if (section == 5) {
      return tr("Running matches\nin current round");
    }

    if (section == 6) {
      return tr("Waiting matches\nin current round");
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


