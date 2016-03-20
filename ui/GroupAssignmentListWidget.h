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

#ifndef _GROUPASSIGNMENTLISTWIDGET_H
#define	_GROUPASSIGNMENTLISTWIDGET_H

#include "ui_GroupAssignmentListWidget.h"

#include <QListWidget>
#include <QLabel>
#include <QList>
#include <QQueue>

#include "Tournament.h"
#include "PlayerPair.h"
#include "delegates/PairItemDelegate.h"

#define MIN_PLAYER_LIST_WIDTH 200

using namespace QTournament;

class GroupAssignmentListWidget : public QWidget
{
  Q_OBJECT
public:
  GroupAssignmentListWidget(QWidget* parent = 0);
  virtual ~GroupAssignmentListWidget();
  
  void setup(QList<PlayerPairList> ppListList);
  void teardown();
  PlayerPairList getSelectedPlayerPairs();
  void swapSelectedPlayers();
  vector<PlayerPairList> getGroupAssignments();

public slots:
  void onRowSelectionChanged();
  
private:
  Ui::GroupAssignmentListWidget ui;
  QListWidget* lwGroup[MAX_GROUP_COUNT];
  QLabel* laGroup[MAX_GROUP_COUNT];
  PairItemDelegate* delegate[MAX_GROUP_COUNT];
  bool isInitialized;
  int getColCountForGroupCount(int grpCount);
  QQueue<QListWidget*> selectionQueue;
  bool inhibitItemChangedSig;
} ;

#endif	/* _GROUPASSIGNMENTLISTWIDGET_H */
