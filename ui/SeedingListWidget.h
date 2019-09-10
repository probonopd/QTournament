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

#ifndef SEEDINGLISTWIDGET_H
#define SEEDINGLISTWIDGET_H

#include <memory>

#include <QObject>
#include <QWidget>
#include <QListWidget>
#include <QQueue>

#include "PlayerPair.h"
#include "delegates/PairItemDelegate.h"
#include "TournamentDB.h"
#include "AutoSizingTable.h"


class SeedingListWidget : public GuiHelpers::AutoSizingTableWidget
{
  Q_OBJECT

public:
  static constexpr int IdxColName = 0;
  static constexpr int IdxColGroup = 1;
  static constexpr int RowHeight = 60;

  struct AnnotatedSeedEntry
  {
    int playerPairId;
    QString pairName;
    QString teamName;
    QString groupHint;
  };

  SeedingListWidget(QWidget* parent);
  ~SeedingListWidget();
  int getSelectedItemIndex() const;
  bool canMoveSelectedPlayerUp() const;
  bool canMoveSelectedPlayerDown() const;
  void shufflePlayers(int fromIndex = -1);
  void moveSelectedPlayerUp();
  void moveSelectedPlayerDown();
  void warpSelectedPlayerTo(int targetRow);
  std::vector<int> getSeedList() const;

  /** \brief Replaces all widget contents with a new initial
   * seeding content
   */
  void clearListAndFillFromSeed(const std::vector<AnnotatedSeedEntry>& seed   ///< the list of player pairs to display in the widget
      );

private:
  QQueue<QListWidgetItem*> selectionQueue;
  void swapListItems(int row1, int row2);
  std::unique_ptr<PairItemDelegate> pairColumnDelegate;
  std::vector<AnnotatedSeedEntry> seedList;
};

#endif // SEEDINGLISTWIDGET_H
