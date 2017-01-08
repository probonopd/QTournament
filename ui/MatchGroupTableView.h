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

#ifndef MATCHGROUPTABLEVIEW_H
#define	MATCHGROUPTABLEVIEW_H

#include <memory>

#include <QTableView>
#include <QSortFilterProxyModel>
#include <QStringListModel>

#include "TournamentDB.h"
#include "models/MatchGroupTabModel.h"
#include "AutoSizingTable.h"

using namespace QTournament;

class MatchGroupTableView : public GuiHelpers::AutoSizingTableView_WithDatabase<MatchGroupTableModel>
{
  Q_OBJECT
  
public:
  enum class FilterType : std::int8_t { IDLE = 1, STAGED = 2, NONE = 0 };

  MatchGroupTableView (QWidget* parent);
  virtual ~MatchGroupTableView () {}
  void setFilter(FilterType ft);
  void clearFilter();
  unique_ptr<MatchGroup> getSelectedMatchGroup();
  
protected:
  static constexpr int REL_NUMERIC_COL_WIDTH = 2;
  static constexpr int REL_CATEGORY_COL_WIDTH = 3;

  void hook_onDatabaseOpened() override;

public slots:
  void onFilterUpdateTriggered();
  
private:
  FilterType currentFilter;

};

#endif	/* MATCHGROUPTABLEVIEW_H */

