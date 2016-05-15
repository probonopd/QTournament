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
#include "CourtTableView.h"
#include "MainFrame.h"
#include "CourtMngr.h"
#include "MatchMngr.h"
#include "ui/GuiHelpers.h"
#include "ui/commonCommands/cmdAssignRefereeToMatch.h"

CourtTableView::CourtTableView(QWidget* parent)
  :QTableView(parent), db(nullptr), curCourtTabModel(nullptr)
{
  // an empty model for clearing the table when
  // no tournament is open
  emptyModel = new QStringListModel();
  defaultDelegate = itemDelegate();

  // prepare a proxy model to support sorting by columns
  sortedModel = new QSortFilterProxyModel();
  sortedModel->setSourceModel(emptyModel);
  setModel(sortedModel);

  // initiate the model(s) as empty
  setDatabase(nullptr);

  // react on selection changes in the court table view
  connect(selectionModel(),
    SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
    SLOT(onSelectionChanged(const QItemSelection&, const QItemSelection&)));

  // handle context menu requests
  setContextMenuPolicy(Qt::CustomContextMenu);
  connect(this, SIGNAL(customContextMenuRequested(const QPoint&)),
          this, SLOT(onContextMenuRequested(const QPoint&)));

  // setup the context menu and its actions
  initContextMenu();
}

//----------------------------------------------------------------------------
    
CourtTableView::~CourtTableView()
{
  delete emptyModel;
  delete sortedModel;
  //delete itemDelegate;

  if (curCourtTabModel != nullptr) delete curCourtTabModel;
  if (defaultDelegate != nullptr) delete defaultDelegate;
}

//----------------------------------------------------------------------------
    
unique_ptr<Court> CourtTableView::getSelectedCourt() const
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
  CourtMngr cm{db};
  return cm.getCourtBySeqNum(selectedSourceRow);
}

//----------------------------------------------------------------------------

unique_ptr<Match> CourtTableView::getSelectedMatch() const
{
  auto co = getSelectedCourt();
  if (co == nullptr) return nullptr;

  return co->getMatch();
}

//----------------------------------------------------------------------------

void CourtTableView::setDatabase(TournamentDB* _db)
{
  // set the new data model
  CourtTableModel* newCourtTabModel = nullptr;
  if (_db != nullptr)
  {
    newCourtTabModel = new CourtTableModel(_db);
    sortedModel->setSourceModel(newCourtTabModel);
    sortedModel->sort(CourtTableModel::COURT_NUM_COL_ID, Qt::AscendingOrder);

    // update the delegate
    courtItemDelegate = make_unique<CourtItemDelegate>(_db, this);
    courtItemDelegate->setProxy(sortedModel);
    setItemDelegate(courtItemDelegate.get());

    // resize columns and rows to content once (we do not want permanent automatic resizing)
    horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
    verticalHeader()->resizeSections(QHeaderView::ResizeToContents);

  } else {
    sortedModel->setSourceModel(emptyModel);
    setItemDelegate(defaultDelegate);
  }

  // delete the old data model, if it was a
  // CategoryTableModel instance
  if (curCourtTabModel != nullptr)
  {
    delete curCourtTabModel;
  }

  // store the new CategoryTableModel instance, if any
  curCourtTabModel = newCourtTabModel;

  // update the database pointer and set the widget's enabled state
  db = _db;
  setEnabled(db != nullptr);
}

//----------------------------------------------------------------------------

void CourtTableView::onSelectionChanged(const QItemSelection& selectedItem, const QItemSelection& deselectedItem)
{
  resizeRowsToContents();
  for (auto item : selectedItem)
  {
    courtItemDelegate->setSelectedRow(item.top());
    resizeRowToContents(item.top());
  }
  for (auto item : deselectedItem)
  {
    resizeRowToContents(item.top());
  }
}

//----------------------------------------------------------------------------
    
void CourtTableView::initContextMenu()
{
  // prepare all actions
  actAddCourt = new QAction(tr("Add court"), this);
  actUndoCall = new QAction(tr("Undo call"), this);
  actFinishMatch = new QAction(tr("Finish match"), this);
  actAddCall = new QAction(tr("Repeat call"), this);
  actSwapReferee = new QAction(tr("Swap umpire"), this);

  // create sub-actions for the walkover-selection
  actWalkoverP1 = new QAction("P1", this);  // this is just a dummy
  actWalkoverP2 = new QAction("P2", this);  // this is just a dummy

  // link actions to slots
  connect(actAddCourt, SIGNAL(triggered()), this, SLOT(onActionAddCourtTriggered()));
  connect(actWalkoverP1, SIGNAL(triggered(bool)), this, SLOT(onWalkoverP1Triggered()));
  connect(actWalkoverP2, SIGNAL(triggered(bool)), this, SLOT(onWalkoverP2Triggered()));
  connect(actUndoCall, SIGNAL(triggered(bool)), this, SLOT(onActionUndoCallTriggered()));
  connect(actAddCall, SIGNAL(triggered(bool)), this, SLOT(onActionAddCallTriggered()));
  connect(actSwapReferee, SIGNAL(triggered(bool)), this, SLOT(onActionSwapRefereeTriggered()));

  // create the context menu and connect it to the actions
  contextMenu = make_unique<QMenu>();
  contextMenu->addAction(actFinishMatch);
  contextMenu->addAction(actAddCall);
  walkoverSelectionMenu = contextMenu->addMenu(tr("Walkover for..."));
  walkoverSelectionMenu->addAction(actWalkoverP1);
  walkoverSelectionMenu->addAction(actWalkoverP2);
  contextMenu->addAction(actUndoCall);
  contextMenu->addSeparator();
  contextMenu->addAction(actSwapReferee);
  contextMenu->addSeparator();
  contextMenu->addAction(actAddCourt);

  updateContextMenu(false);
}

