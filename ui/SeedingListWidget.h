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


class SeedingListWidget : public QListWidget
{
  Q_OBJECT

public:
  SeedingListWidget(QWidget* parent);
  void initSeedingList(const QTournament::PlayerPairList& _seed);
  ~SeedingListWidget();
  int getSelectedItemIndex() const;
  bool canMoveSelectedPlayerUp() const;
  bool canMoveSelectedPlayerDown() const;
  void shufflePlayers(int fromIndex = -1);
  void moveSelectedPlayerUp();
  void moveSelectedPlayerDown();
  void warpSelectedPlayerTo(int targetRow);
  QTournament::PlayerPairList getSeedList() const;
  void clearListAndFillFromSeed(const QTournament::PlayerPairList& seed);
  void setDatabase(QTournament::TournamentDB* _db);

private:
  QTournament::TournamentDB* db;
  std::unique_ptr<PairItemDelegate> pairDelegate;
  QAbstractItemDelegate* defaultDelegate;
  QQueue<QListWidgetItem*> selectionQueue;
  void swapListItems(int row1, int row2);
};

#endif // SEEDINGLISTWIDGET_H
