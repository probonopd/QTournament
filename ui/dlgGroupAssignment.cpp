/*
 * File:   dlgGroupAssignment.cpp
 * Author: volker
 *
 * Created on September 7, 2014, 6:28 PM
 */

#include "dlgGroupAssignment.h"

#include <QDateTime>
#include <QMessageBox>

dlgGroupAssignment::dlgGroupAssignment(Category* _cat)
:cfg(KO_Config(QUARTER, false))    // dummy, just for formal initialization
{
  ui.setupUi(this);
  cat = _cat;
  cfg = KO_Config(cat->getParameter(GROUP_CONFIG).toString());
  
  // fill all group lists initially with random assignments
  onBtnRandomizeClicked();
}

//----------------------------------------------------------------------------

dlgGroupAssignment::~dlgGroupAssignment()
{
}

//----------------------------------------------------------------------------

QList<PlayerPairList> dlgGroupAssignment::getRandomizedPlayerPairListList()
{
  PlayerPairList ppList = cat->getPlayerPairs();
  
  QList<PlayerPairList> result;
  
  // initialize the random number generator
  qsrand(QDateTime::currentDateTime().toTime_t());
  
  // each list item is a list of player pair for one specific group
  
  // fill the groups, one by one
  int grpIndex = 0;
  GroupDefList gdl = cfg.getGroupDefList();
  for (int grpDefIndex=0; grpDefIndex < gdl.count(); grpDefIndex++)
  {
    GroupDef grpDef = gdl.at(grpDefIndex);
    for (int innerCount=0; innerCount < grpDef.getNumGroups(); innerCount++)
    {
      PlayerPairList grpMemberList;
      for (int memberCount=0; memberCount < grpDef.getGroupSize(); memberCount++)
      {
	int playerPairIndex = qrand() % ppList.count();
	grpMemberList.append(ppList.at(playerPairIndex));
	ppList.removeAt(playerPairIndex);
      }
      result.append(grpMemberList);
      grpIndex++;
    }
  }
  
  return result;
}

//----------------------------------------------------------------------------

void dlgGroupAssignment::done(int result)
{
  QDialog::done(result);
}

//----------------------------------------------------------------------------

void dlgGroupAssignment::onBtnRandomizeClicked()
{
  ui.grpWidget->setup(getRandomizedPlayerPairListList());
}

//----------------------------------------------------------------------------

void dlgGroupAssignment::onBtnSwapClicked()
{
  ui.grpWidget->swapSelectedPlayers();
}

//----------------------------------------------------------------------------

QList<PlayerPairList> dlgGroupAssignment::getGroupAssignments()
{
  return ui.grpWidget->getGroupAssignments();
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

