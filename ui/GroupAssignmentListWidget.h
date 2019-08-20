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

#ifndef _GROUPASSIGNMENTLISTWIDGET_H
#define	_GROUPASSIGNMENTLISTWIDGET_H

#include "ui_GroupAssignmentListWidget.h"

#include <QListWidget>
#include <QLabel>
#include <QList>
#include <QQueue>

#include "TournamentDB.h"
#include "PlayerPair.h"
#include "delegates/PairItemDelegate.h"

#define MIN_PLAYER_LIST_WIDTH 200

class GroupAssignmentListWidget : public QWidget
{
  Q_OBJECT
public:
  GroupAssignmentListWidget(QWidget* parent = nullptr);
  virtual ~GroupAssignmentListWidget();
  
  void setup(QTournament::TournamentDB* _db, QList<QTournament::PlayerPairList> ppListList);
  void teardown();
  QTournament::PlayerPairList getSelectedPlayerPairs();
  void swapSelectedPlayers();
  std::vector<QTournament::PlayerPairList> getGroupAssignments();
  void setDatabase(QTournament::TournamentDB* _db);

public slots:
  void onRowSelectionChanged();
  
private:
  Ui::GroupAssignmentListWidget ui;
  QTournament::TournamentDB* db;
  QListWidget* lwGroup[MaxGroupCount];
  QLabel* laGroup[MaxGroupCount];
  std::unique_ptr<PairItemDelegate> delegate[MaxGroupCount];
  bool isInitialized;
  int getColCountForGroupCount(int grpCount);
  QQueue<QListWidget*> selectionQueue;
  bool inhibitItemChangedSig;
} ;

#endif	/* _GROUPASSIGNMENTLISTWIDGET_H */
