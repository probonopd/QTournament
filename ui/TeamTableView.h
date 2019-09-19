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
  virtual ~TeamTableView() override;
  void setDatabase(const QTournament::TournamentDB* _db);
  std::optional<QTournament::Team> getSelectedTeam();
  
protected:
  static constexpr int NameColRelWidth = 10;
  static constexpr int SizeColRelWidth = 2;
  static constexpr int UnregColRelWidth = 2;
  static constexpr int NameColMaxWidth = 350;
  static constexpr int SizeColMaxWidth = (NameColMaxWidth / (NameColRelWidth * 1.0)) * SizeColRelWidth;
  static constexpr int UnregColMaxWidth = (NameColMaxWidth / (NameColRelWidth * 1.0)) * UnregColRelWidth;
  static constexpr int TotalColMaxWidth = NameColMaxWidth + SizeColMaxWidth + UnregColMaxWidth;
  static constexpr int TotalWidthUnits = NameColRelWidth + SizeColRelWidth + UnregColRelWidth;
  virtual void resizeEvent(QResizeEvent *event) override;
  void autosizeColumns();

public slots:
  void onTeamDoubleClicked(const QModelIndex& index);
  QModelIndex mapToSource(const QModelIndex& proxyIndex);

private:
  const QTournament::TournamentDB* db{nullptr};
  std::unique_ptr<QStringListModel> emptyModel;
  std::unique_ptr<QTournament::TeamTableModel> curDataModel;
  std::unique_ptr<QSortFilterProxyModel> sortedModel;
  std::unique_ptr<TeamItemDelegate> teamItemDelegate;
  std::unique_ptr<QAbstractItemDelegate> defaultDelegate;

};

#endif	/* TEAMLISTVIEW_H */

