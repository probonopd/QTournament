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

#ifndef DLGSELECTREFEREE_H
#define DLGSELECTREFEREE_H

#include <tuple>

#include <QDialog>
#include <QTableWidget>

#include "TournamentDB.h"
#include "Match.h"
#include "delegates/RefereeSelectionDelegate.h"
#include "AutoSizingTable.h"

namespace Ui {
  class DlgSelectReferee;
}

using TaggedPlayer = std::pair<QTournament::Player, int>;
using TaggedPlayerList = QList<TaggedPlayer>;

class DlgSelectReferee : public QDialog
{
  Q_OBJECT

public:
  static constexpr int MAX_NUM_LOSERS = 30;
  explicit DlgSelectReferee(const QTournament::TournamentDB& _db, const QTournament::Match& _ma, QTournament::REFEREE_ACTION _refAction, QWidget *parent = nullptr);
  ~DlgSelectReferee();
  std::optional<QTournament::Player> getFinalPlayerSelection();

public slots:
  void onFilterModeChanged();
  void onTeamSelectionChanged();
  void onPlayerSelectionChanged();
  void onBtnSelectClicked();
  void onBtnNoneClicked();
  void onPlayerDoubleClicked();

private:
  Ui::DlgSelectReferee *ui;
  std::reference_wrapper<const QTournament::TournamentDB> db;
  QTournament::Match ma;
  QTournament::REFEREE_ACTION refAction;
  void updateControls();

  void initTeamList(int defaultTeamId = -1);
  void rebuildPlayerList();
  void resizeTabColumns();

  TaggedPlayerList getPlayerList_recentFinishers();

  std::optional<QTournament::Player> finalPlayerSelection;
};

//----------------------------------------------------------------------------

class RefereeTableWidget : public GuiHelpers::AutoSizingTableWidget_WithDatabase
{
  Q_OBJECT

public:
  static constexpr int ObjState::COL_ID = 0;
  static constexpr int NAME_COL_ID = 1;
  static constexpr int TEAM_COL_ID = 2;
  static constexpr int REFEREE_COUNT_COL_ID = 3;
  static constexpr int LAST_FINISH_TIME_COL_ID = 4;
  static constexpr int NEXT_MATCH_DIST_COL_ID = 5;
  RefereeTableWidget(QWidget* parent=0);
  virtual ~RefereeTableWidget() {}

  void rebuildPlayerList(const TaggedPlayerList& pList, int selectedMatchNumer, QTournament::REFEREE_MODE _refMode);
  std::optional<QTournament::Player> getSelectedPlayer();
  bool hasPlayerSelected();

protected:
  static constexpr int REL_WIDTH_NAME = 25;
  static constexpr int REL_WIDTH_TEAM = 25;
  static constexpr int REL_WIDTH_OTHER = 10;
  static constexpr int REL_WIDTH_STATE = 1;
  static constexpr int MAX_OTHER_COL_WIDTH = 90;

  void hook_onDatabaseOpened() override;

  QTournament::REFEREE_MODE refMode;
};

#endif // DLGSELECTREFEREE_H
