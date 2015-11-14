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
  connect(Tournament::getCatMngr(), SIGNAL(beginDeleteCategory(int)), this, SLOT(onBeginDeleteCategory(int)), Qt::DirectConnection);
  connect(Tournament::getCatMngr(), SIGNAL(endDeleteCategory(int)), this, SLOT(onEndDeleteCategory(int)), Qt::DirectConnection);
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
    
    // name
    if (index.column() == COL_NAME)
    {
      return c.getName();
    }

    // number of finished rounds
    if (index.column() == COL_FINISHED_ROUNDS)
    {
      CatRoundStatus crs = c.getRoundStatus();
      int finishedRounds = crs.getFinishedRoundsCount();
      if (finishedRounds == CatRoundStatus::NO_ROUNDS_FINISHED_YET)
      {
        return "--";
      }

      return finishedRounds;
    }

    // number of currently played round
    if (index.column() == COL_CURRENT_ROUND)
    {
      CatRoundStatus crs = c.getRoundStatus();
      int currentRoundNum = crs.getCurrentlyRunningRoundNumber();

      if (currentRoundNum == CatRoundStatus::NO_CURRENTLY_RUNNING_ROUND)
      {
        return "--";
      }

      if (currentRoundNum == CatRoundStatus::MULTIPLE_ROUNDS_RUNNING)
      {
        // generate a comma-separated list of currently running rounds
        QList<int> runningRounds = crs.getCurrentlyRunningRoundNumbers();
        QString result = "";
        for (int round : runningRounds)
        {
          result += QString::number(round) + ", ";
        }
        return result.left(result.length() - 2);
      }

      return currentRoundNum;
    }

    // total number of matches in the current round
    if (index.column() == COL_TOTAL_MATCHES)
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

    // number of unfinished matches in the current round
    if (index.column() == COL_UNFINISHED_MATCHES)
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

    // number of running matches in the current round
    if (index.column() == COL_RUNNING_MATCHES)
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

    // number of waiting matches in the current round
    if (index.column() == COL_WAITING_MATCHES)
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

    // total number of rounds
    if (index.column() == COL_TOTAL_ROUNDS)
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
    if (section == COL_NAME) {
      return tr("Name");
    }
    
    if (section == COL_FINISHED_ROUNDS) {
      return tr("Finished rounds");
    }

    if (section == COL_CURRENT_ROUND) {
      return tr("Current round");
    }

    if (section == COL_TOTAL_MATCHES) {
      return tr("Total matches\nin current round");
    }

    if (section == COL_UNFINISHED_MATCHES) {
      return tr("Unfinished matches\nin current round");
    }

    if (section == COL_RUNNING_MATCHES) {
      return tr("Running matches\nin current round");
    }

    if (section == COL_WAITING_MATCHES) {
      return tr("Waiting matches\nin current round");
    }

    if (section == COL_TOTAL_ROUNDS) {
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


