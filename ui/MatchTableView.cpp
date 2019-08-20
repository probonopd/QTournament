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

#include <QMessageBox>
#include <QScrollBar>
#include <QResizeEvent>

#include "MatchTableView.h"
#include "MainFrame.h"

#include "CourtMngr.h"
#include "Court.h"
#include "ui/GuiHelpers.h"
#include "SignalRelay.h"
#include "MatchMngr.h"
#include "CourtMngr.h"
#include "DlgSelectReferee.h"
#include "ui/commonCommands/cmdAssignRefereeToMatch.h"
#include "ui/commonCommands/cmdCallMatch.h"
#include "CentralSignalEmitter.h"
#include "reports/ResultSheets.h"
#include <SimpleReportGeneratorLib/SimpleReportViewer.h>

MatchTableView::MatchTableView(QWidget* parent)
  :AutoSizingTableView_WithDatabase<MatchTableModel>{GuiHelpers::AutosizeColumnDescrList{
     {"", REL_NUMERIC_COL_WIDTH, -1, MAX_NUMERIC_COL_WIDTH},   // match number
     {"", REL_MATCH_COL_WIDTH, -1, -1},                        // match descrption
     {"", REL_CONFIGCOL_WIDTH, -1, -1},                          // category name
     {"", REL_NUMERIC_COL_WIDTH, -1, MAX_NUMERIC_COL_WIDTH},   // round
     {"", REL_NUMERIC_COL_WIDTH, -1, MAX_NUMERIC_COL_WIDTH},   // match group
     {"", 0, -1, -1},                                          // match state, invisible
     {"", REL_REFEREE_COL_WIDTH, -1, -1},                      // umpire
     {"", REL_NUMERIC_COL_WIDTH, -1, MAX_NUMERIC_COL_WIDTH},   // est. start
     {"", REL_NUMERIC_COL_WIDTH, -1, MAX_NUMERIC_COL_WIDTH},   // est.finish
     {"", REL_NUMERIC_COL_WIDTH, -1, MAX_NUMERIC_COL_WIDTH}    // est. court
   },true, parent}
{
  setRubberBandCol(1);

  // react on selection changes in the match table view
  connect(selectionModel(),
    SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
    SLOT(onSelectionChanged(const QItemSelection&, const QItemSelection&)));

  // handle context menu requests
  setContextMenuPolicy(Qt::CustomContextMenu);
  connect(this, SIGNAL(customContextMenuRequested(const QPoint&)),
          this, SLOT(onContextMenuRequested(const QPoint&)));

  // catch a UI-corner case when undoing match calls.
  // this feels a bit like a bad hack because it SHOULD be
  // handled through the underlying model and the DataChanged()-event
  connect(CentralSignalEmitter::getInstance(), SIGNAL(matchStatusChanged(int,int,ObjState,ObjState)),
          this, SLOT(onMatchStatusChanged(int,int,ObjState,ObjState)));

  // handle double clicks on a column header
  connect(horizontalHeader(), SIGNAL(sectionDoubleClicked(int)), this, SLOT(onSectionHeaderDoubleClicked()));

  // setup the context menu
  initContextMenu();

  // register for an ackward signal relay mechanism in order
  // to give reports access to the currently selected match
  SignalRelay::getInstance()->registerSender(this);

  // prepare a timer that periodically updates the predicted start / finish times for matches
  predictionUpdateTimer = make_unique<QTimer>(this);
  connect(predictionUpdateTimer.get(), SIGNAL(timeout()), this, SLOT(onMatchTimePredictionUpdate()));
  predictionUpdateTimer->start(PREDICTION_UPDATE_INTERVAL__MS);
}

//----------------------------------------------------------------------------
    
MatchTableView::~MatchTableView()
{
  SignalRelay::cleanUp();
}

//----------------------------------------------------------------------------
    
std::optional<QTournament::Match> MatchTableView::getSelectedMatch() const
{
  int srcRow = getSelectedSourceRow();
  if (srcRow < 0) return nullptr;

  MatchMngr mm{db};
  return mm.getMatchBySeqNum(srcRow);
}

//----------------------------------------------------------------------------

