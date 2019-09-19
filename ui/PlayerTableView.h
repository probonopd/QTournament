/*
 *    This is QTournament, a badminton tournament management program.
 *    Copyright (C) 2014 - 2019  Volker Knollmann
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
#include <QMenu>

#include "TournamentDB.h"
#include "delegates/PlayerItemDelegate.h"
#include "models/PlayerTableModel.h"
#include "AutoSizingTable.h"

class PlayerTableView : public GuiHelpers::AutoSizingTableView_WithDatabase<QTournament::PlayerTableModel>
{
  Q_OBJECT
  
public:
  PlayerTableView (QWidget* parent);
  std::optional<QTournament::Player> getSelectedPlayer() const;
    
protected:
  static constexpr int NameColRelWidth = 10;
  static constexpr int SexColRelWidth = 1;
  static constexpr int TeamColRelWidth = 10;
  static constexpr int CatColRelWidth = 7;
  static constexpr int NameColMaxWidth = 350;
  static constexpr int SexColMaxWidth = 30;
  static constexpr int TeamColMaxWidth = 300;
  static constexpr int CatColMaxWidth = 300;
  void hook_onDatabaseOpened() override;

public slots:
  void onAddPlayerTriggered();
  void onEditPlayerTriggered();
  void onPlayerDoubleCLicked();
  void onShowPlayerInfoTriggered();
  void onRemovePlayerTriggered();
  void onShowNextMatchesForPlayerTriggered();
  void onRegisterPlayerTriggered();
  void onUnregisterPlayerTriggered();
  void onImportFromExtDatabase();
  void onExportToExtDatabase();
  void onSyncAllToExtDatabase();
  void onShowPlayerProfile();

private slots:
  void onContextMenuRequested(const QPoint& pos);
  void onSectionHeaderDoubleClicked();
  
private:
  PlayerItemDelegate* playerItemDelegate;

  std::unique_ptr<QMenu> contextMenu;
  QAction* actAddPlayer;
  QAction* actEditPlayer;
  QAction* actRemovePlayer;
  QAction* actShowNextMatchesForPlayer;
  QAction* actRegister;
  QAction* actUnregister;
  QAction* actImportFromExtDatabase;
  QAction* actExportToExtDatabase;
  QAction* actSyncAllToExtDatabase;
  QAction* actShowPlayerProfile;

  void initContextMenu();
};

#endif	/* PLAYERTABLEVIEW_H */

