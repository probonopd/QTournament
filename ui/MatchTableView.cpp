/* 
 * File:   PlayerTableView.cpp
 * Author: volker
 * 
 * Created on March 17, 2014, 8:19 PM
 */

#include <QMessageBox>

#include "MatchTableView.h"
#include "MainFrame.h"

#include "CourtMngr.h"
#include "Court.h"

MatchTableView::MatchTableView(QWidget* parent)
  :QTableView(parent)
{
  // an empty model for clearing the table when
  // no tournament is open
  emptyModel = new QStringListModel();

  // prepare a proxy model to support sorting by columns
  sortedModel = new QSortFilterProxyModel();
  sortedModel->setSourceModel(emptyModel);
  setModel(sortedModel);

  connect(MainFrame::getMainFramePointer(), &MainFrame::tournamentOpened, this, &MatchTableView::onTournamentOpened);
  
  // react on selection changes in the match table view
  connect(selectionModel(),
    SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
    SLOT(onSelectionChanged(const QItemSelection&, const QItemSelection&)));

  // handle context menu requests
  setContextMenuPolicy(Qt::CustomContextMenu);
  connect(this, SIGNAL(customContextMenuRequested(const QPoint&)),
          this, SLOT(onContextMenuRequested(const QPoint&)));

  // define a delegate for drawing the match items
  itemDelegate = new MatchItemDelegate(this);
  itemDelegate->setProxy(sortedModel);
  setItemDelegate(itemDelegate);

  // setup the context menu
  prepContextMenu();
}

//----------------------------------------------------------------------------
    
MatchTableView::~MatchTableView()
{
  delete emptyModel;
  delete sortedModel;
  delete itemDelegate;
}

//----------------------------------------------------------------------------
    
void MatchTableView::onTournamentOpened(Tournament* _tnmt)
{
  tnmt = _tnmt;
  sortedModel->setSourceModel(Tournament::getMatchTableModel());
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
  setEnabled(true);
  
  // connect signals from the Tournament and TeamMngr with my slots
  connect(tnmt, &Tournament::tournamentClosed, this, &MatchTableView::onTournamentClosed, Qt::DirectConnection);

  // resize columns and rows to content once (we do not want permanent automatic resizing)
  horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
  verticalHeader()->resizeSections(QHeaderView::ResizeToContents);

  horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
}

//----------------------------------------------------------------------------
    
void MatchTableView::onTournamentClosed()
{
  // disconnect from all signals, because
  // the sending objects don't exist anymore
  disconnect(tnmt, &Tournament::tournamentClosed, this, &MatchTableView::onTournamentClosed);
  
  // invalidate the tournament handle and deactivate the view
  tnmt = nullptr;
  sortedModel->setSourceModel(emptyModel);
  setEnabled(false);
  
}

//----------------------------------------------------------------------------

unique_ptr<Match> MatchTableView::getSelectedMatch()
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
  return Tournament::getMatchMngr()->getMatchBySeqNum(selectedSourceRow);
}

//----------------------------------------------------------------------------

void MatchTableView::onSelectionChanged(const QItemSelection& selectedItem, const QItemSelection& deselectedItem)
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

  // update the availability of actions, depending on the selected match
  auto ma = getSelectedMatch();
  if (ma == nullptr) return;  // shouldn't happen
  actWalkover->setEnabled(ma->getState() == STAT_MA_READY);
  actCourtSubmenu->setEnabled(ma->getState() == STAT_MA_READY);
}

//----------------------------------------------------------------------------

void MatchTableView::updateSelectionAfterDataChange()
{
  QModelIndexList indexes = selectionModel()->selection().indexes();
  if (indexes.count() == 0)
  {
    return;
  }
  resizeRowsToContents();
  int selectedTargetRow = indexes.at(0).row();
  itemDelegate->setSelectedRow(selectedTargetRow);
  resizeRowToContents(selectedTargetRow);
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

  // show the context menu
  updateCourtSelectionMenu();
  QAction* selectedItem = contextMenu->exec(globalPos);

  if (selectedItem == nullptr) return; // user canceled

  QMessageBox::information(this, "ksdjf", "action = " + selectedItem->text());
}

//----------------------------------------------------------------------------

void MatchTableView::prepContextMenu()
{
  // prepare all actions
  actWalkover = new QAction(tr("Walkover"), this);
  actPostponeMatch = new QAction(tr("Postpone"), this);

  // create the context menu and connect it to the actions
  contextMenu = unique_ptr<QMenu>(new QMenu());
  contextMenu->addAction(actWalkover);
  contextMenu->addAction(actPostponeMatch);
  contextMenu->addSeparator();
  courtSelectionMenu = contextMenu->addMenu(tr("Call match on court..."));
  actCourtSubmenu = courtSelectionMenu->menuAction();
  updateCourtSelectionMenu();
}

//----------------------------------------------------------------------------

void MatchTableView::updateCourtSelectionMenu()
{
  courtSelectionMenu->clear();


  // completely re-build the list of available courts


  CourtMngr* cm = Tournament::getCourtMngr();
  if (cm == nullptr) return;   // occurs if we're launching the application without a tournament loaded yet


  QStringList availCourtNum;
  for (auto co : cm->getAllCourts())
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
}

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

