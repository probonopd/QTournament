/* 
 * File:   PlayerTableView.cpp
 * Author: volker
 * 
 * Created on March 17, 2014, 8:19 PM
 */

#include "CatTableView.h"
#include "MainFrame.h"
#include <stdexcept>

CategoryTableView::CategoryTableView(QWidget* parent)
:QTableView(parent)
{
  // an empty model for clearing the table when
  // no tournament is open
  emptyModel = new QStringListModel();
  
  connect(MainFrame::getMainFramePointer(), &MainFrame::tournamentOpened, this, &CategoryTableView::onTournamentOpened);
}

//----------------------------------------------------------------------------
    
CategoryTableView::~CategoryTableView()
{
  delete emptyModel;
}

//----------------------------------------------------------------------------
    
void CategoryTableView::onTournamentOpened(Tournament* _tnmt)
{
  tnmt = _tnmt;
  setModel(Tournament::getCategoryTableModel());
  setEnabled(true);
  
  // connect signals from the Tournament and TeamMngr with my slots
  connect(tnmt, &Tournament::tournamentClosed, this, &CategoryTableView::onTournamentClosed);
  
  // tell the rest of the category widget, that the model has changed
  // and that it should update
  emit catModelChanged();
}

//----------------------------------------------------------------------------
    
void CategoryTableView::onTournamentClosed()
{
  // disconnect from all signals, because
  // the sending objects don't exist anymore
  disconnect(tnmt, &Tournament::tournamentClosed, this, &CategoryTableView::onTournamentClosed);
  
  // invalidate the tournament handle and deactivate the view
  tnmt = 0;
  setModel(emptyModel);
  setEnabled(false);
  
  
  // tell the rest of the category widget, that the model has changed
  // and that it should update
  emit catModelChanged();
}

//----------------------------------------------------------------------------

bool CategoryTableView::isEmptyModel()
{
  return (model()->rowCount() == 0);
}

//----------------------------------------------------------------------------

Category CategoryTableView::getSelectedCategory()
{
  QModelIndexList indexes = selectionModel()->selection().indexes();
  if (indexes.count() == 0)
  {
    throw std::invalid_argument("No category selected");
  }
  
  int selectedSeqNum = indexes.at(0).row();
  
  return Tournament::getCatMngr()->getCategoryBySeqNum(selectedSeqNum);
  
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
    

