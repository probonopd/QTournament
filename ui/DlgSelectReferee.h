/*
 *    This is QTournament, a badminton tournament management program.
 *    Copyright (C) 2014 - 2016  Volker Knollmann
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

#include <QDialog>
#include <QTableWidget>

#include "TournamentDB.h"
#include "Match.h"

namespace Ui {
  class DlgSelectReferee;
}

using namespace QTournament;

class DlgSelectReferee : public QDialog
{
  Q_OBJECT

public:
  static constexpr int MAX_NUM_LOSERS = 30;
  explicit DlgSelectReferee(TournamentDB* _db, const Match& _ma, REFEREE_ACTION _refAction, QWidget *parent = 0);
  ~DlgSelectReferee();
  upPlayer getFinalPlayerSelection();

public slots:
  void onFilterModeChanged();
  void onTeamSelectionChanged();
  void onPlayerSelectionChanged();
  void onBtnSelectClicked();
  void onBtnNoneClicked();
  void onPlayerDoubleClicked();

private:
  Ui::DlgSelectReferee *ui;
  TournamentDB* db;
  Match ma;
  REFEREE_ACTION refAction;
  void updateControls();

  void initTeamList(int defaultTeamId = -1);
  void rebuildPlayerList();
  void resizeTabColumns();

  PlayerList getPlayerList_recentLosers();

  upPlayer finalPlayerSelection;
};

//----------------------------------------------------------------------------

class RefereeTableWidget : public QTableWidget
{
  Q_OBJECT

public:
  static constexpr int STAT_COL_ID = 0;
  static constexpr int NAME_COL_ID = 1;
  static constexpr int TEAM_COL_ID = 2;
  static constexpr int REFEREE_COUNT_COL_ID = 3;
  static constexpr int LAST_FINISH_TIME_COL_ID = 4;
  static constexpr int NEXT_MATCH_DIST_COL_ID = 5;
  static constexpr int NUM_TAB_COLUMNS = 6;
  RefereeTableWidget(QWidget* parent=0);

  void rebuildPlayerList(const PlayerList& pList, int selectedMatchNumer);
  upPlayer getSelectedPlayer();
  bool hasPlayerSelected();

protected:
  static constexpr int REL_WIDTH_NAME = 25;
  static constexpr int REL_WIDTH_TEAM = 25;
  static constexpr int REL_WIDTH_OTHER = 10;
  static constexpr int REL_WIDTH_STATE = 1;

  TournamentDB* db;

  virtual void resizeEvent(QResizeEvent *_event) override;
};

#endif // DLGSELECTREFEREE_H
