/*
 *    This is QTournament, a badminton tournament management program.
 *    Copyright (C) 2014 - 2016  Volker Knollmann
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

#include <QWidget>
#include <QMessageBox>

#include "TeamTableView.h"
#include "MainFrame.h"

TeamTableView::TeamTableView(QWidget* parent)
:QTableView(parent), db(nullptr), curDataModel(nullptr), teamItemDelegate(nullptr)
{
  // an empty model for clearing the list when
  // no tournament is open
  emptyModel = new QStringListModel();
  defaultDelegate = itemDelegate();

  // initiate the model(s) as empty
  setDatabase(nullptr);
}

//----------------------------------------------------------------------------
    
TeamTableView::~TeamTableView()
{
  delete emptyModel;
  if (curDataModel != nullptr) delete curDataModel;
  if (defaultDelegate != nullptr) delete defaultDelegate;
}

//----------------------------------------------------------------------------

void TeamTableView::setDatabase(TournamentDB* _db)
{
  // According to the Qt documentation, the selection model
  // has to be explicitly deleted by the user
  //
  // Thus we store the model pointer for later deletion
  QItemSelectionModel *oldSelectionModel = selectionModel();

  // set the new data model
  TeamTableModel* newDataModel = nullptr;
  if (_db != nullptr)
  {
    newDataModel = new TeamTableModel(_db);
    setModel(newDataModel);

    // define a delegate for drawing the category items
    teamItemDelegate = make_unique<TeamItemDelegate>(_db, this);
    //teamItemDelegate->setProxy(sortedModel);
    setItemDelegate(teamItemDelegate.get());
  } else {
    setModel(emptyModel);
    setItemDelegate(defaultDelegate);
  }

  // delete the old data model, if it was a
  // CategoryTableModel instance
  if (curDataModel != nullptr)
  {
    delete curDataModel;
  }

  // store the new CategoryTableModel instance, if any
  curDataModel = newDataModel;

  // delete the old selection model
  delete oldSelectionModel;

  // update the database pointer and set the widget's enabled state
  db = _db;
  setEnabled(db != nullptr);
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
    
