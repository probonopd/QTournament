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
}

//----------------------------------------------------------------------------

int PlayerTableModel::rowCount(const QModelIndex& parent) const
{
  return playerTab.length();
}

//----------------------------------------------------------------------------

int PlayerTableModel::columnCount(const QModelIndex& parent) const
{
  return 1;  // only one column, for now (player's name)
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
    
    if (index.column() == 0)
    {
      return p.getDisplayName();
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
    
    return QString("Not implemented");
  }
  
  return QString::number(section + 1);
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


//----------------------------------------------------------------------------