void MatchTableView::onSelectionChanged(const QItemSelection& selectedItem, const QItemSelection& deselectedItem)
{
  for (auto item : selectedItem)
  {
    matchItemDelegate->setSelectedRow(item.top());
    resizeRowToContents(item.top());
  }
  for (auto item : deselectedItem)
  {
    resizeRowToContents(item.top());
  }

  // emit a signal containing the newly selected match
  // this is used by the result sheets report to update
  // the range of printed matches
  auto ma = getSelectedMatch();
  emit matchSelectionChanged((ma == nullptr) ? -1 : ma->getId());
}

//----------------------------------------------------------------------------

void MatchTableView::updateSelectionAfterDataChange()
{
  QModelIndexList indexes = selectionModel()->selection().indexes();
  if (indexes.count() == 0)
  {
    return;
  }
  int selectedTargetRow = indexes.at(0).row();
  matchItemDelegate->setSelectedRow(selectedTargetRow);
  resizeRowToContents(selectedTargetRow);
}

//----------------------------------------------------------------------------

void MatchTableView::updateRefereeColumn()
{
  if (db == nullptr) return;

  QModelIndex topLeft = customDataModel->getIndex(0, MatchTableModel::RefereeMode_COL_ID);
  QModelIndex bottomRight = customDataModel->getIndex(customDataModel->rowCount(), MatchTableModel::RefereeMode_COL_ID);
  dataChanged(topLeft, bottomRight);
}

//----------------------------------------------------------------------------

void MatchTableView::hook_onDatabaseOpened()
{
  AutoSizingTableView_WithDatabase::hook_onDatabaseOpened();

  setColumnHidden(MatchTableModel::STATE_COL_ID, true);  // hide the column containing the internal object state

  // create a regular expression, that matches either the match state
  // READY, BUSY, FUZZY or WAITING
  QString reString = "^" + QString::number(static_cast<int>(ObjState::MA_Ready)) + "|";
  reString += QString::number(static_cast<int>(ObjState::MA_Busy)) + "|";
  reString += QString::number(static_cast<int>(ObjState::MA_Fuzzy)) + "|";   // TODO: check if there can be a condition where a match is FUZZY but without assigned match number
  reString += QString::number(static_cast<int>(ObjState::MA_Waiting)) + "$";

  // apply the regExp as a filter on the state id column
  sortedModel->setFilterRegExp(reString);
  sortedModel->setFilterKeyColumn(MatchTableModel::STATE_COL_ID);

  // sort matches in ascing match number order
  sortedModel->sort(MatchTableModel::MATCH_NUM_COL_ID, Qt::AscendingOrder);

  // define a delegate for drawing the match items
  matchItemDelegate =  new MatchItemDelegate(db, this);
  matchItemDelegate->setProxy(sortedModel.get());
  setCustomDelegate(matchItemDelegate);  // takes ownership
}

//----------------------------------------------------------------------------

void MatchTableView::onContextMenuRequested(const QPoint& pos)
{
  // map from scroll area coordinates to global widget coordinates
  QPoint globalPos = viewport()->mapToGlobal(pos);

  // resolve the click coordinates to the table row
  int clickedRow = rowAt(pos.y());

  // exit if no table row is under the cursor
  if (clickedRow < 0) return;

  // find out which match is selected
  auto ma = getSelectedMatch();
  if (ma == nullptr) return;  // shouldn't happen

  // enable / disable the action for assigning umpires, depending
  // on the current umpire mode and match state
  actAssignReferee->setEnabled(ma->canAssignReferee(REFEREE_ACTION::PRE_ASSIGN) == Error::OK);
  actRemoveReferee->setEnabled(ma->hasRefereeAssigned());

  // show the context menu
  updateContextMenu();
  QAction* selectedItem = contextMenu->exec(globalPos);

  if (selectedItem == nullptr) return; // user canceled

  // a bad hack:
  // if the text of the selected item is purely numeric, the user
  // selected "call match on court XXX". We use this information
  // to trigger the right action
  bool isOk = false;
  int selectedCourt = selectedItem->text().toInt(&isOk);
  if (isOk)
  {
    // get the selected court
    CourtMngr cm{db};
    auto co = cm.getCourt(selectedCourt);
    if (co == nullptr) return;  // shouldn't happen

    // call the match on the selected court
    cmdCallMatch cmd{this, *ma, *co};
    if (cmd.exec() == Error::OK)
    {
      updateSelectionAfterDataChange();
    }
  }

  // another hack:
  // if the selected item has non-empty user data, one of the umpire modes
  // has been selected.
  //
  // try to assign the new umpire mode to the match
  if (!(selectedItem->data().isNull()))
  {
    int modeId = selectedItem->data().toInt();
    RefereeMode refMode = static_cast<RefereeMode>(modeId);

    MatchMngr mm{db};
    Error e = mm.setRefereeMode(*ma, refMode);
    if (e != Error::OK)
    {
      QMessageBox::warning(this, tr("Set umpire mode"),
                           tr("The mode can't be set for this match."));
    }
  }
}

