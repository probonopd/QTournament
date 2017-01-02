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

#ifndef PLAYERTABLEVIEW_H
#define	PLAYERTABLEVIEW_H

#include <memory>

#include <QTableView>
#include <QSortFilterProxyModel>
#include <QStringListModel>

#include "TournamentDB.h"
#include "delegates/PlayerItemDelegate.h"
#include "models/PlayerTableModel.h"

using namespace QTournament;

class PlayerTableView : public QTableView
{
  Q_OBJECT
  
public:
  PlayerTableView (QWidget* parent);
  virtual ~PlayerTableView ();
  unique_ptr<Player> getSelectedPlayer() const;
    
protected:
  static constexpr int REL_NAME_COL_WIDTH = 10;
  static constexpr int REL_SEX_COL_WIDTH = 1;
  static constexpr int REL_TEAM_COL_WIDTH = 10;
  static constexpr int REL_CAT_COL_WIDTH = 7;
  static constexpr int MAX_NAME_COL_WIDTH = 350;
  static constexpr int MAX_SEX_COL_WIDTH = (MAX_NAME_COL_WIDTH / (REL_NAME_COL_WIDTH * 1.0)) * REL_SEX_COL_WIDTH;
  static constexpr int MAX_TEAM_COL_WIDTH = (MAX_NAME_COL_WIDTH / (REL_NAME_COL_WIDTH * 1.0)) * REL_TEAM_COL_WIDTH;
  static constexpr int MAX_CAT_COL_WIDTH = (MAX_NAME_COL_WIDTH / (REL_NAME_COL_WIDTH * 1.0)) * REL_CAT_COL_WIDTH;
  static constexpr int MAX_TOTAL_COL_WIDTH = MAX_NAME_COL_WIDTH + MAX_SEX_COL_WIDTH + MAX_TEAM_COL_WIDTH + MAX_CAT_COL_WIDTH;
  static constexpr int TOTAL_WIDTH_UNITS = REL_NAME_COL_WIDTH + REL_SEX_COL_WIDTH + REL_TEAM_COL_WIDTH + REL_CAT_COL_WIDTH;
  virtual void resizeEvent(QResizeEvent *event) override;
  void autosizeColumns();

public slots:
  void setDatabase(TournamentDB* _db);
  QModelIndex mapToSource(const QModelIndex& proxyIndex);
  void onAddPlayerTriggered();
  void onEditPlayerTriggered();
  void onRemovePlayerTriggered();
  void onShowNextMatchesForPlayerTriggered();
  void onRegisterPlayerTriggered();
  void onUnregisterPlayerTriggered();
  void onImportFromExtDatabase();
  void onExportToExtDatabase();
  void onSyncAllToExtDatabase();

private slots:
  void onContextMenuRequested(const QPoint& pos);
  void onSectionHeaderDoubleClicked();
  
private:
  TournamentDB* db;
  QStringListModel* emptyModel;
  PlayerTableModel* curDataModel;
  QSortFilterProxyModel* sortedModel;
  unique_ptr<PlayerItemDelegate> playerItemDelegate;
  QAbstractItemDelegate* defaultDelegate;

  unique_ptr<QMenu> contextMenu;
  QAction* actAddPlayer;
  QAction* actEditPlayer;
  QAction* actRemovePlayer;
  QAction* actShowNextMatchesForPlayer;
  QAction* actRegister;
  QAction* actUnregister;
  QAction* actImportFromExtDatabase;
  QAction* actExportToExtDatabase;
  QAction* actSyncAllToExtDatabase;

  void initContextMenu();
};

#endif	/* PLAYERTABLEVIEW_H */

