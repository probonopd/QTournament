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

using namespace QTournament;

TeamTableView::TeamTableView(QWidget* parent)
:QTableView(parent)
{
  // an empty model for clearing the list when
  // no tournament is open
  emptyModel = std::make_unique<QStringListModel>();
  defaultDelegate.reset(itemDelegate());

  // prepare a proxy model to support sorting by columns
  sortedModel = std::make_unique<QSortFilterProxyModel>();
  sortedModel->setSourceModel(emptyModel.get());
  setModel(sortedModel.get());

  // set an initial default sorting column
  sortByColumn(TeamTableModel::NameColId, Qt::AscendingOrder);

  // initiate the model(s) as empty
  setDatabase(nullptr);
}

//----------------------------------------------------------------------------
    
TeamTableView::~TeamTableView()
{
}

//----------------------------------------------------------------------------

void TeamTableView::setDatabase(const TournamentDB* _db)
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
    auto newDataModel = std::make_unique<TeamTableModel>(*_db);
    sortedModel->setSourceModel(newDataModel.release());

    // define a delegate for drawing the category items
    teamItemDelegate = make_unique<TeamItemDelegate>(_db, this);
    teamItemDelegate->setProxy(sortedModel.get());
    setItemDelegate(teamItemDelegate.get());
  } else {
    sortedModel->setSourceModel(emptyModel.get());
    setItemDelegate(defaultDelegate.get());
  }

  // store the new CategoryTableModel instance, if any
  //
  // implicitly delete the old data model, if it was a
  // CategoryTableModel instance
  curDataModel.reset(newDataModel);

  // delete the old selection model
  //delete oldSelectionModel;

  // update the database pointer and set the widget's enabled state
  db = _db;
  setEnabled(db != nullptr);

  // update the column size
  autosizeColumns();
}

//----------------------------------------------------------------------------

std::optional<QTournament::Team> TeamTableView::getSelectedTeam()
{
  // make sure we have a non-empty model
  auto mod = model();
  if (mod == nullptr) return {};
  if (mod->rowCount() == 0) return {};

  // make sure we have one item selected
  QModelIndexList indexes = selectionModel()->selection().indexes();
  if (indexes.count() == 0)
  {
    return {};
  }

  // return the selected item
  TeamMngr tm{*db};
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
  double unitWidth = widthAvail / (1.0 * TotalWidthUnits);
  bool isWidthExceeded = (widthAvail >= TotalColMaxWidth);
  int nameColWidth = isWidthExceeded ? NameColMaxWidth : unitWidth * NameColRelWidth;
  int sizeColWidth = isWidthExceeded ? SizeColMaxWidth : unitWidth * SizeColRelWidth;
  int unregColWidth = isWidthExceeded ? UnregColMaxWidth : unitWidth * UnregColRelWidth;

  // set the column widths
  setColumnWidth(TeamTableModel::NameColId, nameColWidth);
  setColumnWidth(TeamTableModel::MemberCountColId, sizeColWidth);
  setColumnWidth(TeamTableModel::UnregisteredMemberCountColId, unregColWidth);
}

//----------------------------------------------------------------------------

void TeamTableView::onTeamDoubleClicked(const QModelIndex& index)
{
  auto selectedTeam = getSelectedTeam();
  if (!selectedTeam) return;

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
    TeamMngr tm{*db};
    Error e = tm.renameTeam(*selectedTeam, newName);

    if (e == Error::InvalidName)
    {
      QMessageBox::critical(this, tr("Rename team"), tr("The name you entered is invalid (e.g., too long)"));
      isOk = false;
      continue;
    }

    if (e == Error::NameExists)
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
    
