/* 
 * File:   GroupAssignmentListWidget.h
 * Author: volker
 *
 * Created on September 3, 2014, 8:06 PM
 */

#ifndef _GROUPASSIGNMENTLISTWIDGET_H
#define	_GROUPASSIGNMENTLISTWIDGET_H

#include "ui_GroupAssignmentListWidget.h"

#include <QListWidget>
#include <QLabel>
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
