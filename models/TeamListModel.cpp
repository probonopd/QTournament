/* 
 * File:   TeamListModel.cpp
 * Author: volker
 * 
 * Created on March 14, 2014, 9:04 PM
 */

#include <Qt>
#include <QMessageBox>

#include "TeamListModel.h"
#include "TeamMngr.h"
#include "CentralSignalEmitter.h"

using namespace SqliteOverlay;

namespace QTournament
{

  TeamListModel::TeamListModel(TournamentDB* _db)
  : QAbstractListModel(0), db(_db), teamTab(db->getTab(TAB_TEAM))
  {
    CentralSignalEmitter* cse = CentralSignalEmitter::getInstance();
    connect(cse, SIGNAL(beginCreateTeam()), this, SLOT(onBeginCreateTeam()), Qt::DirectConnection);
    connect(cse, SIGNAL(endCreateTeam(int)), this, SLOT(onEndCreateTeam(int)), Qt::DirectConnection);
    connect(cse, SIGNAL(teamRenamed(int)), this, SLOT(onTeamRenamed(int)), Qt::DirectConnection);
  }

//----------------------------------------------------------------------------

  int TeamListModel::rowCount(const QModelIndex& parent) const
  {
    return teamTab->length();
  }

//----------------------------------------------------------------------------
  
  QVariant TeamListModel::data(const QModelIndex& index, int role) const
  {
    if (!index.isValid())
      return QVariant();

    if (index.row() >= teamTab->length())
      return QVariant();

    if (role == Qt::DisplayRole)
    {
      string name = teamTab->getSingleRowByColumnValue(GENERIC_SEQNUM_FIELD_NAME, index.row())[GENERIC_NAME_FIELD_NAME];
      return QString::fromUtf8(name.data());
    }

    else
      return QVariant();
  }

//----------------------------------------------------------------------------

  QVariant TeamListModel::headerData(int section, Qt::Orientation orientation, int role) const
  {
    return QVariant();
  }

//----------------------------------------------------------------------------

  void TeamListModel::onBeginCreateTeam()
  {
    int newPos = teamTab->length();
    beginInsertRows(QModelIndex(), newPos, newPos);
  }

//----------------------------------------------------------------------------

  void TeamListModel::onEndCreateTeam(int newTeamSeqNum)
  {
    endInsertRows();
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
    
    TeamMngr tm{db};
    Team t = tm.getTeamBySeqNum(index.row());
    
    ERR e = t.rename(value.toString());
    
    return (e == OK);
  }

//----------------------------------------------------------------------------

  void TeamListModel::onTeamRenamed(int teamSeqNum)
  {
    QModelIndex index = QAbstractItemModel::createIndex(teamSeqNum, 0);
    emit dataChanged(index, index);
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

}