//----------------------------------------------------------------------------

void MatchTableView::onWalkoverP1Triggered()
{
  execWalkover(1);
}

//----------------------------------------------------------------------------

void MatchTableView::onWalkoverP2Triggered()
{
  execWalkover(2);
}

//----------------------------------------------------------------------------

void MatchTableView::onMatchDoubleClicked(const QModelIndex& index)
{
  auto ma = getSelectedMatch();
  if (ma == nullptr) return;

  // special case: if the user double-clicked the umpire-column,
  // we do not call the match, but we present the dialog for
  // selecting an umpire, if possible and meaningful
  if (index.column() == MatchTableModel::RefereeMode_COL_ID)
  {
    onAssignRefereeTriggered();
    return;  // do not proceed with a match call
  }

  // special case: if the match is busy, we show information
  // why the match can't be called
  if (ma->getState() == ObjState::MA_Busy)
  {
    showMatchBusyReason(*ma);
    return;  // do not proceed with a match call
  }

  CourtMngr cm{db};
  MatchMngr mm{db};

  // first of all, make sure that the match is eligible for being started
  if (ma->getState() != ObjState::MA_Ready)
  {
    QString msg = tr("This match cannot be started at this point in time.\n");
    msg += tr("It's probably waiting for all players to become available or \n");
    msg += tr("for matches in earlier rounds to be finished first.");
    QMessageBox::warning(this, tr("Assign match to court"), msg);
    return;
  }

  // check if there is a court available
  Error err;
  auto nextCourt = cm.autoSelectNextUnusedCourt(&err, false);
  if (err == OnlyManualCourtAvail)
  {
    QString msg = tr("There are no free courts for automatic match assignment available right now.\n");
    msg += tr("However, there is at least one free court for manual match assignment.\n\n");
    msg += tr("Use this court for running the match?");
    int result = QMessageBox::question(this, tr("Assign match to court"), msg);
    if (result != QMessageBox::Yes)
    {
      return;
    }

    nextCourt = cm.autoSelectNextUnusedCourt(&err, true);
    if (nextCourt == nullptr)
    {
      QString msg = tr("An unexpected error occured.\n");
      msg += tr("Sorry, this shouldn't happen.\n");
      msg += tr("The match cannot be started.");
      QMessageBox::critical(this, tr("Assign match to court"), msg);
      return;
    }
  }

  if (err == NoCourtAvail)
  {
    QString msg = tr("The match cannot be started since there is no\n");
    msg += tr("free court available right now.");
    QMessageBox::warning(this, tr("Assign match to court"), msg);
    return;
  }

  // if we made it to this point and nextCourt is not null and err is OK,
  // we may try to assign the match
  if ((err == Error::OK) && (nextCourt != nullptr))
  {
    cmdCallMatch cmd{this, *ma, *nextCourt};
    if (cmd.exec() == Error::OK)
    {
      updateSelectionAfterDataChange();
    }
  }
}

//----------------------------------------------------------------------------

void MatchTableView::onAssignRefereeTriggered()
{
  auto ma = getSelectedMatch();
  if (ma == nullptr) return;

  cmdAssignRefereeToMatch cmd{this, *ma, REFEREE_ACTION::PRE_ASSIGN};
  cmd.exec();
}

