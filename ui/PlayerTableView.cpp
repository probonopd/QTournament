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
  
  connect(MainFrame::getMainFramePointer(), &MainFrame::tournamentOpened, this, &PlayerTableView::onTournamentOpened);
}

//----------------------------------------------------------------------------
    
PlayerTableView::~PlayerTableView()
{
  delete emptyModel;
}

//----------------------------------------------------------------------------
    
void PlayerTableView::onTournamentOpened(Tournament* _tnmt)
{
  tnmt = _tnmt;
  setModel(Tournament::getPlayerTableModel());
  setEnabled(true);
  
  // connect signals from the Tournament and TeamMngr with my slots
  connect(tnmt, &Tournament::tournamentClosed, this, &PlayerTableView::onTournamentClosed);
}

//----------------------------------------------------------------------------
    
void PlayerTableView::onTournamentClosed()
{
  // disconnect from all signals, because
  // the sending objects don't exist anymore
  disconnect(tnmt, &Tournament::tournamentClosed, this, &PlayerTableView::onTournamentClosed);
  
  // invalidate the tournament handle and deactivate the view
  tnmt = 0;
  setModel(emptyModel);
  setEnabled(false);
  
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
    

