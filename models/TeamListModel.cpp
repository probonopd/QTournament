/*
 *    This is QTournament, a badminton tournament management program.
 *    Copyright (C) 2014 - 2016  Volker Knollmann
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
