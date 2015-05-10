/* 
 * File:   PlayerTableView.cpp
 * Author: volker
 * 
 * Created on March 17, 2014, 8:19 PM
 */

#include <QMessageBox>
#include "CourtTableView.h"
#include "MainFrame.h"
#include "CourtMngr.h"
#include "ui/GuiHelpers.h"

CourtTableView::CourtTableView(QWidget* parent)
  :QTableView(parent)
{
  // an empty model for clearing the table when
  // no tournament is open
  emptyModel = new QStringListModel();

  // prepare a proxy model to support sorting by columns
  sortedModel = new QSortFilterProxyModel();
  sortedModel->setSourceModel(emptyModel);
  setModel(sortedModel);

  connect(MainFrame::getMainFramePointer(), &MainFrame::tournamentOpened, this, &CourtTableView::onTournamentOpened);
  
  // react on selection changes in the court table view
  connect(selectionModel(),
    SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
    SLOT(onSelectionChanged(const QItemSelection&, const QItemSelection&)));

  // handle context menu requests
  setContextMenuPolicy(Qt::CustomContextMenu);
  connect(this, SIGNAL(customContextMenuRequested(const QPoint&)),
          this, SLOT(onContextMenuRequested(const QPoint&)));

  // define a delegate for drawing the court items
  itemDelegate = new CourtItemDelegate(this);
  itemDelegate->setProxy(sortedModel);
  setItemDelegate(itemDelegate);

  // setup the context menu and its actions
  initContextMenu();
}

//----------------------------------------------------------------------------
    
CourtTableView::~CourtTableView()
{
  delete emptyModel;
  delete sortedModel;
  //delete itemDelegate;
}

//----------------------------------------------------------------------------
    
void CourtTableView::onTournamentOpened(Tournament* _tnmt)
{
  tnmt = _tnmt;
  sortedModel->setSourceModel(Tournament::getCourtTableModel());
  sortedModel->sort(CourtTableModel::COURT_NUM_COL_ID, Qt::AscendingOrder);
  setEnabled(true);
  
  // connect signals from the Tournament and TeamMngr with my slots
  connect(tnmt, &Tournament::tournamentClosed, this, &CourtTableView::onTournamentClosed, Qt::DirectConnection);

  // resize columns and rows to content once (we do not want permanent automatic resizing)
  horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
  verticalHeader()->resizeSections(QHeaderView::ResizeToContents);

  //horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  //verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
}

//----------------------------------------------------------------------------
    
void CourtTableView::onTournamentClosed()
{
  // disconnect from all signals, because
  // the sending objects don't exist anymore
  disconnect(tnmt, &Tournament::tournamentClosed, this, &CourtTableView::onTournamentClosed);
  
  // invalidate the tournament handle and deactivate the view
  tnmt = nullptr;
  sortedModel->setSourceModel(emptyModel);
  setEnabled(false);
  
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
  return Tournament::getCourtMngr()->getCourtBySeqNum(selectedSourceRow);
}

//----------------------------------------------------------------------------

unique_ptr<Match> CourtTableView::getSelectedMatch() const
{
  auto co = getSelectedCourt();
  if (co == nullptr) return nullptr;

  return co->getMatch();
}

//----------------------------------------------------------------------------

void CourtTableView::onSelectionChanged(const QItemSelection& selectedItem, const QItemSelection& deselectedItem)
{
  resizeRowsToContents();
  for (auto item : selectedItem)
  {
    itemDelegate->setSelectedRow(item.top());
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

  // create sub-actions for the walkover-selection
  actWalkoverP1 = new QAction("P1", this);  // this is just a dummy
  actWalkoverP2 = new QAction("P2", this);  // this is just a dummy

  // link actions to slots
  connect(actAddCourt, SIGNAL(triggered()), this, SLOT(onActionAddCourtTriggered()));
  connect(actWalkoverP1, SIGNAL(triggered(bool)), this, SLOT(onWalkoverP1Triggered()));
  connect(actWalkoverP2, SIGNAL(triggered(bool)), this, SLOT(onWalkoverP2Triggered()));

  // create the context menu and connect it to the actions
  contextMenu = unique_ptr<QMenu>(new QMenu());
  contextMenu->addAction(actFinishMatch);
  walkoverSelectionMenu = contextMenu->addMenu(tr("Walkover for..."));
  walkoverSelectionMenu->addAction(actWalkoverP1);
  walkoverSelectionMenu->addAction(actWalkoverP2);
  contextMenu->addAction(actUndoCall);
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

void CourtTableView::execWalkover(int playerNum) const
{
  auto ma = getSelectedMatch();
  if (ma == nullptr) return;
  if ((playerNum != 1) && (playerNum != 2)) return; // shouldn't happen
  GuiHelpers::execWalkover(*ma, playerNum);
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
  CourtMngr* cm = Tournament::getCourtMngr();

  int nextCourtNum = cm->getHighestUnusedCourtNumber();

  ERR err;
  cm->createNewCourt(nextCourtNum, QString::number(nextCourtNum), &err);

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
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

