/*
 * File:   GroupAssignmentListWidget.cpp
 * Author: volker
 *
 * Created on September 3, 2014, 8:06 PM
 */

#include <QLabel>
#include <QScrollBar>
#include <qt/QtWidgets/qmessagebox.h>

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
  //ui.scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
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
      lw->addItem(ppList.at(cnt).getDisplayName());
    }
  }
  
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
    lwGroup[i] = 0;
    laGroup[i] = 0;
  }
  
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
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    
