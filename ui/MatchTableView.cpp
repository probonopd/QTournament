/*
 *    This is QTournament, a badminton tournament management program.
 *    Copyright (C) 2014 - 2015  Volker Knollmann
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

MatchTableView::MatchTableView(QWidget* parent)
  :QTableView(parent), db(nullptr), curDataModel(nullptr)
{
  // an empty model for clearing the table when
  // no tournament is open
  emptyModel = new QStringListModel();

  // prepare a proxy model to support sorting by columns
  sortedModel = new QSortFilterProxyModel();
  sortedModel->setSourceModel(emptyModel);
  setModel(sortedModel);

  defaultDelegate = itemDelegate();

  // initiate the model(s) as empty
  setDatabase(nullptr);

  // react on selection changes in the match table view
  connect(selectionModel(),
    SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
    SLOT(onSelectionChanged(const QItemSelection&, const QItemSelection&)));

  // handle context menu requests
  setContextMenuPolicy(Qt::CustomContextMenu);
  connect(this, SIGNAL(customContextMenuRequested(const QPoint&)),
          this, SLOT(onContextMenuRequested(const QPoint&)));

  // setup the context menu
  initContextMenu();

  // register for an ackward signal relay mechanism in order
  // to give reports access to the currently selected match
  SignalRelay::getInstance()->registerSender(this);
}

//----------------------------------------------------------------------------
    
MatchTableView::~MatchTableView()
{
  delete emptyModel;
  delete sortedModel;
  SignalRelay::cleanUp();
  if (curDataModel != nullptr) delete curDataModel;
  if (defaultDelegate != nullptr) delete defaultDelegate;
}

//----------------------------------------------------------------------------
    
unique_ptr<Match> MatchTableView::getSelectedMatch() const
{
  // make sure we have non-empty model
  auto mod = model();
  if (mod == nullptr) return nullptr;
  if (mod->rowCount() == 0) return nullptr;

  // make sure we have one item selected
  QModelIndexList indexes = selectionModel()->selection().indexes();
  if (indexes.count() == 0)
  {
    return nullptr;
  }

  // return the selected item
  int selectedSourceRow = sortedModel->mapToSource(indexes.at(0)).row();
  MatchMngr mm{db};
  return mm.getMatchBySeqNum(selectedSourceRow);
}

//----------------------------------------------------------------------------

void MatchTableView::onSelectionChanged(const QItemSelection& selectedItem, const QItemSelection& deselectedItem)
{
  for (auto item : selectedItem)
  {
    matchItemDelegate->setSelectedRow(item.top());
    //resizeRowToContents(item.top());
  }
  for (auto item : deselectedItem)
  {
    //resizeRowToContents(item.top());
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

void MatchTableView::setDatabase(TournamentDB* _db)
{
  // set the new data model
  MatchTableModel* newDataModel = nullptr;
  if (_db != nullptr)
  {
    newDataModel = new MatchTableModel(_db);
    sortedModel->setSourceModel(newDataModel);
    setColumnHidden(MatchTableModel::STATE_COL_ID, true);  // hide the column containing the internal object state

    // create a regular expression, that matches either the match state
    // READY, BUSY, FUZZY or WAITING
    QString reString = "^" + QString::number(static_cast<int>(STAT_MA_READY)) + "|";
    reString += QString::number(static_cast<int>(STAT_MA_BUSY)) + "|";
    reString += QString::number(static_cast<int>(STAT_MA_FUZZY)) + "|";   // TODO: check if there can be a condition where a match is FUZZY but without assigned match number
    reString += QString::number(static_cast<int>(STAT_MA_WAITING)) + "$";

    // apply the regExp as a filter on the state id column
    sortedModel->setFilterRegExp(reString);
    sortedModel->setFilterKeyColumn(MatchTableModel::STATE_COL_ID);

    // sort matches in ascing match number order
    sortedModel->sort(MatchTableModel::MATCH_NUM_COL_ID, Qt::AscendingOrder);

    // define a delegate for drawing the match items
    matchItemDelegate = make_unique<MatchItemDelegate>(_db, this);
    matchItemDelegate->setProxy(sortedModel);
    setItemDelegate(matchItemDelegate.get());

    // resize columns and rows to content once (we do not want permanent automatic resizing)
    //horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
    //verticalHeader()->resizeSections(QHeaderView::ResizeToContents);

  } else {
    sortedModel->setSourceModel(emptyModel);
    setItemDelegate(defaultDelegate);
  }

  // delete the old data model, if it was a
  // CategoryTableModel instance
  if (curDataModel != nullptr)
  {
    delete curDataModel;
  }

  // store the new CategoryTableModel instance, if any
  curDataModel = newDataModel;

  // update the database pointer and set the widget's enabled state
  db = _db;
  setEnabled(db != nullptr);

  // trigger a resizing of the displayed columns
  autosizeColumns();
}

//----------------------------------------------------------------------------

void MatchTableView::updateRefereeColumn()
{
  if (db == nullptr) return;

  QModelIndex topLeft = curDataModel->getIndex(0, MatchTableModel::REFEREE_MODE_COL_ID);
  QModelIndex bottomRight = curDataModel->getIndex(curDataModel->rowCount(), MatchTableModel::REFEREE_MODE_COL_ID);
  dataChanged(topLeft, bottomRight);
}

//----------------------------------------------------------------------------

void MatchTableView::resizeEvent(QResizeEvent* event)
{
  // call parent handler
  QTableView::resizeEvent(event);

  // resize all columns
  autosizeColumns();

  // finish event processing
  event->accept();
}

//----------------------------------------------------------------------------

void MatchTableView::autosizeColumns()
{
  // distribute the available space according to relative column widths
  int totalUnits = (REL_NUMERIC_COL_WIDTH + // Match number
                    REL_MATCH_COL_WIDTH + // Match details
                    REL_CAT_COL_WIDTH + // category
                    REL_NUMERIC_COL_WIDTH + // round
                    REL_NUMERIC_COL_WIDTH + // group
                    REL_REFEREE_COL_WIDTH); // referee

  int widthAvail = width();
  if ((verticalScrollBar() != nullptr) && (verticalScrollBar()->isVisible()))
  {
    widthAvail -= verticalScrollBar()->width();
  }
  int unitWidth = widthAvail / totalUnits;

  // determine a max width for numeric columns
  int numericColWidth = REL_NUMERIC_COL_WIDTH * unitWidth;
  if (numericColWidth > MAX_NUMERIC_COL_WIDTH) numericColWidth = MAX_NUMERIC_COL_WIDTH;

  int usedWidth = 0;

  // a little lambda that sets the column width and
  // aggregates it in a dedicated local variable
  auto myWidthSetter = [&](int colId, int newColWidth) {
    setColumnWidth(colId, newColWidth);
    usedWidth += newColWidth;
  };

  myWidthSetter(MatchTableModel::MATCH_NUM_COL_ID, numericColWidth);
  myWidthSetter(1, REL_MATCH_COL_WIDTH * unitWidth);  // Match details
  myWidthSetter(2, REL_CAT_COL_WIDTH * unitWidth);  // category
  myWidthSetter(3, numericColWidth);  // round
  myWidthSetter(4, numericColWidth);  // group

  // assign the remaining width to the referee. This accounts for
  // rounding errors when dividing / multiplying pixel widths and makes
  // that we always used the full width of the widget
  myWidthSetter(MatchTableModel::REFEREE_MODE_COL_ID, widthAvail - usedWidth);  // referee
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
  actAssignReferee->setEnabled(ma->canAssignReferee(REFEREE_ACTION::PRE_ASSIGN) == OK);
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
    MatchMngr mm{db};
    ERR e = mm.canAssignMatchToCourt(*ma, *co);
    if (e != OK)
    {
      QMessageBox::warning(this, tr("Call match"),
                           tr("Match can't be called on the selected court."));
      return;
    }

    cmdCallMatch cmd{this, *ma, *co};
    if (cmd.exec() == OK)
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
    REFEREE_MODE refMode = static_cast<REFEREE_MODE>(modeId);

    MatchMngr mm{db};
    ERR e = mm.setRefereeMode(*ma, refMode);
    if (e != OK)
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
  if (index.column() == MatchTableModel::REFEREE_MODE_COL_ID)
  {
    onAssignRefereeTriggered();
    return;  // do not proceed with a match call
  }

  CourtMngr cm{db};
  MatchMngr mm{db};

  // first of all, make sure that the match is eligible for being started
  if (ma->getState() != STAT_MA_READY)
  {
    QString msg = tr("This match cannot be started at this point in time.\n");
    msg += tr("It's probably waiting for all players to become available or \n");
    msg += tr("for matches in earlier rounds to be finished first.");
    QMessageBox::warning(this, tr("Assign match to court"), msg);
    return;
  }

  // check if there is a court available
  ERR err;
  auto nextCourt = cm.autoSelectNextUnusedCourt(&err, false);
  if (err == ONLY_MANUAL_COURT_AVAIL)
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

  if (err == NO_COURT_AVAIL)
  {
    QString msg = tr("The match cannot be started since there is no\n");
    msg += tr("free court available right now.");
    QMessageBox::warning(this, tr("Assign match to court"), msg);
    return;
  }

  // if we made it to this point and nextCourt is not null and err is OK,
  // we may try to assign the match
  if ((err == OK) && (nextCourt != nullptr))
  {
    cmdCallMatch cmd{this, *ma, *nextCourt};
    if (cmd.exec() == OK)
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
  ERR e = mm.removeReferee(*ma);
  if (e != OK)
  {
    QString msg = tr("Could not remove umpire assignment from match.\n");
    msg += tr("Some unexpected error occured.");
    QMessageBox::warning(this, tr("Umpire removal failed"), msg);
  }
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
  newAction->setData(static_cast<int>(REFEREE_MODE::NONE));
  refereeMode_submenu->addSeparator();
  newAction = refereeMode_submenu->addAction(tr("Pick from all players"));
  newAction->setData(static_cast<int>(REFEREE_MODE::ALL_PLAYERS));
  newAction = refereeMode_submenu->addAction(tr("Pick from recent losers"));
  newAction->setData(static_cast<int>(REFEREE_MODE::RECENT_LOSERS));
  newAction = refereeMode_submenu->addAction(tr("Pick from special team"));
  newAction->setData(static_cast<int>(REFEREE_MODE::SPECIAL_TEAM));
  refereeMode_submenu->addSeparator();
  newAction = refereeMode_submenu->addAction(tr("Manual"));
  newAction->setData(static_cast<int>(REFEREE_MODE::HANDWRITTEN));
  refereeMode_submenu->addSeparator();
  newAction = refereeMode_submenu->addAction(tr("Use tournament default"));
  newAction->setData(static_cast<int>(REFEREE_MODE::USE_DEFAULT));
  actAssignReferee = new QAction(tr("Assign umpire..."), this);
  actRemoveReferee = new QAction(tr("Remove assigned umpire"), this);
  contextMenu->addAction(actAssignReferee);
  contextMenu->addAction(actRemoveReferee);

  // connect actions and slots
  connect(actWalkoverP1, SIGNAL(triggered(bool)), this, SLOT(onWalkoverP1Triggered()));
  connect(actWalkoverP2, SIGNAL(triggered(bool)), this, SLOT(onWalkoverP2Triggered()));
  connect(actAssignReferee, SIGNAL(triggered(bool)), this, SLOT(onAssignRefereeTriggered()));
  connect(actRemoveReferee, SIGNAL(triggered(bool)), this, SLOT(onRemoveRefereeTriggered()));

  updateContextMenu();
}

//----------------------------------------------------------------------------

void MatchTableView::updateContextMenu()
{
  auto ma = getSelectedMatch();

  // completely re-build the list of available courts
  bool isCallPossible = ((ma != nullptr) && (ma->getState() == STAT_MA_READY));
  courtSelectionMenu->setEnabled(isCallPossible);
  if (isCallPossible)
  {
    courtSelectionMenu->clear();

    CourtMngr cm{db};

    QStringList availCourtNum;
    for (auto co : cm.getAllCourts())
    {
      if (co.getState() == STAT_CO_AVAIL)
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


//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

