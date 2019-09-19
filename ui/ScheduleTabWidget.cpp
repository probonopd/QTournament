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

#include <QMessageBox>

#include "ScheduleTabWidget.h"
#include "ui_ScheduleTabWidget.h"
#include "GuiHelpers.h"
#include "Score.h"
#include "ui/DlgMatchResult.h"
#include "ui/MainFrame.h"
#include "CentralSignalEmitter.h"
#include "MatchMngr.h"
#include "CatMngr.h"
#include "CourtMngr.h"
#include "Procedures.h"
#include "../BackendAPI.h"

using namespace QTournament;

ScheduleTabWidget::ScheduleTabWidget(QWidget *parent) :
    QDialog(parent), ui(new Ui::ScheduleTabWidget)
{
    ui->setupUi(this);

    // initialize sorting and filtering in the two match group views
    ui->mgIdleView->setFilter(MatchGroupTableView::FilterType::IDLE);
    ui->mgStagedView->setFilter(MatchGroupTableView::FilterType::STAGED);
    ui->mgStagedView->sortByColumn(MatchGroupTableModel::StageSeqColId, Qt::AscendingOrder);

    // react on selection changes in the IDLE match groups view
    connect(ui->mgIdleView->selectionModel(),
      SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
      SLOT(onIdleSelectionChanged(const QItemSelection&, const QItemSelection&)));

    // react on selection changes in the STAGED match groups view
    connect(ui->mgStagedView->selectionModel(),
      SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
      SLOT(onStagedSelectionChanged(const QItemSelection&, const QItemSelection&)));

    // default all buttons to "disabled"
    ui->btnSchedule->setEnabled(false);
    ui->btnStage->setEnabled(false);
    ui->btnUnstage->setEnabled(false);

    // make the btnHideStagingArea button as small as possible
    ui->btnHideStagingArea->setMaximumWidth(20);

}

//----------------------------------------------------------------------------

ScheduleTabWidget::~ScheduleTabWidget()
{
  delete ui;
}

//----------------------------------------------------------------------------

void ScheduleTabWidget::setDatabase(const TournamentDB* _db)
{
  db = _db;

  ui->mgIdleView->setDatabase(db);
  ui->mgStagedView->setDatabase(db);
  ui->tvCourts->setDatabase(db);
  ui->tvMatches->setDatabase(db);
  ui->pbRemainingMatches->setDatabase(db);

  if (db != nullptr)
  {
    // things to do when we open a new tournament
  } else {
    // things to do when we close a tournament
  }

  setEnabled(db != nullptr);
}

//----------------------------------------------------------------------------

void ScheduleTabWidget::updateRefereeColumn()
{
  ui->tvMatches->updateRefereeColumn();
}

//----------------------------------------------------------------------------

void ScheduleTabWidget::onBtnStageClicked()
{
  // lets check if a valid match group is selected
  auto mg = ui->mgIdleView->getSelectedMatchGroup();
  if (!mg) return;

  MatchMngr mm{*db};
  if (mm.canStageMatchGroup(*mg) != Error::OK) return;

  mm.stageMatchGroup(*mg);
}

//----------------------------------------------------------------------------

void ScheduleTabWidget::onBtnUnstageClicked()
{
  // lets check if a valid match group is selected
  auto mg = ui->mgStagedView->getSelectedMatchGroup();
  if (!mg) return;

  MatchMngr mm{*db};
  if (mm.canUnstageMatchGroup(*mg) != Error::OK) return;

  mm.unstageMatchGroup(*mg);
}

//----------------------------------------------------------------------------

void ScheduleTabWidget::onBtnScheduleClicked()
{
  // is at least one match group staged?
  MatchMngr mm{*db};
  if (mm.getMaxStageSeqNum() == 0) return;

  mm.scheduleAllStagedMatchGroups();
  updateButtons();
}

//----------------------------------------------------------------------------

void ScheduleTabWidget::onIdleSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
  updateButtons();
}

//----------------------------------------------------------------------------

void ScheduleTabWidget::onStagedSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
  updateButtons();
}

//----------------------------------------------------------------------------

void ScheduleTabWidget::updateButtons()
{
  MatchMngr mm{*db};

  // update the "stage"-button
  auto mg = ui->mgIdleView->getSelectedMatchGroup();
  if ((mg.has_value()) && (mm.canStageMatchGroup(*mg) == Error::OK))
  {
    ui->btnStage->setEnabled(true);
  } else {
    ui->btnStage->setEnabled(false);
  }

  // update the "unstage"-button
  mg = ui->mgStagedView->getSelectedMatchGroup();
  if ((mg.has_value()) && (mm.canUnstageMatchGroup(*mg) == Error::OK))
  {
    ui->btnUnstage->setEnabled(true);
  } else {
    ui->btnUnstage->setEnabled(false);
  }

  // update the "Schedule"-button
  ui->btnSchedule->setEnabled(mm.getMaxStageSeqNum() != 0);
}

//----------------------------------------------------------------------------

void ScheduleTabWidget::onCourtDoubleClicked(const QModelIndex &index)
{
  auto court = ui->tvCourts->getSelectedCourt();
  if (!court) return;

  auto ma = court->getMatch();
  if (!ma)
  {
    return;  // no match assigned to this court
  }

  askAndStoreMatchResult(*ma);
}

//----------------------------------------------------------------------------

void ScheduleTabWidget::onBtnHideStagingAreaClicked()
{
  bool setToVisible = !(ui->stagingWidget->isVisible());
  ui->stagingWidget->setVisible(setToVisible);

  if (setToVisible)
  {
    ui->btnHideStagingArea->setText("◀");
  } else {
    ui->btnHideStagingArea->setText("▶");
  }
}

//----------------------------------------------------------------------------

void ScheduleTabWidget::askAndStoreMatchResult(const Match &ma)
{
  // only accept results for running matches
  if (ma.is_NOT_InState(ObjState::MA_Running))
  {
    return;
  }

  // ask the user for the match result
  DlgMatchResult dlg(this, ma);
  dlg.setModal(true);
  int dlgResult = dlg.exec();
  if (dlgResult != QDialog::Accepted)
  {
    return;
  }
  auto matchResult = dlg.getMatchScore();
  assert(matchResult);

  // let the user confirm the result
  if (!GuiHelpers::showAndConfirmMatchResult(this, ma, matchResult)) return;

  // actually store the data and update the internal object states
  MatchMngr mm{*db};
  auto finalizationResult = mm.setMatchScoreAndFinalizeMatch(ma, *matchResult);
  Procedures::afterMatch(this, ma, finalizationResult);
  if (finalizationResult.err != Error::OK)
  {
    return;
  }

  // ask the user if the next available match should be started on the
  // now free court
  //
  // only do this if the court is not limited to manual match assignment
  //
  auto oldCourt = ma.getCourt(nullptr);
  assert(oldCourt);
  if (oldCourt->isManualAssignmentOnly()) return;

  // first of all, check if there is a next match available
  auto nextMatch = API::Qry::nextCallableMatch(*db);
  if (!nextMatch) return;

  // now ask if the match should be started
  int rc = QMessageBox::question(this, tr("Next Match"), tr("Start the next available match on the free court?"));
  if (rc != QMessageBox::Yes) return;

  // try to start the match on the old court
  Procedures::callMatch(this, *nextMatch, *oldCourt);
}

//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------

