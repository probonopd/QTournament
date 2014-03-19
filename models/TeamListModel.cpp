/* 
 * File:   TeamListModel.cpp
 * Author: volker
 * 
 * Created on March 14, 2014, 9:04 PM
 */

#include <Qt>
#include <qt/QtWidgets/qmessagebox.h>

#include "TeamListModel.h"
#include "TeamMngr.h"
#include "Tournament.h"

using namespace dbOverlay;

namespace QTournament
{

  TeamListModel::TeamListModel(TournamentDB* _db)
  : QAbstractListModel(0), db(_db), teamTab(_db->getTab(TAB_TEAM))
  {
    connect(Tournament::getTeamMngr(), &TeamMngr::newTeamCreated, this, &TeamListModel::onTeamCreated);

  }

//----------------------------------------------------------------------------

  int TeamListModel::rowCount(const QModelIndex& parent) const
  {
    return teamTab.length();
  }

//----------------------------------------------------------------------------
  
  QVariant TeamListModel::data(const QModelIndex& index, int role) const
  {
    if (!index.isValid())
      return QVariant();

    if (index.row() >= teamTab.length())
      return QVariant();

    if (role == Qt::DisplayRole)
      return (teamTab.getSingleRowByColumnValue(GENERIC_SEQNUM_FIELD_NAME, index.row()))[GENERIC_NAME_FIELD_NAME];

    else
      return QVariant();
  }

//----------------------------------------------------------------------------

  QVariant TeamListModel::headerData(int section, Qt::Orientation orientation, int role) const
  {
    return QVariant();
  }

//----------------------------------------------------------------------------

  void TeamListModel::onTeamCreated(const Team& newTeam)
  {
    emit dataChanged(QModelIndex(), QModelIndex());
  }

//----------------------------------------------------------------------------

  Qt::ItemFlags TeamListModel::flags(const QModelIndex& index) const
  {
    if (!index.isValid()) {
      return Qt::ItemIsEnabled;
    }

    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
  }

//----------------------------------------------------------------------------

  bool TeamListModel::setData(const QModelIndex& index, const QVariant& value, int role)
  {
    if (!(index.isValid()) || (role != Qt::EditRole))
    {
      return false;
    }
    
    Team t = Tournament::getTeamMngr()->getTeamBySeqNum(index.row());
    
    ERR e = t.rename(value.toString());
    
    return (e == OK);
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

}