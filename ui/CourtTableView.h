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

#ifndef COURTTABLEVIEW_H
#define	COURTTABLEVIEW_H

#include <memory>

#include <QTableView>
#include <QSortFilterProxyModel>
#include <QStringListModel>

#include "TournamentDB.h"
#include "delegates/CourtItemDelegate.h"
#include "Match.h"
#include "models/CourtTabModel.h"
#include "AutoSizingTable.h"

using namespace QTournament;

class CourtTableView : public GuiHelpers::AutoSizingTableView_WithDatabase<CourtTableModel>
{
  Q_OBJECT
  
public:
  CourtTableView (QWidget* parent);
  virtual ~CourtTableView ();
  unique_ptr<Court> getSelectedCourt() const;
  unique_ptr<Match> getSelectedMatch() const;

protected:
  static constexpr int ABS_COURT_COL_WIDTH = 40;
  static constexpr int ABS_DURATION_COL_WIDTH = 60;

  void hook_onDatabaseOpened() override;

private slots:
  void onSelectionChanged(const QItemSelection&selectedItem, const QItemSelection&deselectedItem);
  void onContextMenuRequested(const QPoint& pos);
  void onActionAddCourtTriggered();
  void onWalkoverP1Triggered();
  void onWalkoverP2Triggered();
  void onActionUndoCallTriggered();
  void onActionAddCallTriggered();
  void onActionSwapRefereeTriggered();
  void onSectionHeaderDoubleClicked();
  void onActionToggleMatchAssignmentModeTriggered();
  void onActionToogleEnableStateTriggered();
  void onActionDeleteCourtTriggered();
  void onReprintResultSheetTriggered();

private:
  static constexpr int MAX_NUM_ADD_CALL = 3;
  CourtItemDelegate* courtItemDelegate;

  unique_ptr<QMenu> contextMenu;
  QAction* actAddCourt;
  QAction* actDelCourt;
  QAction* actUndoCall;
  QAction* actFinishMatch;
  QMenu* walkoverSelectionMenu;
  QAction* actWalkoverP1;
  QAction* actWalkoverP2;
  QAction* actAddCall;
  QAction* actSwapReferee;
  QAction* actToggleAssignmentMode;
  QAction* actToggleEnableState;
  QAction* actReprintResultSheet;

  void initContextMenu();
  void updateContextMenu(bool isRowClicked);
  void execWalkover(int playerNum);
};

#endif	/* COURTTABLEVIEW_H */