//----------------------------------------------------------------------------

void MatchTableView::onRemoveRefereeTriggered()
{
  auto ma = getSelectedMatch();
  if (ma == nullptr) return;

  // make sure there is a referee assigned
  if (!(ma->hasRefereeAssigned()))
  {
    return;
  }

  // try to remove the assignment
  MatchMngr mm{db};
  Error e = mm.removeReferee(*ma);
  if (e != Error::OK)
  {
    QString msg = tr("Could not remove umpire assignment from match.\n");
    msg += tr("Some unexpected error occured.");
    QMessageBox::warning(this, tr("Umpire removal failed"), msg);
  }
}

//----------------------------------------------------------------------------

void MatchTableView::onSectionHeaderDoubleClicked()
{
  autosizeColumns();
}

//----------------------------------------------------------------------------

void MatchTableView::onMatchTimePredictionUpdate()
{
  if (hasCustomDataModel())
  {
    customDataModel->recalcPrediction();
  }
}

//----------------------------------------------------------------------------

void MatchTableView::onMatchStatusChanged(int maId, int maSeqNum, ObjState oldStat, ObjState newStat)
{
  // a special hack here: if we undo a match call, the match is re-inserted into
  // the match table. Thus we need to update the selection data because otherwise
  // the delegate uses the "paintSelectedCell"-method for the wrong row.
  //
  // Background: when re-inserting the match, the relation between selected row
  // and associated match changes...
  if ((oldStat == ObjState::MA_Running) && (newStat == ObjState::MA_Ready))
  {
    auto selMatch = getSelectedMatch();
    if (selMatch == nullptr) return;

    // will the match be inserted before or after the currently selected match?
    //
    // we only need to take action if the match is inserted BEFORE the currently
    // selected match
    if (selMatch->getSeqNum() < maSeqNum) return;

    int oldRow = matchItemDelegate->getSelectedRow();
    matchItemDelegate->setSelectedRow(oldRow + 1);
  }

  // the same is true when setting a match to "walkover" straight from the match table.
  // But in this case, a match is REMOVED from the list and thus we need
  // we need to invert the setSelectedRow-logic here
  if ((oldStat != ObjState::MA_Running) && (newStat == ObjState::MA_Finished))
  {
    updateSelectionAfterDataChange();
  }

}

//----------------------------------------------------------------------------

void MatchTableView::onPrint1Selected()
{
  printResultSheets(1);
}

//----------------------------------------------------------------------------

void MatchTableView::onPrint4Selected()
{
  printResultSheets(4);
}

//----------------------------------------------------------------------------

void MatchTableView::onPrint8Selected()
{
  printResultSheets(8);
}

//----------------------------------------------------------------------------

void MatchTableView::onPrint12Selected()
{
  printResultSheets(12);
}

//----------------------------------------------------------------------------

