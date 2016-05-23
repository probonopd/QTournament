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

#include <QScrollBar>

#include "MatchGroupTableView.h"
#include "MainFrame.h"
#include "MatchMngr.h"

MatchGroupTableView::MatchGroupTableView(QWidget* parent)
  :QTableView(parent), db(nullptr), curDataModel(nullptr), currentFilter(FilterType::NONE)
{
  // an empty model for clearing the table when
  // no tournament is open
  emptyModel = new QStringListModel();

  // prepare a proxy model to support sorting by columns
  sortedModel = new QSortFilterProxyModel();
  sortedModel->setSourceModel(emptyModel);
  setModel(sortedModel);

  // initiate the model(s) as empty
  setDatabase(nullptr);
}

//----------------------------------------------------------------------------
    
MatchGroupTableView::~MatchGroupTableView()
{
  delete emptyModel;
  delete sortedModel;
  //delete itemDelegate;
  if (curDataModel != nullptr) delete curDataModel;
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
  MatchMngr mm{db};
  return mm.getMatchGroupBySeqNum(selectedSourceRow);
}

//----------------------------------------------------------------------------
    
void MatchGroupTableView::setDatabase(TournamentDB* _db)
{
  // set the new data model
  MatchGroupTableModel* newDataModel = nullptr;
  if (_db != nullptr)
  {
    newDataModel = new MatchGroupTableModel(_db);
    sortedModel->setSourceModel(newDataModel);
    setColumnHidden(MatchGroupTableModel::STATE_COL_ID, true);  // hide the column containing the internal object state
    setColumnHidden(MatchGroupTableModel::STAGE_SEQ_COL_ID, true);  // hide the column containing the stage sequence number

    // receive triggers from the underlying models when a filter update is necessary
    connect(newDataModel, SIGNAL(triggerFilterUpdate()), this, SLOT(onFilterUpdateTriggered()), Qt::DirectConnection);
  } else {
    sortedModel->setSourceModel(emptyModel);
  }

  // delete the old data model, if it was a
  // CategoryTableModel instance
  if (curDataModel != nullptr)
  {
    delete curDataModel;
  }

  // store the new CategoryTableModel instance, if any
  curDataModel = newDataModel;

  // update the database pointer and set the widget's enabled state
  db = _db;
  setEnabled(db != nullptr);

  // initialize column widths
  autosizeColumns();
}

//----------------------------------------------------------------------------

void MatchGroupTableView::resizeEvent(QResizeEvent *event)
{
  // call parent function
  QTableView::resizeEvent(event);

  // resize all columns
  autosizeColumns();

  // finish event processing
  event->accept();
}

//----------------------------------------------------------------------------

void MatchGroupTableView::autosizeColumns()
{
  // distribute the available space according to relative column widths
  int totalUnits = (REL_CATEGORY_COL_WIDTH + // category name
                    REL_NUMERIC_COL_WIDTH + // round
                    REL_NUMERIC_COL_WIDTH + // group
                    REL_NUMERIC_COL_WIDTH); // match count

  // calc the available width
  int widthAvail = width();
  if ((verticalScrollBar() != nullptr) && (verticalScrollBar()->isVisible()))
  {
    widthAvail -= verticalScrollBar()->width();
  }
  double unitWidth = widthAvail / (1.0 * totalUnits);

  // a little lambda that sets the column width and
  // aggregates it in a dedicated local variable
  int usedWidth = 0;
  auto myWidthSetter = [&](int colId, int newColWidth) {
    setColumnWidth(colId, newColWidth);
    usedWidth += newColWidth;
  };

  myWidthSetter(0, REL_CATEGORY_COL_WIDTH * unitWidth);
  myWidthSetter(1, REL_NUMERIC_COL_WIDTH * unitWidth);
  myWidthSetter(2, REL_NUMERIC_COL_WIDTH * unitWidth);

  // assign the remaining width to the last column. This accounts for
  // rounding errors when dividing / multiplying pixel widths and makes
  // that we always used the full width of the widget
  myWidthSetter(3, widthAvail - usedWidth);  // referee
}

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

