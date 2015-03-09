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
using namespace dbOverlay;

PlayerTableModel::PlayerTableModel(TournamentDB* _db)
:QAbstractTableModel(0), db(_db), playerTab(_db->getTab(TAB_PLAYER)),
        teamTab(_db->getTab(TAB_TEAM)), catTab((_db->getTab(TAB_CATEGORY)))
{
  connect(Tournament::getTeamMngr(), &TeamMngr::teamRenamed, this, &PlayerTableModel::onTeamRenamed);
  connect(Tournament::getPlayerMngr(), &PlayerMngr::beginCreatePlayer, this, &PlayerTableModel::onBeginCreatePlayer, Qt::DirectConnection);
  connect(Tournament::getPlayerMngr(), &PlayerMngr::endCreatePlayer, this, &PlayerTableModel::onEndCreatePlayer, Qt::DirectConnection);
  connect(Tournament::getPlayerMngr(), &PlayerMngr::playerRenamed, this, &PlayerTableModel::onPlayerRenamed, Qt::DirectConnection);
  connect(Tournament::getPlayerMngr(), &PlayerMngr::playerStatusChanged, this, &PlayerTableModel::onPlayerStatusChanged, Qt::DirectConnection);
}

//----------------------------------------------------------------------------

int PlayerTableModel::rowCount(const QModelIndex& parent) const
{
  return playerTab.length();
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

    if (index.row() >= playerTab.length())
      return QVariant();

    if (role != Qt::DisplayRole)
      return QVariant();
    
    Player p = Tournament::getPlayerMngr()->getPlayerBySeqNum(index.row());
    
    // first column: name
    if (index.column() == 0)
    {
      return p.getDisplayName();
    }
    
    // second column: sex
    if (index.column() == 1)
    {
      return ((p.getSex() == M) ? QString("♂") : QString("♀"));
    }
    
    // third column: team name
    if (index.column() == 2)
    {
      return p.getTeam().getName();
    }
    
    // fourth column: assigned categories
    if (index.column() == 3)
    {
      QString result = "";
      QList<Category> assignedCats = p.getAssignedCategories();
      for (int i=0; i < assignedCats.count(); i++)
      {
	result += assignedCats.at(i).getName() + ", ";
      }
      
      if (assignedCats.count() > 0)
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
  int newPos = playerTab.length();
  beginInsertRows(QModelIndex(), newPos, newPos);
}

//----------------------------------------------------------------------------

void PlayerTableModel::onEndCreatePlayer(int newPlayerSeqNum)
{
  emit endInsertRows();
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


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