//----------------------------------------------------------------------------

void CourtTableView::updateContextMenu(bool isRowClicked)
{
  // disable / enable actions that depend on a selected match
  auto ma = getSelectedMatch();
  bool isMatchClicked = (isRowClicked && (ma != nullptr));
  actUndoCall->setEnabled(isMatchClicked);
  walkoverSelectionMenu->setEnabled(isMatchClicked);
  actFinishMatch->setEnabled(isMatchClicked);

  QList<QDateTime> callTimes;
  if (ma != nullptr)
  {
    callTimes = ma->getAdditionalCallTimes();
  }
  actAddCall->setEnabled(isMatchClicked && (callTimes.size() < MAX_NUM_ADD_CALL));

  // enable / disable actions that depend on a selected row
  actAddCourt->setEnabled(!isRowClicked);

  // update the player pair names for the walkover menu
  if (ma != nullptr)
  {
    actWalkoverP1->setText(ma->getPlayerPair1().getDisplayName());
    actWalkoverP2->setText(ma->getPlayerPair2().getDisplayName());
  }
}

//----------------------------------------------------------------------------

void CourtTableView::execWalkover(int playerNum)
{
  auto ma = getSelectedMatch();
  if (ma == nullptr) return;
  if ((playerNum != 1) && (playerNum != 2)) return; // shouldn't happen
  GuiHelpers::execWalkover(this, *ma, playerNum);
}

//----------------------------------------------------------------------------
    
void CourtTableView::onContextMenuRequested(const QPoint& pos)
{
  // map from scroll area coordinates to global widget coordinates
  QPoint globalPos = viewport()->mapToGlobal(pos);

  // resolve the click coordinates to the table row
  int clickedRow = rowAt(pos.y());

  // if no table row is under the cursor, we may only
  // add a court. If a row is under the cursor, we may
  // do everything except for adding courts
  bool isRowClicked = (clickedRow >= 0);
  updateContextMenu(isRowClicked);

  // show the context menu; actions are triggered
  // by the menu itself and we do not need to take
  // further steps here
  contextMenu->exec(globalPos);
}

//----------------------------------------------------------------------------

void CourtTableView::onActionAddCourtTriggered()
{
  CourtMngr cm{db};

  int nextCourtNum = cm.getHighestUnusedCourtNumber();

  ERR err;
  cm.createNewCourt(nextCourtNum, QString::number(nextCourtNum), &err);

  if (err != OK)
  {
    QMessageBox::warning(this, tr("Add court"),
                         tr("Something went wrong, error code = ") + QString::number(static_cast<int>(err)));
  }
}

//----------------------------------------------------------------------------

void CourtTableView::onWalkoverP1Triggered()
{
  execWalkover(1);
}

//----------------------------------------------------------------------------

void CourtTableView::onWalkoverP2Triggered()
{
  execWalkover(2);
}

//----------------------------------------------------------------------------

void CourtTableView::onActionUndoCallTriggered()
{
  auto ma = getSelectedMatch();
  if (ma == nullptr) return;

  MatchMngr mm{db};
  mm.undoMatchCall(*ma);
}

//----------------------------------------------------------------------------

void CourtTableView::onActionAddCallTriggered()
{
  auto ma = getSelectedMatch();
  if (ma == nullptr) return;

  QList<QDateTime> callTimes = ma->getAdditionalCallTimes();
  if (callTimes.size() > MAX_NUM_ADD_CALL) return;

  auto co = ma->getCourt();
  assert(co != nullptr);

  QString callText = GuiHelpers::prepCall(*ma, *co, callTimes.size() + 1);

  int result = QMessageBox::question(this, tr("Repeat call"), callText);

  if (result == QMessageBox::Yes)
  {
    ma->addAddtionalCallTime();
    return;
  }
  QMessageBox::information(this, tr("Repeat call"), tr("Call cancled"));
}

//----------------------------------------------------------------------------

void CourtTableView::onActionSwapRefereeTriggered()
{
  auto ma = getSelectedMatch();
  if (ma == nullptr) return;

  // see if we can assign a new referee
  if (ma->canAssignReferee(REFEREE_ACTION::SWAP) != OK) return;

  // trigger the assign-umpire-procedure
  cmdAssignRefereeToMatch cmd{this, *ma, REFEREE_ACTION::SWAP};
  cmd.exec();
}

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

