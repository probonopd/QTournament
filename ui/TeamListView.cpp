/* 
 * File:   TeamListView.cpp
 * Author: volker
 * 
 * Created on March 15, 2014, 7:28 PM
 */

#include <qt/QtWidgets/qwidget.h>
#include <qt/QtWidgets/qmessagebox.h>

#include "TeamListView.h"
#include "MainFrame.h"

TeamListView::TeamListView(QWidget* parent)
:QListView(parent)
{
  // an empty model for clearing the list when
  // no tournament is open
  emptyModel = new QStringListModel();
  
  connect(MainFrame::getMainFramePointer(), &MainFrame::tournamentOpened, this, &TeamListView::onTournamentOpened);
}

//----------------------------------------------------------------------------
    
TeamListView::~TeamListView()
{
  delete emptyModel;
}

//----------------------------------------------------------------------------
    
void TeamListView::onTournamentClosed()
{
  // disconnect from all signals, because
  // the sending objects don't exist anymore
  disconnect(tnmt, &Tournament::tournamentClosed, this, &TeamListView::onTournamentClosed);
  
  // invalidate the tournament handle and deactivate the view
  tnmt = 0;
  setModel(emptyModel);
  setEnabled(false);
  
}

//----------------------------------------------------------------------------
    
void TeamListView::onTournamentOpened(Tournament* _tnmt)
{
  tnmt = _tnmt;
  setModel(Tournament::getTeamListModel());
  setEnabled(true);
  
  // connect signals from the Tournament and TeamMngr with my slots
  connect(tnmt, &Tournament::tournamentClosed, this, &TeamListView::onTournamentClosed);
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
    
