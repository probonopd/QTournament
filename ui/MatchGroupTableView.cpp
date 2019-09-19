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

using namespace QTournament;

MatchGroupTableView::MatchGroupTableView(QWidget* parent)
  :GuiHelpers::AutoSizingTableView_WithDatabase<MatchGroupTableModel>{
     GuiHelpers::AutosizeColumnDescrList{
       {"", CategoryColRelWidth, -1, -1},
       {"", NumericColRelWidth, -1, -1},
       {"", NumericColRelWidth, -1, -1},
       {"", NumericColRelWidth, -1, -1}}, true, parent},
    currentFilter{FilterType::None}
{
}

//----------------------------------------------------------------------------
        
void MatchGroupTableView::setFilter(FilterType ft)
{
  currentFilter = ft;

  if (ft == FilterType::None)
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
  sortedModel->setFilterKeyColumn(MatchGroupTableModel::StateColId);
}

//----------------------------------------------------------------------------

void MatchGroupTableView::clearFilter()
{
  setFilter(FilterType::None);
}

//----------------------------------------------------------------------------

void MatchGroupTableView::onFilterUpdateTriggered()
{
  if (currentFilter != FilterType::None) setFilter(currentFilter);
}

//----------------------------------------------------------------------------

std::optional<QTournament::MatchGroup> MatchGroupTableView::getSelectedMatchGroup()
{
  int srcRow = getSelectedSourceRow();
  if (srcRow < 0) return {};
  MatchMngr mm{*db};
  return mm.getMatchGroupBySeqNum(srcRow);
}

//----------------------------------------------------------------------------

void MatchGroupTableView::hook_onDatabaseOpened()
{
  AutoSizingTableView_WithDatabase::hook_onDatabaseOpened();

  setColumnHidden(MatchGroupTableModel::StateColId, true);  // hide the column containing the internal object state
  setColumnHidden(MatchGroupTableModel::StageSeqColId, true);  // hide the column containing the stage sequence number

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
    

