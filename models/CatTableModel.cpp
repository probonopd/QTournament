/* 
 * File:   PlayerTableModel.cpp
 * Author: volker
 * 
 * Created on March 17, 2014, 7:51 PM
 */

#include "CatTableModel.h"

#include "Category.h"
#include "Tournament.h"

using namespace QTournament;
using namespace dbOverlay;

CategoryTableModel::CategoryTableModel(TournamentDB* _db)
:QAbstractTableModel(0), db(_db), catTab((_db->getTab(TAB_CATEGORY)))
{
}

//----------------------------------------------------------------------------

int CategoryTableModel::rowCount(const QModelIndex& parent) const
{
  return catTab.length();
}

//----------------------------------------------------------------------------

int CategoryTableModel::columnCount(const QModelIndex& parent) const
{
  return 1;  // one column: category's
}

//----------------------------------------------------------------------------

QVariant CategoryTableModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
      return QVariant();

    if (index.row() >= catTab.length())
      return QVariant();

    if (role != Qt::DisplayRole)
      return QVariant();
    
    Category c = Tournament::getCatMngr()->getCategoryBySeqNum(index.row());
    
    // first column: name
    if (index.column() == 0)
    {
      return c.getName();
    }
        
    return QString("Not Implemented");
}

//----------------------------------------------------------------------------

QVariant CategoryTableModel::headerData(int section, Qt::Orientation orientation, int role) const
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


//----------------------------------------------------------------------------


