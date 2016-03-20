/* 
 * File:   PlayerTableModel.cpp
 * Author: volker
 * 
 * Created on March 17, 2014, 7:51 PM
 */

#include "PlayerTableModel.h"

#include "Player.h"
#include "PlayerTableModel.h"
#include "Tournament.h"

using namespace QTournament;
using namespace SqliteOverlay;

PlayerTableModel::PlayerTableModel(Tournament* tnmt)
:QAbstractTableModel(0), db(tnmt->getDatabaseHandle()), playerTab(db->getTab(TAB_PLAYER)),
        teamTab(db->getTab(TAB_TEAM)), catTab((db->getTab(TAB_CATEGORY)))
{
  connect(tnmt->getTeamMngr(), SIGNAL(teamRenamed(int)), this, SLOT(onTeamRenamed(int)), Qt::DirectConnection);

  PlayerMngr* pm = tnmt->getPlayerMngr();
  connect(pm, SIGNAL(beginCreatePlayer()), this, SLOT(onBeginCreatePlayer()), Qt::DirectConnection);
  connect(pm, SIGNAL(endCreatePlayer(int)), this, SLOT(onEndCreatePlayer(int)), Qt::DirectConnection);
  connect(pm, SIGNAL(playerRenamed(Player)), this, SLOT(onPlayerRenamed(Player)), Qt::DirectConnection);
  connect(pm, SIGNAL(playerStatusChanged(int,int,OBJ_STATE,OBJ_STATE)), this, SLOT(onPlayerStatusChanged(int,int)), Qt::DirectConnection);
  connect(pm, SIGNAL(beginDeletePlayer(int)), this, SLOT(onBeginDeletePlayer(int)), Qt::DirectConnection);
  connect(pm, SIGNAL(endDeletePlayer()), this, SLOT(onEndDeletePlayer()), Qt::DirectConnection);

  CatMngr* cm = tnmt->getCatMngr();
  connect(cm, SIGNAL(beginResetAllModels()), this, SLOT(onBeginResetModel()), Qt::DirectConnection);
  connect(cm, SIGNAL(endResetAllModels()), this, SLOT(onEndResetModel()), Qt::DirectConnection);
}

//----------------------------------------------------------------------------

int PlayerTableModel::rowCount(const QModelIndex& parent) const
{
  return playerTab->length();
}

//----------------------------------------------------------------------------

int PlayerTableModel::columnCount(const QModelIndex& parent) const
{
  return 4;  // four columns: player's name, sex, team and assigned categories
}

//----------------------------------------------------------------------------

QVariant PlayerTableModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
      return QVariant();

    if (index.row() >= playerTab->length())
      return QVariant();

    if (role != Qt::DisplayRole)
      return QVariant();
    
    auto tnmt = Tournament::getActiveTournament();
    auto p = tnmt->getPlayerMngr()->getPlayerBySeqNum(index.row());
    // no check for a nullptr here, the call above MUST succeed
    
    // first column: name
    if (index.column() == 0)
    {
      return p->getDisplayName();
    }
    
    // second column: sex
    if (index.column() == 1)
    {
      return ((p->getSex() == M) ? QString("♂") : QString("♀"));
    }
    
    // third column: team name
    if (index.column() == 2)
    {
      return p->getTeam().getName();
    }
    
    // fourth column: assigned categories
    if (index.column() == 3)
    {
      QString result = "";
      vector<Category> assignedCats = p->getAssignedCategories();
      for (int i=0; i < assignedCats.size(); i++)
      {
        result += assignedCats.at(i).getName() + ", ";
      }
      
      if (assignedCats.size() > 0)
      {
        result = result.left(result.length() - 2);
      }
      
      return result;
    }
    
    return QString("Not Implemented");
}

//----------------------------------------------------------------------------

QVariant PlayerTableModel::headerData(int section, Qt::Orientation orientation, int role) const
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
      return "";
    }

    if (section == 2) {
      return tr("Team Name");
    }
    
    if (section == 3) {
      return tr("Categories");
    }
    
    return QString("Not implemented");
  }
  
  return QString::number(section + 1);
}

//----------------------------------------------------------------------------

void PlayerTableModel::onBeginCreatePlayer()
{
  int newPos = playerTab->length();
  beginInsertRows(QModelIndex(), newPos, newPos);
}

//----------------------------------------------------------------------------

void PlayerTableModel::onEndCreatePlayer(int newPlayerSeqNum)
{
  endInsertRows();
}

//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

void PlayerTableModel::onTeamRenamed(int teamSeqNum)
{
  // for now, simply update the whole "team" column, even if only
  // one team name has changed
  QModelIndex top = QAbstractItemModel::createIndex(0, 2);
  QModelIndex bottom = QAbstractItemModel::createIndex(rowCount() - 1, 2);
  emit dataChanged(top, bottom);
}

//----------------------------------------------------------------------------

void PlayerTableModel::onPlayerRenamed(const Player& p)
{
  int seqNum = p.getSeqNum();
  QModelIndex top = QAbstractItemModel::createIndex(seqNum, 0);
  QModelIndex bottom = QAbstractItemModel::createIndex(seqNum, 1);
  emit dataChanged(top, bottom);  
}

//----------------------------------------------------------------------------

void PlayerTableModel::onPlayerStatusChanged(int playerId, int playerSeqNum)
{
  QModelIndex startIdx = createIndex(playerSeqNum, 0);
  QModelIndex endIdx = createIndex(playerSeqNum, COLUMN_COUNT-1);
  emit dataChanged(startIdx, endIdx);
}

//----------------------------------------------------------------------------

void PlayerTableModel::onBeginDeletePlayer(int playerSeqNum)
{
  beginRemoveRows(QModelIndex(), playerSeqNum, playerSeqNum);
}

//----------------------------------------------------------------------------

void PlayerTableModel::onEndDeletePlayer()
{
  endRemoveRows();
}

//----------------------------------------------------------------------------

void PlayerTableModel::onBeginResetModel()
{
  beginResetModel();
}

//----------------------------------------------------------------------------

void PlayerTableModel::onEndResetModel()
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


