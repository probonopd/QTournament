/* 
 * File:   PlayerTableView.cpp
 * Author: volker
 * 
 * Created on March 17, 2014, 8:19 PM
 */

#include "PlayerTableView.h"
#include "MainFrame.h"

PlayerTableView::PlayerTableView(QWidget* parent)
:QTableView(parent)
{
  // an empty model for clearing the table when
  // no tournament is open
  emptyModel = new QStringListModel();
  
  // prepare a proxy model to support sorting by columns
  sortedModel = new QSortFilterProxyModel();
  sortedModel->setSourceModel(emptyModel);
  setModel(sortedModel);

  connect(MainFrame::getMainFramePointer(), &MainFrame::tournamentOpened, this, &PlayerTableView::onTournamentOpened);
  
  // define a delegate for drawing the player items
  itemDelegate = new PlayerItemDelegate(this);
  itemDelegate->setProxy(sortedModel);
  setItemDelegate(itemDelegate);
  
  
}

//----------------------------------------------------------------------------
    
PlayerTableView::~PlayerTableView()
{
  delete emptyModel;
  delete sortedModel;
  delete itemDelegate;
}

//----------------------------------------------------------------------------
    
void PlayerTableView::onTournamentOpened(Tournament* _tnmt)
{
  tnmt = _tnmt;
  sortedModel->setSourceModel(Tournament::getPlayerTableModel());
  setEnabled(true);
  
  // connect signals from the Tournament and TeamMngr with my slots
  connect(tnmt, &Tournament::tournamentClosed, this, &PlayerTableView::onTournamentClosed);
  
  // resize columns and rows to content once (we do not want permanent automatic resizing)
  horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
  verticalHeader()->resizeSections(QHeaderView::ResizeToContents);
}

//----------------------------------------------------------------------------
    
void PlayerTableView::onTournamentClosed()
{
  // disconnect from all signals, because
  // the sending objects don't exist anymore
  disconnect(tnmt, &Tournament::tournamentClosed, this, &PlayerTableView::onTournamentClosed);
  
  // invalidate the tournament handle and deactivate the view
  tnmt = nullptr;
  sortedModel->setSourceModel(emptyModel);
  setEnabled(false);
  
}

//----------------------------------------------------------------------------

QModelIndex PlayerTableView::mapToSource(const QModelIndex &proxyIndex)
{
  return sortedModel->mapToSource(proxyIndex);
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
    

