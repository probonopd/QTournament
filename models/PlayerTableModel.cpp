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
    //connect(Tournament::getPlayerMngr(), &PlayerMngr::newPlayerCreated, this, &PlayerTableModel::onPlayerCreated);
}

//----------------------------------------------------------------------------

int PlayerTableModel::rowCount(const QModelIndex& parent) const
{
  return playerTab.length();
}

//----------------------------------------------------------------------------

int PlayerTableModel::columnCount(const QModelIndex& parent) const
{
  return 3;  // three columns: player's name, sex and team
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
    
    return QString("Not implemented");
  }
  
  return QString::number(section + 1);
}

//----------------------------------------------------------------------------

void PlayerTableModel::onPlayerCreated(const Player& newPlayer)
{
  emit dataChanged(QModelIndex(), QModelIndex());
}

//----------------------------------------------------------------------------

ERR PlayerTableModel::createNewPlayer(const QString& firstName, const QString& lastName, SEX sex, const QString& teamName)
{
  PlayerMngr* pmngr = Tournament::getPlayerMngr();
  
  beginInsertRows(QModelIndex(), playerTab.length(), playerTab.length());
  ERR result = pmngr->createNewPlayer(firstName, lastName, sex, teamName);
  endInsertRows();
  
  return result;
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


