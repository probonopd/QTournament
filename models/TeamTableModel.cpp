/*
 *    This is QTournament, a badminton tournament management program.
 *    Copyright (C) 2014 - 2019  Volker Knollmann
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

#include "TeamTableModel.h"
#include "TeamMngr.h"
#include "CentralSignalEmitter.h"

using namespace SqliteOverlay;

namespace QTournament
{

  TeamTableModel::TeamTableModel(const TournamentDB& _db)
    : QAbstractTableModel{nullptr}, db{_db}, teamTab{DbTab{db, TabTeam, false}}
  {
    CentralSignalEmitter* cse = CentralSignalEmitter::getInstance();
    connect(cse, SIGNAL(beginCreateTeam()), this, SLOT(onBeginCreateTeam()), Qt::DirectConnection);
    connect(cse, SIGNAL(endCreateTeam(int)), this, SLOT(onEndCreateTeam(int)), Qt::DirectConnection);
    connect(cse, SIGNAL(teamRenamed(int)), this, SLOT(onTeamRenamed(int)), Qt::DirectConnection);
  }

//----------------------------------------------------------------------------

  int TeamTableModel::rowCount(const QModelIndex& parent) const
  {
    return teamTab.length();
  }

//----------------------------------------------------------------------------

  int TeamTableModel::columnCount(const QModelIndex& parent) const
  {
    return ColCount;
  }

//----------------------------------------------------------------------------
  
  QVariant TeamTableModel::data(const QModelIndex& index, int role) const
  {
    if (!index.isValid())
      return QVariant();

    if (index.row() >= teamTab.length())
      return QVariant();

    if (role != Qt::DisplayRole)
      return QVariant();

    TeamMngr tm{db};
    Team t = tm.getTeamBySeqNum(index.row());

    if (index.column() == NameColId)
    {
      return t.getName();
    }

    if (index.column() == MemberCountColId)
    {
      return t.getMemberCount();
    }

    if (index.column() == UnregisteredMemberCountColId)
    {
      int unreg = t.getUnregisteredMemberCount();
      if (unreg > 0) return unreg;
      return QString();
    }

    return QString("Not implemented");
  }

//----------------------------------------------------------------------------

  QVariant TeamTableModel::headerData(int section, Qt::Orientation orientation, int role) const
  {
    if (role != Qt::DisplayRole)
    {
      return QVariant();
    }

    if (orientation == Qt::Horizontal)
    {
      if (section == NameColId) {
        return tr("Name");
      }
      if (section == MemberCountColId) {
        return tr("Size");
      }
      if (section == UnregisteredMemberCountColId) {
        return tr("Not reg.");
      }

      return QString("Not implemented");
    }

    return (section + 1);
  }

//----------------------------------------------------------------------------

  void TeamTableModel::onBeginCreateTeam()
  {
    int newPos = teamTab.length();
    beginInsertRows(QModelIndex(), newPos, newPos);
  }

//----------------------------------------------------------------------------

  void TeamTableModel::onEndCreateTeam(int newTeamSeqNum)
  {
    endInsertRows();
  }

//----------------------------------------------------------------------------

  void TeamTableModel::onTeamRenamed(int teamSeqNum)
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
