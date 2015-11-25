/*
 *    This is QTournament, a badminton tournament management program.
 *    Copyright (C) 2014 - 2015  Volker Knollmann
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

#include <QtWidgets/qwidget.h>
#include <QtWidgets/qmessagebox.h>

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
    
