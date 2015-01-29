/* 
 * File:   PlayerTableView.cpp
 * Author: volker
 * 
 * Created on March 17, 2014, 8:19 PM
 */

#include "MatchGroupTableView.h"
#include "MainFrame.h"

MatchGroupTableView::MatchGroupTableView(QWidget* parent)
  :QTableView(parent), currentFilter(FilterType::NONE)
{
  // an empty model for clearing the table when
  // no tournament is open
  emptyModel = new QStringListModel();

  // prepare a proxy model to support sorting by columns
  sortedModel = new QSortFilterProxyModel();
  sortedModel->setSourceModel(emptyModel);
  setModel(sortedModel);

  connect(MainFrame::getMainFramePointer(), &MainFrame::tournamentOpened, this, &MatchGroupTableView::onTournamentOpened);
  
  // define a delegate for drawing the player items
  //itemDelegate = new PlayerItemDelegate(this);
  //setItemDelegate(itemDelegate);
  
  
}

//----------------------------------------------------------------------------
    
MatchGroupTableView::~MatchGroupTableView()
{
  delete emptyModel;
  delete sortedModel;
  //delete itemDelegate;
}

//----------------------------------------------------------------------------
    
void MatchGroupTableView::onTournamentOpened(Tournament* _tnmt)
{
  tnmt = _tnmt;
  sortedModel->setSourceModel(Tournament::getMatchGroupTableModel());
  setColumnHidden(MatchGroupTableModel::STATE_COL_ID, true);  // hide the column containing the internal object state
  setColumnHidden(MatchGroupTableModel::STAGE_SEQ_COL_ID, true);  // hide the column containing the stage sequence number
  setEnabled(true);
  
  // connect signals from the Tournament and TeamMngr with my slots
  connect(tnmt, &Tournament::tournamentClosed, this, &MatchGroupTableView::onTournamentClosed, Qt::DirectConnection);

  // receive triggers from the underlying models when a filter update is necessary
  connect(Tournament::getMatchGroupTableModel(), &MatchGroupTableModel::triggerFilterUpdate, this, &MatchGroupTableView::onFilterUpdateTriggered, Qt::DirectConnection);
  
  // resize columns and rows to content once (we do not want permanent automatic resizing)
  horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
  verticalHeader()->resizeSections(QHeaderView::ResizeToContents);
}

//----------------------------------------------------------------------------
    
void MatchGroupTableView::onTournamentClosed()
{
  // disconnect from all signals, because
  // the sending objects don't exist anymore
  disconnect(Tournament::getMatchGroupTableModel(), &MatchGroupTableModel::triggerFilterUpdate, this, &MatchGroupTableView::onFilterUpdateTriggered);
  disconnect(tnmt, &Tournament::tournamentClosed, this, &MatchGroupTableView::onTournamentClosed);
  
  // invalidate the tournament handle and deactivate the view
  tnmt = nullptr;
  sortedModel->setSourceModel(emptyModel);
  setEnabled(false);
  
}

//----------------------------------------------------------------------------

void MatchGroupTableView::setFilter(FilterType ft)
{
  currentFilter = ft;

  if (ft == FilterType::NONE)
  {
    sortedModel->setFilterFixedString("");
    return;
  }

  OBJ_STATE stat = STAT_MG_IDLE;
  if (ft == FilterType::STAGED)
  {
    stat = STAT_MG_STAGED;
  }

  int stateId = static_cast<int>(stat);
  sortedModel->setFilterFixedString(QString::number(stateId));
  sortedModel->setFilterKeyColumn(MatchGroupTableModel::STATE_COL_ID);
}

//----------------------------------------------------------------------------

void MatchGroupTableView::clearFilter()
{
  setFilter(FilterType::NONE);
}

//----------------------------------------------------------------------------

void MatchGroupTableView::onFilterUpdateTriggered()
{
  if (currentFilter != FilterType::NONE) setFilter(currentFilter);
}

//----------------------------------------------------------------------------

unique_ptr<MatchGroup> MatchGroupTableView::getSelectedMatchGroup()
{
  // make sure we have non-empty model
  auto mod = model();
  if (mod == nullptr) return nullptr;
  if (mod->rowCount() == 0) return nullptr;

  // make sure we have one item selected
  QModelIndexList indexes = selectionModel()->selection().indexes();
  if (indexes.count() == 0)
  {
    return nullptr;
  }

  // return the selected item
  int selectedSourceRow = sortedModel->mapToSource(indexes.at(0)).row();
  return Tournament::getMatchMngr()->getMatchGroupBySeqNum(selectedSourceRow);
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
    

