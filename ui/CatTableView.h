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
#include "SeedingListWidget.h"


class CategoryTableView : public GuiHelpers::AutoSizingTableView_WithDatabase<QTournament::CategoryTableModel>
{
  Q_OBJECT
  
public:
  CategoryTableView (QWidget* parent);
  QTournament::Category getSelectedCategory();
  bool hasCategorySelected();

protected:
  void hook_onDatabaseOpened() override;

  /** \brief Converts a list of player pairs into
   * a list of annotated seed items for the seeding dialog.
   *
   * WE DON'T FILL IN THE GROUP HINT HERE!
   *
   * \returns a list of AnnotatedSeedingEntrys
   */
  std::vector<SeedingListWidget::AnnotatedSeedEntry> pp2Annotated(const QTournament::PlayerPairList& ppList);

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

  std::unique_ptr<QMenu> contextMenu;
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
  bool unfreezeAndCleanup(const QTournament::Category& selectedCat);

};

#endif	/* PLAYERTABLEVIEW_H */

