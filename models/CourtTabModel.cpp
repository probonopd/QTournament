/*
 *    This is QTournament, a badminton tournament management program.
 *    Copyright (C) 2014 - 2017  Volker Knollmann
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

#include <QDebug>

#include "CourtTabModel.h"

#include "Category.h"
#include "../ui/GuiHelpers.h"
#include "CourtMngr.h"
#include "CentralSignalEmitter.h"
#include "MatchMngr.h"

using namespace QTournament;
using namespace SqliteOverlay;

CourtTableModel::CourtTableModel(const TournamentDB& _db)
  :QAbstractTableModel{nullptr}, db{_db}, courtTab{DbTab{db, TabCourt, false}}
{
  CentralSignalEmitter* cse = CentralSignalEmitter::getInstance();
  connect(cse, SIGNAL(beginCreateCourt()), this, SLOT(onBeginCreateCourt()), Qt::DirectConnection);
  connect(cse, SIGNAL(endCreateCourt(int)), this, SLOT(onEndCreateCourt(int)), Qt::DirectConnection);
  connect(cse, SIGNAL(courtStatusChanged(int,int,ObjState,ObjState)), this, SLOT(onCourtStatusChanged(int,int)), Qt::DirectConnection);
  connect(cse, SIGNAL(beginDeleteCourt(int)), this, SLOT(onBeginDeleteCourt(int)), Qt::DirectConnection);
  connect(cse, SIGNAL(endDeleteCourt()), this, SLOT(onEndDeleteCourt()), Qt::DirectConnection);

  // a timer for updatng the match duration
  durationUpdateTimer = std::make_unique<QTimer>(this);
  connect(durationUpdateTimer.get(), SIGNAL(timeout()), this, SLOT(onDurationUpdateTimerElapsed()));
  durationUpdateTimer->start(DurationUpdatePeriod_ms);
}

//----------------------------------------------------------------------------

int CourtTableModel::rowCount(const QModelIndex& parent) const
{
  if (parent.isValid()) return 0;
  return courtTab.length();
}

//----------------------------------------------------------------------------

int CourtTableModel::columnCount(const QModelIndex& parent) const
{
  if (parent.isValid()) return 0;
  return ColumnCount;
}

//----------------------------------------------------------------------------

QVariant CourtTableModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
      //return QVariant();
      return QString("Invalid index");

    if (index.row() >= courtTab.length())
      //return QVariant();
      return QString("Invalid row: " + QString::number(index.row()));

    if (role != Qt::DisplayRole)
      return QVariant();
    
    CourtMngr cm{db};
    auto co = cm.getCourtBySeqNum(index.row());
    
    // first column: court number
    if (index.column() == CourtNumColId)
    {
      return co->getNumber();
    }

    // second column: match name
    if (index.column() == 1)
    {
      if (co->getState() != ObjState::CO_Busy) return "";
      return "(FIX: need to look up match data)";
    }

    // third column: match duration in minutes
    if (index.column() == DurationColId)
    {
      if (!co) return "??";

      MatchMngr mm{db};
      auto ma = mm.getMatchForCourt(*co);
      if (!ma) return QString(); // empty court

      int duration = ma->getMatchDuration();
      if (duration < 0) return "??";

      // create a hour::minute string
      int hours = duration / 3600;
      int minutes = (duration % 3600) / 60;
      QString result = "%1:%2";
      result = result.arg(hours).arg(minutes, 2, 10, QLatin1Char('0'));
      return result;
    }


    return QString("Not Implemented, row=" + QString::number(index.row()) + ", col=" + QString::number(index.column()));
}

//----------------------------------------------------------------------------

QVariant CourtTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (role != Qt::DisplayRole)
  {
    return QVariant();
  }

  if (section < 0)
  {
    return QVariant();
  }
  
  if (orientation == Qt::Horizontal)
  {
    if (section == CourtNumColId) {
      return tr("Court");
    }
    if (section == 1) {
      return tr("Match");
    }
    if (section == DurationColId) {
      return tr("Duration");
    }

    return QString("Not implemented, section=" + QString::number(section));
  }
  
  return QString::number(section + 1);
}

//----------------------------------------------------------------------------

void CourtTableModel::onBeginCreateCourt()
{
  int newPos = courtTab.length();
  beginInsertRows(QModelIndex(), newPos, newPos);
}
//----------------------------------------------------------------------------

void CourtTableModel::onEndCreateCourt(int newCourtSeqNum)
{
  endInsertRows();
}

//----------------------------------------------------------------------------

void CourtTableModel::onCourtStatusChanged(int courtId, int courtSeqNum)
{
  QModelIndex startIdx = createIndex(courtSeqNum, 0);
  QModelIndex endIdx = createIndex(courtSeqNum, ColumnCount-1);
  emit dataChanged(startIdx, endIdx);
}

//----------------------------------------------------------------------------

void CourtTableModel::onDurationUpdateTimerElapsed()
{
  QModelIndex startIdx = createIndex(0, DurationColId);
  QModelIndex endIdx = createIndex(rowCount(), DurationColId);
  emit dataChanged(startIdx, endIdx);
}

//----------------------------------------------------------------------------

void CourtTableModel::onBeginDeleteCourt(int courtSeqNum)
{
  beginRemoveRows(QModelIndex(), courtSeqNum, courtSeqNum);
}

//----------------------------------------------------------------------------

void CourtTableModel::onEndDeleteCourt()
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


