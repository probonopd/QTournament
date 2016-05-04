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

#ifndef MATCHGROUPTABLEVIEW_H
#define	MATCHGROUPTABLEVIEW_H

#include <memory>

#include <QTableView>
#include <QSortFilterProxyModel>
#include <QStringListModel>

#include "TournamentDB.h"
#include "models/MatchGroupTabModel.h"
//#include "delegates/PlayerItemDelegate.h"

using namespace QTournament;

class MatchGroupTableView : public QTableView
{
  Q_OBJECT
  
public:
  enum class FilterType : std::int8_t { IDLE = 1, STAGED = 2, NONE = 0 };

  MatchGroupTableView (QWidget* parent);
  virtual ~MatchGroupTableView ();
  void setFilter(FilterType ft);
  void clearFilter();
  unique_ptr<MatchGroup> getSelectedMatchGroup();
  void setDatabase(TournamentDB* _db);
  
public slots:
  void onFilterUpdateTriggered();
  
private:
  TournamentDB* db;
  MatchGroupTableModel* curDataModel;
  QStringListModel* emptyModel;
  QSortFilterProxyModel* sortedModel;
  //PlayerItemDelegate* itemDelegate;
  FilterType currentFilter;

};

#endif	/* MATCHGROUPTABLEVIEW_H */

