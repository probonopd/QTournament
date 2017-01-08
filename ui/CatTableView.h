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

#ifndef CATTABLEVIEW_H
#define	CATTABLEVIEW_H

#include <QTableView>
#include <QSortFilterProxyModel>
#include <QStringListModel>
#include <QMenu>

#include "TournamentDB.h"
#include "Category.h"
#include "models/CatTableModel.h"
#include "delegates/CatItemDelegate.h"
#include "AutoSizingTable.h"

using namespace QTournament;

class CategoryTableView : public GuiHelpers::AutoSizingTableView_WithDatabase<CategoryTableModel>
{
  Q_OBJECT
  
public:
  CategoryTableView (QWidget* parent);
  virtual ~CategoryTableView () {}
  Category getSelectedCategory();
  bool hasCategorySelected();

protected:
  void hook_onDatabaseOpened() override;
  
public slots:
  void onCategoryDoubleClicked(const QModelIndex& index);
  void onAddCategory();
  void onRemoveCategory();
  void onRunCategory();
  void onCloneCategory();
  void onAddPlayers();
  void onRemovePlayers();
  void onCreatePlayer();
  void onImportPlayer();
  
private slots:
  void onContextMenuRequested(const QPoint& pos);

signals:
  void catModelChanged();

private:
  CatItemDelegate* catItemDelegate;

  unique_ptr<QMenu> contextMenu;
  QAction* actAddCategory;
  QAction* actCloneCategory;
  QAction* actRunCategory;
  QAction* actRemoveCategory;
  QAction* actAddPlayer;
  QAction* actRemovePlayer;
  QAction* actCreateNewPlayerInCat;
  QAction* actImportPlayerToCat;

  void initContextMenu();

  void handleIntermediateSeedingForSelectedCat();
  bool unfreezeAndCleanup(unique_ptr<Category> selectedCat);

};

#endif	/* PLAYERTABLEVIEW_H */

