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
  explicit DlgSelectReferee(TournamentDB* _db, const Match& _ma, bool _matchIsBeingCalled, QWidget *parent = 0);
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
  bool matchIsBeingCalled;
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
  static constexpr int NUM_TAB_COLUMNS = 5;
  RefereeTableWidget(QWidget* parent=0);

  void rebuildPlayerList(const PlayerList& pList);
  upPlayer getSelectedPlayer();
  bool hasPlayerSelected();

protected:
  static constexpr int REL_WIDTH_NAME = 30;
  static constexpr int REL_WIDTH_TEAM = 20;
  static constexpr int REL_WIDTH_OTHER = 10;
  static constexpr int REL_WIDTH_STATE = 1;

  TournamentDB* db;

  virtual void resizeEvent(QResizeEvent *event) override;
};

#endif // DLGSELECTREFEREE_H
