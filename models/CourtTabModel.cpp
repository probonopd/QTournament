/* 
 * File:   PlayerTableModel.cpp
 * Author: volker
 * 
 * Created on March 17, 2014, 7:51 PM
 */

#include "CourtTabModel.h"

#include "Category.h"
#include "Tournament.h"
#include "../ui/GuiHelpers.h"
#include <QDebug>

using namespace QTournament;
using namespace dbOverlay;

CourtTableModel::CourtTableModel(TournamentDB* _db)
:QAbstractTableModel(0), db(_db), courtTab((_db->getTab(TAB_COURT)))
{
  connect(Tournament::getCourtMngr(), &CourtMngr::beginCreateCourt, this, &CourtTableModel::onBeginCreateCourt, Qt::DirectConnection);
  connect(Tournament::getCourtMngr(), &CourtMngr::endCreateCourt, this, &CourtTableModel::onEndCreateCourt, Qt::DirectConnection);
  connect(Tournament::getCourtMngr(), &CourtMngr::courtStatusChanged, this, &CourtTableModel::onCourtStatusChanged, Qt::DirectConnection);
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
  return COLUMN_COUNT;
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
    
    auto co = Tournament::getCourtMngr()->getCourtBySeqNum(index.row());
    
    // first column: court number
    if (index.column() == COURT_NUM_COL_ID)
    {
      return co->getNumber();
    }

    // second column: match name
    if (index.column() == 1)
    {
      if (co->getState() != STAT_CO_BUSY) return "";
      return "(FIX: need to look up match data)";
    }


    return QString("Not Implemented, row=" + QString::number(index.row()) + ", col=" + QString::number(index.row()));
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
    if (section == COURT_NUM_COL_ID) {
      return tr("Court");
    }
    if (section == 1) {
      return tr("Match");
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
  QModelIndex endIdx = createIndex(courtSeqNum, COLUMN_COUNT-1);
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


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


