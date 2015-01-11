/* 
 * File:   PlayerTableModel.cpp
 * Author: volker
 * 
 * Created on March 17, 2014, 7:51 PM
 */

#include "MatchGroupTabModel.h"

#include "Category.h"
#include "Tournament.h"

using namespace QTournament;
using namespace dbOverlay;

MatchGroupTableModel::MatchGroupTableModel(TournamentDB* _db)
:QAbstractTableModel(0), db(_db), mgTab((_db->getTab(TAB_MATCH_GROUP)))
{
  connect(Tournament::getMatchMngr(), &MatchMngr::beginCreateMatchGroup, this, &MatchGroupTableModel::onBeginCreateMatchGroup);
  connect(Tournament::getMatchMngr(), &MatchMngr::endCreateMatchGroup, this, &MatchGroupTableModel::onEndCreateMatchGroup);
}

//----------------------------------------------------------------------------

int MatchGroupTableModel::rowCount(const QModelIndex& parent) const
{
  return mgTab.length();
}

//----------------------------------------------------------------------------

int MatchGroupTableModel::columnCount(const QModelIndex& parent) const
{
  return 4;
}

//----------------------------------------------------------------------------

QVariant MatchGroupTableModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
      return QVariant();

    if (index.row() >= mgTab.length())
      return QVariant();

    if (role != Qt::DisplayRole)
      return QVariant();
    
    auto mg = Tournament::getMatchMngr()->getMatchGroupBySeqNum(index.row());
    
    // first column: category
    if (index.column() == 0)
    {
      Category c = mg->getCategory();
      return c.getName();
    }

    // second column: group number, if applicable
    if (index.column() == 1)
    {
      int grpNum = mg->getGroupNumber();
      if (grpNum > 0) return grpNum;
      return "";
    }

    // third column: round
    if (index.column() == 2)
    {
      return mg->getRound();
    }

    // fourth column: number of matches in this match group
    if (index.column() == 3)
    {
      return mg->getMatchCount();
    }

    return QString("Not Implemented");
}

//----------------------------------------------------------------------------

QVariant MatchGroupTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (role != Qt::DisplayRole)
  {
    return QVariant();
  }
  
  if (orientation == Qt::Horizontal)
  {
    if (section == 0) {
      return tr("Category");
    }
    if (section == 1) {
      return tr("Group");
    }
    if (section == 2) {
      return tr("Round");
    }
    if (section == 3) {
      return tr("Number of matches");
    }

    return QString("Not implemented");
  }
  
  return QString::number(section + 1);
}

//----------------------------------------------------------------------------

void MatchGroupTableModel::onBeginCreateMatchGroup()
{
  beginInsertRows(QModelIndex(), mgTab.length(), mgTab.length());
}
//----------------------------------------------------------------------------

void MatchGroupTableModel::onEndCreateMatchGroup(int newCatSeqNum)
{
  endInsertRows();
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


