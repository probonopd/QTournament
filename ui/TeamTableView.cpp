/*
 *    This is QTournament, a badminton tournament management program.
 *    Copyright (C) 2014 - 2017  Volker Knollmann
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
#include <QScrollBar>
#include <QInputDialog>

#include "TeamTableView.h"
#include "MainFrame.h"
#include "TeamMngr.h"

TeamTableView::TeamTableView(QWidget* parent)
:QTableView(parent), db(nullptr), curDataModel(nullptr), teamItemDelegate(nullptr)
{
  // an empty model for clearing the list when
  // no tournament is open
  emptyModel = new QStringListModel();
  defaultDelegate = itemDelegate();

  // prepare a proxy model to support sorting by columns
  sortedModel = new QSortFilterProxyModel();
  sortedModel->setSourceModel(emptyModel);
  setModel(sortedModel);

  // set an initial default sorting column
  sortByColumn(TeamTableModel::NAME_COL_ID, Qt::AscendingOrder);

  // initiate the model(s) as empty
  setDatabase(nullptr);
}

//----------------------------------------------------------------------------
    
TeamTableView::~TeamTableView()
{
  delete emptyModel;
  delete sortedModel;
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
  //QItemSelectionModel *oldSelectionModel = selectionModel();

  // set the new data model
  TeamTableModel* newDataModel = nullptr;
  if (_db != nullptr)
  {
    newDataModel = new TeamTableModel(_db);
    sortedModel->setSourceModel(newDataModel);

    // define a delegate for drawing the category items
    teamItemDelegate = make_unique<TeamItemDelegate>(_db, this);
    teamItemDelegate->setProxy(sortedModel);
    setItemDelegate(teamItemDelegate.get());
  } else {
    sortedModel->setSourceModel(emptyModel);
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
  //delete oldSelectionModel;

  // update the database pointer and set the widget's enabled state
  db = _db;
  setEnabled(db != nullptr);

  // update the column size
  autosizeColumns();
}

//----------------------------------------------------------------------------

unique_ptr<Team> TeamTableView::getSelectedTeam()
{
  // make sure we have a non-empty model
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
  TeamMngr tm{db};
  int selectedSourceRow = sortedModel->mapToSource(indexes.at(0)).row();
  return tm.getTeamBySeqNum2(selectedSourceRow);
}

//----------------------------------------------------------------------------

void TeamTableView::resizeEvent(QResizeEvent* event)
{
  // call parent function
  QTableView::resizeEvent(event);

  autosizeColumns();

  // finish event processing
  event->accept();
}

//----------------------------------------------------------------------------

void TeamTableView::autosizeColumns()
{
  // distribute the available space over the columns but
  // set a maximum to prevent too wide name fields
  int widthAvail = width();
  if ((verticalScrollBar() != nullptr) && (verticalScrollBar()->isVisible()))
  {
    widthAvail -= verticalScrollBar()->width();
  }
  double unitWidth = widthAvail / (1.0 * TOTAL_WIDTH_UNITS);
  bool isWidthExceeded = (widthAvail >= MAX_TOTAL_COL_WIDTH);
  int nameColWidth = isWidthExceeded ? MAX_NAME_COL_WIDTH : unitWidth * REL_NAME_COL_WIDTH;
  int sizeColWidth = isWidthExceeded ? MAX_SIZE_COL_WIDTH : unitWidth * REL_SIZE_COL_WIDTH;
  int unregColWidth = isWidthExceeded ? MAX_UNREG_COL_WIDTH : unitWidth * REL_UNREG_COL_WIDTH;

  // set the column widths
  setColumnWidth(TeamTableModel::NAME_COL_ID, nameColWidth);
  setColumnWidth(TeamTableModel::MEMBER_COUNT_COL_ID, sizeColWidth);
  setColumnWidth(TeamTableModel::UNREGISTERED_MEMBER_COUNT_COL_ID, unregColWidth);
}

//----------------------------------------------------------------------------

void TeamTableView::onTeamDoubleClicked(const QModelIndex& index)
{
  unique_ptr<Team> selectedTeam = getSelectedTeam();
  if (selectedTeam == nullptr) return;

  QString oldName = selectedTeam->getName();

  bool isOk = false;
  while (!isOk)
  {
    QString newName = QInputDialog::getText(this, tr("Rename team"), tr("Enter new team name:"),
      QLineEdit::Normal, oldName, &isOk);

    if (!isOk)
    {
      return;  // the user hit cancel
    }

    if (newName.isEmpty())
    {
      QMessageBox::critical(this, tr("Rename team"), tr("The new name may not be empty!"));
      isOk = false;
      continue;
    }

    if (oldName == newName)
    {
      return;
    }

    // okay, we have a valid name. try to rename the team
    newName = newName.trimmed();
    TeamMngr tm{db};
    ERR e = tm.renameTeam(*selectedTeam, newName);

    if (e == INVALID_NAME)
    {
      QMessageBox::critical(this, tr("Rename team"), tr("The name you entered is invalid (e.g., too long)"));
      isOk = false;
      continue;
    }

    if (e == NAME_EXISTS)
    {
      QMessageBox::critical(this, tr("Rename team"), tr("A team of this name already exists"));
      isOk = false;
      continue;
    }
  }
}

//----------------------------------------------------------------------------

QModelIndex TeamTableView::mapToSource(const QModelIndex& proxyIndex)
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
    