void MatchTableView::initContextMenu()
{
  // prepare all actions
  actPostponeMatch = new QAction(tr("Postpone"), this);

  // create sub-actions for the walkover-selection
  actWalkoverP1 = new QAction("P1", this);  // this is just a dummy
  actWalkoverP2 = new QAction("P2", this);  // this is just a dummy

  // create the context menu and connect it to the actions
  contextMenu = make_unique<QMenu>();
  walkoverSelectionMenu = contextMenu->addMenu(tr("Walkover for..."));
  walkoverSelectionMenu->addAction(actWalkoverP1);
  walkoverSelectionMenu->addAction(actWalkoverP2);

  // disable the "Postpone"-entry as long as this function
  // is not implemented
  //contextMenu->addAction(actPostponeMatch);

  contextMenu->addSeparator();
  courtSelectionMenu = contextMenu->addMenu(tr("Call match on court..."));

  // submenus for umpire functions
  contextMenu->addSeparator();
  refereeMode_submenu = contextMenu->addMenu(tr("Set umpire mode"));
  auto newAction = refereeMode_submenu->addAction(tr("None"));
  newAction->setData(static_cast<int>(RefereeMode::None));
  refereeMode_submenu->addSeparator();
  newAction = refereeMode_submenu->addAction(tr("Pick from all players"));
  newAction->setData(static_cast<int>(RefereeMode::AllPlayers));
  newAction = refereeMode_submenu->addAction(tr("Pick from recent finishers"));
  newAction->setData(static_cast<int>(RefereeMode::RecentFinishers));
  newAction = refereeMode_submenu->addAction(tr("Pick from special team"));
  newAction->setData(static_cast<int>(RefereeMode::SpecialTeam));
  refereeMode_submenu->addSeparator();
  newAction = refereeMode_submenu->addAction(tr("Manual"));
  newAction->setData(static_cast<int>(RefereeMode::HandWritten));
  refereeMode_submenu->addSeparator();
  newAction = refereeMode_submenu->addAction(tr("Use tournament default"));
  newAction->setData(static_cast<int>(RefereeMode::UseDefault));
  actAssignReferee = new QAction(tr("Assign umpire..."), this);
  actRemoveReferee = new QAction(tr("Remove assigned umpire"), this);
  contextMenu->addAction(actAssignReferee);
  contextMenu->addAction(actRemoveReferee);

  // create a sub-menu for printing result sheets
  contextMenu->addSeparator();
  printSelectionMenu = contextMenu->addMenu(tr("Print result sheet"));
  printResultSheet1 = printSelectionMenu->addAction(tr("This match"));
  printResultSheet4 = printSelectionMenu->addAction(tr("4 matches"));
  printResultSheet8 = printSelectionMenu->addAction(tr("8 matches"));
  printResultSheet12 = printSelectionMenu->addAction(tr("12 matches"));

  // connect actions and slots
  connect(actWalkoverP1, SIGNAL(triggered(bool)), this, SLOT(onWalkoverP1Triggered()));
  connect(actWalkoverP2, SIGNAL(triggered(bool)), this, SLOT(onWalkoverP2Triggered()));
  connect(actAssignReferee, SIGNAL(triggered(bool)), this, SLOT(onAssignRefereeTriggered()));
  connect(actRemoveReferee, SIGNAL(triggered(bool)), this, SLOT(onRemoveRefereeTriggered()));
  connect(printResultSheet1, SIGNAL(triggered(bool)), this, SLOT(onPrint1Selected()));
  connect(printResultSheet4, SIGNAL(triggered(bool)), this, SLOT(onPrint4Selected()));
  connect(printResultSheet8, SIGNAL(triggered(bool)), this, SLOT(onPrint8Selected()));
  connect(printResultSheet12, SIGNAL(triggered(bool)), this, SLOT(onPrint12Selected()));

  updateContextMenu();
}

//----------------------------------------------------------------------------

void MatchTableView::updateContextMenu()
{
  auto ma = getSelectedMatch();

  // completely re-build the list of available courts
  bool isCallPossible = ((ma != nullptr) && (ma->getState() == ObjState::MA_Ready));
  courtSelectionMenu->setEnabled(isCallPossible);
  if (isCallPossible)
  {
    courtSelectionMenu->clear();

    CourtMngr cm{db};

    QStringList availCourtNum;
    for (auto co : cm.getAllCourts())
    {
      if (co.getState() == ObjState::CO_Avail)
      {
        availCourtNum << QString::number(co.getNumber());
      }
    }

    availCourtNum.sort();

    for (QString num : availCourtNum)
    {
      courtSelectionMenu->addAction(num);
    }

    courtSelectionMenu->setEnabled(availCourtNum.size() > 0);
  }

  // update the player pair names for the walkover menu
  walkoverSelectionMenu->setEnabled(ma != nullptr);
  if (ma != nullptr)
  {
    if (ma->isWalkoverPossible())
    {
      actWalkoverP1->setText(ma->getPlayerPair1().getDisplayName());
      actWalkoverP2->setText(ma->getPlayerPair2().getDisplayName());
    } else {
      walkoverSelectionMenu->setEnabled(false);
    }
  }

  // update the "postpone" action
  actPostponeMatch->setEnabled(ma != nullptr);

  // enable / disable print the result sheet
  printSelectionMenu->setEnabled(ma != nullptr);
}

//----------------------------------------------------------------------------

