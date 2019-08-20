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

#include <QScrollBar>

#include "MatchGroupTableView.h"
#include "MainFrame.h"
#include "MatchMngr.h"

MatchGroupTableView::MatchGroupTableView(QWidget* parent)
  :GuiHelpers::AutoSizingTableView_WithDatabase<MatchGroupTableModel>{
     GuiHelpers::AutosizeColumnDescrList{
       {"", REL_CATEGORY_COL_WIDTH, -1, -1},
       {"", REL_NUMERIC_COL_WIDTH, -1, -1},
       {"", REL_NUMERIC_COL_WIDTH, -1, -1},
       {"", REL_NUMERIC_COL_WIDTH, -1, -1}}, true, parent},
    currentFilter{FilterType::RefereeMode::None}
{
}

//----------------------------------------------------------------------------
        
void MatchGroupTableView::setFilter(FilterType ft)
{
  currentFilter = ft;

  if (ft == FilterType::RefereeMode::None)
  {
    sortedModel->setFilterFixedString("");
    return;
  }

  ObjState stat = ObjState::MG_Idle;
  if (ft == FilterType::STAGED)
  {
    stat = ObjState::MG_Staged;
  }

  int stateId = static_cast<int>(stat);
  sortedModel->setFilterFixedString(QString::number(stateId));
  sortedModel->setFilterKeyColumn(MatchGroupTableModel::STATE_COL_ID);
}

//----------------------------------------------------------------------------

void MatchGroupTableView::clearFilter()
{
  setFilter(FilterType::RefereeMode::None);
}

//----------------------------------------------------------------------------

void MatchGroupTableView::onFilterUpdateTriggered()
{
  if (currentFilter != FilterType::RefereeMode::None) setFilter(currentFilter);
}

//----------------------------------------------------------------------------

std::optional<QTournament::MatchGroup> MatchGroupTableView::getSelectedMatchGroup()
{
  int srcRow = getSelectedSourceRow();
  if (srcRow < 0) return nullptr;
  MatchMngr mm{db};
  return mm.getMatchGroupBySeqNum(srcRow);
}

//----------------------------------------------------------------------------

void MatchGroupTableView::hook_onDatabaseOpened()
{
  AutoSizingTableView_WithDatabase::hook_onDatabaseOpened();

  setColumnHidden(MatchGroupTableModel::STATE_COL_ID, true);  // hide the column containing the internal object state
  setColumnHidden(MatchGroupTableModel::STAGE_SEQ_COL_ID, true);  // hide the column containing the stage sequence number

  // receive triggers from the underlying models when a filter update is necessary
  connect(customDataModel.get(), SIGNAL(triggerFilterUpdate()), this, SLOT(onFilterUpdateTriggered()), Qt::DirectConnection);
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
    

