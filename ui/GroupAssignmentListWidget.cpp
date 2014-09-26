/*
 * File:   GroupAssignmentListWidget.cpp
 * Author: volker
 *
 * Created on September 3, 2014, 8:06 PM
 */

#include <QLabel>
#include <QScrollBar>
#include <QMessageBox>

#include "GroupAssignmentListWidget.h"

GroupAssignmentListWidget::GroupAssignmentListWidget(QWidget* parent)
:QWidget(parent)
{
  ui.setupUi(this);

  // clear all list widget pointers
  for (int i=0; i < MAX_GROUP_COUNT; i++) lwGroup[i] = 0;
  
  // set a flag that we are not yet fully initialized
  isInitialized = false;
  
  // prepare the scroll area for a resizable widget
  ui.scrollAreaWidgetContents->setLayout(ui.grid);
  ui.scrollArea->setWidgetResizable(true);
  
}

//----------------------------------------------------------------------------
    
GroupAssignmentListWidget::~GroupAssignmentListWidget()
{
}

//----------------------------------------------------------------------------

void GroupAssignmentListWidget::setup(QList<PlayerPairList> ppListList)
{
  int grpCount = ppListList.count();
  
  // determine the number of list widget in a row
  int colCount = getColCountForGroupCount(grpCount);
  if (colCount < 0) return;
  
  // delete old widgets, if existing
  if (isInitialized) teardown();
  
  // create and fill new widgets
  for (int i=0; i<grpCount; i++)
  {
    int r = i / colCount;
    int c = i % colCount;
    
    // create the list widget itself
    QListWidget* lw = new QListWidget(this);
    lw->setMinimumWidth(MIN_PLAYER_LIST_WIDTH);
    
    // create a label
    QLabel* la = new QLabel(tr("Group ")+QString::number(i+1)+":", this);
    
    // add label and list widget to the grid
    ui.grid->addWidget(la, 2*r, c, Qt::AlignHCenter);
    ui.grid->addWidget(lw, 2*r+1, c);
    lwGroup[i] = lw;
    laGroup[i] = la;
    
    // fill the newly created widget
    PlayerPairList ppList = ppListList.at(i);
    for (int cnt=0; cnt < ppList.count(); cnt++)
    {
      QListWidgetItem* lwi = new QListWidgetItem(lw);
      lwi->setData(Qt::UserRole, ppList.at(cnt).getPairId());
      lwi->setData(Qt::DisplayRole, ppList.at(cnt).getDisplayName());
    }
    
    // assign a delegate to the list widget for drawing the entries
    delegate[i] = new PairItemDelegate();
    lw->setItemDelegate(delegate[i]);
    
    // connect the row-selection-changed-signal
    QObject::connect(lw, SIGNAL(itemSelectionChanged()), this, SLOT(onRowSelectionChanged()));
  }
  
  // Initialize the selection
  // There are always at least two groups with at least one entry each.
  // So it's safe to select the first entries of the first two groups
  inhibitItemChangedSig = true;
  lwGroup[0]->setCurrentRow(0);
  lwGroup[1]->setCurrentRow(0);
  selectionQueue.enqueue(lwGroup[0]);
  selectionQueue.enqueue(lwGroup[1]);
  inhibitItemChangedSig = false;
  
  isInitialized = true;
}

//----------------------------------------------------------------------------
    
void GroupAssignmentListWidget::teardown()
{
  if (!isInitialized) return;
  
  // delete everything from the grid
  QLayoutItem *child;
  while ((child = ui.grid->takeAt(0)) != 0)
  {
    delete child;
  }
  
  // reset the pointer arrays
  for (int i=0; i < MAX_GROUP_COUNT; i++)
  {
    if (lwGroup[i] == 0) continue;  // unused entry
    delete lwGroup[i];  // this should be redundant to the "delete child" above but it works. Weird.
    delete laGroup[i];  // this should be redundant to the "delete child" above but it works. Weird.
    delete delegate[i];
    lwGroup[i] = 0;
    laGroup[i] = 0;
  }
  
  // clear all selection info
  selectionQueue.clear();
  
  isInitialized = false;
}

//----------------------------------------------------------------------------

