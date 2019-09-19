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

#ifndef GROUPASSIGNMENTLISTWIDGET_H
#define	GROUPASSIGNMENTLISTWIDGET_H

#include "ui_GroupAssignmentListWidget.h"

#include <QListWidget>
#include <QLabel>
#include <QList>
#include <QQueue>

#include "TournamentDB.h"
#include "PlayerPair.h"
#include "delegates/PairItemDelegate.h"

class GroupAssignmentListWidget : public QWidget
{
  Q_OBJECT
public:
  GroupAssignmentListWidget(QWidget* parent = nullptr);
  virtual ~GroupAssignmentListWidget();
  
  void setup(const QTournament::TournamentDB* _db, const QList<QTournament::PlayerPairList>& ppListList);
  void teardown();
  QTournament::PlayerPairList getSelectedPlayerPairs();
  void swapSelectedPlayers();
  std::vector<QTournament::PlayerPairList> getGroupAssignments();
  void setDatabase(const QTournament::TournamentDB* _db);

public slots:
  void onRowSelectionChanged();
  
private:
  static constexpr int MinPlayerListWidth = 200;

  Ui::GroupAssignmentListWidget ui;
  const QTournament::TournamentDB* db{nullptr};
  QListWidget* lwGroup[QTournament::MaxGroupCount];
  QLabel* laGroup[QTournament::MaxGroupCount];
  std::unique_ptr<PairItemDelegate> delegate[QTournament::MaxGroupCount];
  bool isInitialized;
  int getColCountForGroupCount(int grpCount);
  QQueue<QListWidget*> selectionQueue;
  bool inhibitItemChangedSig;
} ;

#endif	/* _GROUPASSIGNMENTLISTWIDGET_H */
