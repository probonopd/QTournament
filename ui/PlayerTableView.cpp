/* 
 * File:   PlayerTableView.cpp
 * Author: volker
 * 
 * Created on March 17, 2014, 8:19 PM
 */

#include "PlayerTableView.h"
#include "MainFrame.h"

PlayerTableView::PlayerTableView(QWidget* parent)
:QTableView(parent)
{
  // an empty model for clearing the table when
  // no tournament is open
  emptyModel = new QStringListModel();
  
  // prepare a proxy model to support sorting by columns
  sortedModel = new QSortFilterProxyModel();
  sortedModel->setSourceModel(emptyModel);
  setModel(sortedModel);

  connect(MainFrame::getMainFramePointer(), &MainFrame::tournamentOpened, this, &PlayerTableView::onTournamentOpened);
  
  // handle context menu requests
  setContextMenuPolicy(Qt::CustomContextMenu);
  connect(this, SIGNAL(customContextMenuRequested(const QPoint&)),
          this, SLOT(onContextMenuRequested(const QPoint&)));

  // define a delegate for drawing the player items
  itemDelegate = new PlayerItemDelegate(this);
  itemDelegate->setProxy(sortedModel);
  setItemDelegate(itemDelegate);
  
  // setup the context menu
  initContextMenu();

}

//----------------------------------------------------------------------------
    
PlayerTableView::~PlayerTableView()
{
  delete emptyModel;
  delete sortedModel;
  delete itemDelegate;
}

//----------------------------------------------------------------------------

unique_ptr<Player> PlayerTableView::getSelectedPlayer() const
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
  return Tournament::getPlayerMngr()->getPlayerBySeqNum(selectedSourceRow);
}

//----------------------------------------------------------------------------
    
void PlayerTableView::onTournamentOpened(Tournament* _tnmt)
{
  tnmt = _tnmt;
  sortedModel->setSourceModel(Tournament::getPlayerTableModel());
  setEnabled(true);
  
  // connect signals from the Tournament and TeamMngr with my slots
  connect(tnmt, &Tournament::tournamentClosed, this, &PlayerTableView::onTournamentClosed);
  
  // resize columns and rows to content once (we do not want permanent automatic resizing)
  horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
  verticalHeader()->resizeSections(QHeaderView::ResizeToContents);
}

//----------------------------------------------------------------------------
    
void PlayerTableView::onTournamentClosed()
{
  // disconnect from all signals, because
  // the sending objects don't exist anymore
  disconnect(tnmt, &Tournament::tournamentClosed, this, &PlayerTableView::onTournamentClosed);
  
  // invalidate the tournament handle and deactivate the view
  tnmt = nullptr;
  sortedModel->setSourceModel(emptyModel);
  setEnabled(false);
  
}

//----------------------------------------------------------------------------

QModelIndex PlayerTableView::mapToSource(const QModelIndex &proxyIndex)
{
  return sortedModel->mapToSource(proxyIndex);
}

//----------------------------------------------------------------------------

void PlayerTableView::onContextMenuRequested(const QPoint& pos)
{
  // map from scroll area coordinates to global widget coordinates
  QPoint globalPos = viewport()->mapToGlobal(pos);

  // resolve the click coordinates to the table row
  int clickedRow = rowAt(pos.y());
  int clickedCol = columnAt(pos.x());
  bool isRowClicked = ((clickedRow >= 0) && (clickedCol >= 0));

  // if no row is clicked, we may only add a player.
  // in other cases, we may also try to edit or delete
  // players
  actAddPlayer->setEnabled(true);   // always possible
  actEditPlayer->setEnabled(isRowClicked);
  actShowNextMatchesForPlayer->setEnabled(isRowClicked);
  actRemovePlayer->setEnabled(isRowClicked);

  // show the context menu
  QAction* selectedItem = contextMenu->exec(globalPos);
}

//----------------------------------------------------------------------------

void PlayerTableView::initContextMenu()
{
  // prepare all actions
  actAddPlayer = new QAction(tr("Add Player..."), this);
  actEditPlayer = new QAction(tr("Edit Player..."), this);
  actRemovePlayer = new QAction(tr("Remove Player..."), this);
  actShowNextMatchesForPlayer = new QAction(tr("Show next Matches..."), this);

  // create the context menu and connect it to the actions
  contextMenu = unique_ptr<QMenu>(new QMenu());
  contextMenu->addAction(actAddPlayer);
  contextMenu->addAction(actEditPlayer);
  contextMenu->addSeparator();
  contextMenu->addAction(actShowNextMatchesForPlayer);
  contextMenu->addSeparator();
  contextMenu->addAction(actRemovePlayer);
}

//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

