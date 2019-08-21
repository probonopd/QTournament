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

#ifndef SCHEDULETABWIDGET_H
#define SCHEDULETABWIDGET_H

#include <QDialog>
#include <QItemSelection>
#include <QItemSelectionModel>
#include <QTimer>

#include "TournamentDB.h"
#include "Match.h"

namespace Ui {
  class ScheduleTabWidget;
}


class ScheduleTabWidget : public QDialog
{
  Q_OBJECT

public:
  static constexpr int InitialAvgMatchDuration_secs = 25 * 60;
  static constexpr int ProgressBarUpdateInterval_secs = 20;

  explicit ScheduleTabWidget(QWidget *parent = nullptr);
  ~ScheduleTabWidget();

  void setDatabase(const QTournament::TournamentDB* _db);

  void updateRefereeColumn();


public slots:
  void onBtnStageClicked();
  void onBtnUnstageClicked();
  void onBtnScheduleClicked();
  void onIdleSelectionChanged(const QItemSelection &, const QItemSelection &);
  void onStagedSelectionChanged(const QItemSelection &, const QItemSelection &);
  void onCourtDoubleClicked(const QModelIndex& index);
  void onRoundCompleted(int catId, int round);
  void onBtnHideStagingAreaClicked();

private:
  const QTournament::TournamentDB* db{nullptr};
  Ui::ScheduleTabWidget *ui;
  void updateButtons();
  void askAndStoreMatchResult(const QTournament::Match& ma);

};

#endif // SCHEDULETABWIDGET_H
