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

#ifndef TEAMLISTVIEW_H
#define	TEAMLISTVIEW_H

#include <QTableView>
#include <QStringListModel>
#include <QSortFilterProxyModel>

#include "TournamentDB.h"
#include "models/TeamTableModel.h"
#include "ui/delegates/TeamItemDelegate.h"
#include "Team.h"

class TeamTableView : public QTableView
{
  Q_OBJECT
  
public:
  TeamTableView (QWidget* parent);
  virtual ~TeamTableView();
  void setDatabase(QTournament::TournamentDB* _db);
  std::optional<QTournament::Team> getSelectedTeam();
  
protected:
  static constexpr int REL_NAME_COL_WIDTH = 10;
  static constexpr int REL_SIZE_COL_WIDTH = 2;
  static constexpr int REL_UNREG_COL_WIDTH = 2;
  static constexpr int MAX_NAME_COL_WIDTH = 350;
  static constexpr int MAX_SIZE_COL_WIDTH = (MAX_NAME_COL_WIDTH / (REL_NAME_COL_WIDTH * 1.0)) * REL_SIZE_COL_WIDTH;
  static constexpr int MAX_UNREG_COL_WIDTH = (MAX_NAME_COL_WIDTH / (REL_NAME_COL_WIDTH * 1.0)) * REL_UNREG_COL_WIDTH;
  static constexpr int MAX_TOTAL_COL_WIDTH = MAX_NAME_COL_WIDTH + MAX_SIZE_COL_WIDTH + MAX_UNREG_COL_WIDTH;
  static constexpr int TOTAL_WIDTH_UNITS = REL_NAME_COL_WIDTH + REL_SIZE_COL_WIDTH + REL_UNREG_COL_WIDTH;
  virtual void resizeEvent(QResizeEvent *event) override;
  void autosizeColumns();

public slots:
  void onTeamDoubleClicked(const QModelIndex& index);
  QModelIndex mapToSource(const QModelIndex& proxyIndex);

private:
  QTournament::TournamentDB* db;
  QStringListModel* emptyModel;
  QTournament::TeamTableModel* curDataModel;
  QSortFilterProxyModel* sortedModel;
  std::unique_ptr<TeamItemDelegate> teamItemDelegate;
  QAbstractItemDelegate* defaultDelegate;

};

#endif	/* TEAMLISTVIEW_H */