int GroupAssignmentListWidget::getColCountForGroupCount(int grpCount)
{
  if (grpCount < 2) return -1;
  
  switch (grpCount)
  {
  case 2: return 2;
  case 3: return 3;
  case 4: return 2;
  case 5: return 3;
  case 6: return 3;
  case 7: return 4;
  case 8: return 4;
  case 9: return 5;
  case 10: return 5;
  case 11: return 6;
  case 12: return 6;
  case 13: return 6;
  case 14: return 6;
  case 15: return 6;
  case 16: return 6;
  case 17: return 6;
  case 18: return 6;
  case 19: return 7;
  case 20: return 7;
  case 21: return 7;
  }
  
  if (grpCount <= MAX_GROUP_COUNT) return 8;
  
  return -1;
}

//----------------------------------------------------------------------------

void GroupAssignmentListWidget::onRowSelectionChanged()
{
  // Avoid "recursive" calls to this slot by the selection modifications issued below
  if (inhibitItemChangedSig) return;
  
  if (!isInitialized) return;
  
  // determine which list box sent the signal
  QListWidget* senderWidget = (QListWidget*) QObject::sender();
  
  // if the selection change happened in one of the already selected 
  // list widgets, do nothing
  if ((senderWidget == selectionQueue.first()) || (senderWidget == selectionQueue.last())) return;
  
  // get the widget with the oldest selection and remove the selection
  QListWidget* oldestSelectionList = selectionQueue.dequeue();
  inhibitItemChangedSig = true;  // block "recursive" calls to this slot triggered by the next calls
  oldestSelectionList->clearSelection();
  inhibitItemChangedSig = false;  // re-enable this slot
  
  // enqueue the new widget
  selectionQueue.enqueue(senderWidget);
  }

//----------------------------------------------------------------------------

PlayerPairList GroupAssignmentListWidget::getSelectedPlayerPairs()
{
  PlayerPairList result;
  
  if (isInitialized)
  {
    // we blindly assume that there is at least one item selected. Must be. ;)
    int id1 = selectionQueue.first()->selectedItems()[0]->data(Qt::UserRole).toInt();
    int id2 = selectionQueue.last()->selectedItems()[0]->data(Qt::UserRole).toInt();
    
    result.append(Tournament::getPlayerMngr()->getPlayerPair(id1));
    result.append(Tournament::getPlayerMngr()->getPlayerPair(id2));
  }
  
  return result;
}

//----------------------------------------------------------------------------

void GroupAssignmentListWidget::swapSelectedPlayers()
{
  if (!isInitialized) return;
  
  QListWidgetItem* firstItem = selectionQueue.first()->selectedItems()[0];
  QListWidgetItem* secondItem = selectionQueue.last()->selectedItems()[0];
  
  // temp. store the contents of the first item
  int tmpPairId = firstItem->data(Qt::UserRole).toInt();
  QString tmpName = firstItem->data(Qt::DisplayRole).toString();
  
  // overwrite the contents of the first item with the data of the second item
  firstItem->setData(Qt::UserRole, secondItem->data(Qt::UserRole));
  firstItem->setData(Qt::DisplayRole, secondItem->data(Qt::DisplayRole));
  
  // overwrite the second item with the temp. data of the first
  secondItem->setData(Qt::UserRole, tmpPairId);
  secondItem->setData(Qt::DisplayRole, tmpName);
}

//----------------------------------------------------------------------------

QList<PlayerPairList> GroupAssignmentListWidget::getGroupAssignments()
{
  QList<PlayerPairList> result;
  if (!isInitialized) return result;

  PlayerMngr* pmngr = Tournament::getPlayerMngr();
  for (int i=0; i < MAX_GROUP_COUNT; i++)
  {
    if (lwGroup[i] == 0) continue;

    QListWidget* lw = lwGroup[i];
    PlayerPairList ppList;

    for (int cnt; cnt < lw->count(); cnt++)
    {
      QListWidgetItem* it = lw->item(cnt);
      int ppId = it->data(Qt::UserRole).toInt();
      ppList.append(pmngr->getPlayerPair(ppId));
    }

    result.append(ppList);
  }

  return result;
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
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    
