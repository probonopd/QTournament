/* 
 * File:   PlayerTableView.cpp
 * Author: volker
 * 
 * Created on March 17, 2014, 8:19 PM
 */

#include "MatchTableView.h"
#include "MainFrame.h"
#include <QMessageBox>

MatchTableView::MatchTableView(QWidget* parent)
  :QTableView(parent)
{
  // an empty model for clearing the table when
  // no tournament is open
  emptyModel = new QStringListModel();

  // prepare a proxy model to support sorting by columns
  sortedModel = new QSortFilterProxyModel();
  sortedModel->setSourceModel(emptyModel);
  setModel(sortedModel);

  connect(MainFrame::getMainFramePointer(), &MainFrame::tournamentOpened, this, &MatchTableView::onTournamentOpened);
  
  // react on selection changes in the match table view
  connect(selectionModel(),
    SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
    SLOT(onSelectionChanged(const QItemSelection&, const QItemSelection&)));

  // define a delegate for drawing the match items
  itemDelegate = new MatchItemDelegate(this);
  itemDelegate->setProxy(sortedModel);
  setItemDelegate(itemDelegate);
}

//----------------------------------------------------------------------------
    
MatchTableView::~MatchTableView()
{
  delete emptyModel;
  delete sortedModel;
  delete itemDelegate;
}

//----------------------------------------------------------------------------
    
void MatchTableView::onTournamentOpened(Tournament* _tnmt)
{
  tnmt = _tnmt;
  sortedModel->setSourceModel(Tournament::getMatchTableModel());
  setColumnHidden(MatchTableModel::STATE_COL_ID, true);  // hide the column containing the internal object state
  sortedModel->setFilterRegExp("^[0-9]{1,4}$");  // matches "0" to "9999"
  sortedModel->setFilterKeyColumn(MatchTableModel::MATCH_NUM_COL_ID);
  sortedModel->sort(MatchTableModel::MATCH_NUM_COL_ID, Qt::AscendingOrder);
  setEnabled(true);
  
  // connect signals from the Tournament and TeamMngr with my slots
  connect(tnmt, &Tournament::tournamentClosed, this, &MatchTableView::onTournamentClosed, Qt::DirectConnection);

  // resize columns and rows to content once (we do not want permanent automatic resizing)
  horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
  verticalHeader()->resizeSections(QHeaderView::ResizeToContents);

  horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
}

//----------------------------------------------------------------------------
    
void MatchTableView::onTournamentClosed()
{
  // disconnect from all signals, because
  // the sending objects don't exist anymore
  disconnect(tnmt, &Tournament::tournamentClosed, this, &MatchTableView::onTournamentClosed);
  
  // invalidate the tournament handle and deactivate the view
  tnmt = nullptr;
  sortedModel->setSourceModel(emptyModel);
  setEnabled(false);
  
}

//----------------------------------------------------------------------------

unique_ptr<Match> MatchTableView::getSelectedMatch()
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
  return Tournament::getMatchMngr()->getMatchBySeqNum(selectedSourceRow);
}

//----------------------------------------------------------------------------

void MatchTableView::onSelectionChanged(const QItemSelection& selectedItem, const QItemSelection& deselectedItem)
{
  resizeRowsToContents();
  for (auto item : selectedItem)
  {
    itemDelegate->setSelectedRow(item.top());
    resizeRowToContents(item.top());
  }
  for (auto item : deselectedItem)
  {
    resizeRowToContents(item.top());
  }
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
    