void MatchTableView::execWalkover(int playerNum)
{
  auto ma = getSelectedMatch();
  if (ma == nullptr) return; // shouldn't happen
  if ((playerNum != 1) && (playerNum != 2)) return; // shouldn't happen
  GuiHelpers::execWalkover(this, *ma, playerNum);
}

//----------------------------------------------------------------------------

void MatchTableView::showMatchBusyReason(const Match& ma)
{
  if (ma.getState() != ObjState::MA_Busy)
  {
    return;
  }

  QString txt = tr("The match cannot be called because:\n\n");

  //--------------------
  //
  // a little helper function that return the "busy reason" for
  // a single player
  auto getBusyReasonForPlayer = [](const Player& pl)
  {
    ObjState plStat = pl.getState();
    if (plStat == ObjState::PL_Idle) return QString();

    // maybe the player is busy in another match
    if (plStat == ObjState::PL_Playing)
    {
      QString result = tr("%1 is playing");
      result = result.arg(pl.getDisplayName_FirstNameFirst());

      // try to retrieve the court number where the player
      // is the referee
      std::unique_ptr<Court> co = pl.getMatchCourt();
      if (co != nullptr)
      {
        result += tr(" on court %1");
        result = result.arg(co->getNumber());
      }
      return result;
    }

    // maybe the player is acting as an umpire
    if (plStat == ObjState::PL_Referee)
    {
      QString result = tr("%1 is umpire");
      result = result.arg(pl.getDisplayName_FirstNameFirst());

      // try to retrieve the court number where the player
      // is the referee
      std::unique_ptr<Court> co = pl.getRefereeCourt();
      if (co != nullptr)
      {
        result += tr(" on court %1");
        result = result.arg(co->getNumber());
      }
      return result;
    }

    return pl.getDisplayName_FirstNameFirst() + tr(" is not available (but why???)");
  };
  //
  //--------------------

  // check all assigned players for their absence reason
  PlayerList pl;
  PlayerPair pp = ma.getPlayerPair1();
  pl.push_back(pp.getPlayer1());
  if (pp.hasPlayer2())
  {
    pl.push_back(pp.getPlayer2());
  }
  pp = ma.getPlayerPair2();
  pl.push_back(pp.getPlayer1());
  if (pp.hasPlayer2())
  {
    pl.push_back(pp.getPlayer2());
  }
  QString reason;
  for (const Player& p : pl)
  {
    QString r = getBusyReasonForPlayer(p);
    if (!(r.isEmpty()))
    {
      if (!(reason.isEmpty()))
      {
        reason += ",\n\n";
      }
      reason += "   - " + r;
    }
  }

  // if this match has already an umpire assigned, we need to
  // check the umpire's availability as well
  RefereeMode refMode = ma.get_EFFECTIVE_RefereeMode();
  if ((refMode != RefereeMode::None) && (refMode != RefereeMode::HandWritten))
  {
    upPlayer referee = ma.getAssignedReferee();
    if (referee != nullptr)
    {
      QString r = getBusyReasonForPlayer(*referee);

      if (!(r.isEmpty()))
      {
        if (!(reason.isEmpty()))
        {
          reason += ",\n\n";
        }
        reason += "   - " + tr("Umpire ") + r;
      }
    }
  }

  txt += reason;
  QMessageBox::information(this, tr("Match status"), txt);
}

//----------------------------------------------------------------------------

void MatchTableView::printResultSheets(int matchCount)
{
  if (matchCount < 1) return;

  auto curMatch = getSelectedMatch();
  if (curMatch == nullptr) return;

  // try to create the result sheet report with the
  // requested number of matches
  std::unique_ptr<ResultSheets> rep{nullptr};
  try
  {
    rep = make_unique<ResultSheets>(db, *curMatch, matchCount);
  }
  catch (...) {}
  if (rep == nullptr) return;

  // let the report object create the actual output
  upSimpleReport sr = rep->regenerateReport();

  // create an invisible report viewer and directly trigger
  // the print reaction
  SimpleReportLib::SimpleReportViewer viewer{this};
  viewer.setReport(sr.get());
  viewer.onBtnPrintClicked();

}

//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

