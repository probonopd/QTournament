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

#include <QDateTime>
#include <QMessageBox>

#include "dlgGroupAssignment.h"
#include "Category.h"

dlgGroupAssignment::dlgGroupAssignment(TournamentDB* _db, QWidget* p, Category& _cat)
  :QDialog(p), db(_db),
    cfg(KO_Config(QUARTER, false)), cat(_cat)    // dummy, just for formal initialization
{
  ui.setupUi(this);
  cfg = KO_Config(cat.getParameter(GROUP_CONFIG).toString());

  ui.grpWidget->setDatabase(db);

  // set the window title
  setWindowTitle(tr("Group assignment for ") + cat.getName());
  
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
  PlayerPairList ppList = cat.getPlayerPairs();
  
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
        int playerPairIndex = qrand() % ppList.size();
        grpMemberList.push_back(ppList.at(playerPairIndex));
        ppList.erase(ppList.begin() + playerPairIndex);
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
  ui.grpWidget->setup(db, getRandomizedPlayerPairListList());
}

//----------------------------------------------------------------------------

void dlgGroupAssignment::onBtnSwapClicked()
{
  ui.grpWidget->swapSelectedPlayers();
}

//----------------------------------------------------------------------------

vector<PlayerPairList> dlgGroupAssignment::getGroupAssignments()
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

