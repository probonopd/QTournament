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

#include <algorithm>

#include "PlayerMngr.h"
#include "SeedingListWidget.h"


SeedingListWidget::SeedingListWidget(QWidget* parent)
  :QListWidget(parent), pairDelegate(nullptr), defaultDelegate(itemDelegate())
{
}

//----------------------------------------------------------------------------

SeedingListWidget::~SeedingListWidget()
{
  if (defaultDelegate != nullptr) delete defaultDelegate;
}

//----------------------------------------------------------------------------

int SeedingListWidget::getSelectedItemIndex() const
{
  return currentRow();
}

//----------------------------------------------------------------------------

bool SeedingListWidget::canMoveSelectedPlayerUp() const
{
  return (currentRow() > 0);
}

//----------------------------------------------------------------------------

bool SeedingListWidget::canMoveSelectedPlayerDown() const
{
  return ((currentRow() >= 0) && (currentRow() < (count() - 1)));
}

//----------------------------------------------------------------------------

void SeedingListWidget::shufflePlayers(int fromIndex)
{
  // nothing to shuffle if the fromIndex doesn't point
  // at least to the second to last item
  if (fromIndex > (count() - 2)) return;

  // set a lower boundary to fromIndex so that we can
  // use it as an iterator index
  if (fromIndex < 0) fromIndex = 0;

  // shuffle the seeding list
  PlayerPairList seedList = getSeedList();
  std::random_shuffle(seedList.begin() + fromIndex, seedList.end());

  // and "re-set" the widget contents
  clearListAndFillFromSeed(seedList);
}

//----------------------------------------------------------------------------

void SeedingListWidget::moveSelectedPlayerUp()
{
  if (!(canMoveSelectedPlayerUp())) return;

  int selectedListIndex = currentRow();
  swapListItems(selectedListIndex, selectedListIndex-1);

  // move the selection up as well
  setCurrentRow(selectedListIndex-1);
}

//----------------------------------------------------------------------------

void SeedingListWidget::moveSelectedPlayerDown()
{
  if (!(canMoveSelectedPlayerDown())) return;

  int selectedListIndex = currentRow();
  swapListItems(selectedListIndex, selectedListIndex+1);

  // move the selection down as well
  setCurrentRow(selectedListIndex+1);
}

//----------------------------------------------------------------------------

void SeedingListWidget::warpSelectedPlayerTo(int targetRow)
{
  int selectedRow = currentRow();
  if (selectedRow < 0) return;
  if ((targetRow < 0) || (targetRow >= count())) return;
  if (targetRow == selectedRow) return;

  QListWidgetItem* i = takeItem(selectedRow);
  insertItem(targetRow, i);

  // let the selection follow the item
  setCurrentRow(targetRow);
}

//----------------------------------------------------------------------------

PlayerPairList SeedingListWidget::getSeedList() const
{
  PlayerPairList result;
  PlayerMngr pm{db};

  for (int row = 0; row < count(); ++row)
  {
    QListWidgetItem* i = item(row);
    if (i == nullptr) continue;   // shouldn't happen
    int pairId = i->data(Qt::UserRole).toInt();
    result.push_back(pm.getPlayerPair(pairId));
  }

  return result;
}

//----------------------------------------------------------------------------

void SeedingListWidget::swapListItems(int row1, int row2)
{
  if ((row1 < 0) || (row1 > (count() - 1))) return;
  if ((row2 < 0) || (row2 > (count() - 1))) return;
  if (row1 == row2) return;

  // make sure that row1 is always the lower row number
  if (row2 < row1)
  {
    int tmp = row2;
    row2 = row1;
    row1 = tmp;
  }

  // take the higher indexed item first, so that
  // the index of the other (lower) item doesn't change
  QListWidgetItem* item2 = takeItem(row2);
  QListWidgetItem* item1 = takeItem(row1);

  // insert the item at the lower index first because
  // that repairs / restores the index count for the
  // second item
  insertItem(row1, item2);
  insertItem(row2, item1);
}

//----------------------------------------------------------------------------

void SeedingListWidget::clearListAndFillFromSeed(const PlayerPairList& seed)
{
  int oldSelection = currentRow();

  clear();
  for (int cnt=0; cnt < seed.size(); ++cnt)
  {
    QListWidgetItem* lwi = new QListWidgetItem(this);
    lwi->setData(Qt::UserRole, seed.at(cnt).getPairId());
    lwi->setData(Qt::DisplayRole, seed.at(cnt).getDisplayName());
  }

  // restore the old selection, if necessary and possible
  if ((!(seed.empty())) && (oldSelection >= 0))
  {
    if (oldSelection < seed.size())
    {
      setCurrentRow(oldSelection);
    } else {
      setCurrentRow(seed.size() - 1);
    }
  }
}

//----------------------------------------------------------------------------

void SeedingListWidget::setDatabase(TournamentDB* _db)
{
  db = _db;

  // assign a delegate to the list widget for drawing the entries
  if (db != nullptr)
  {
    pairDelegate = make_unique<PairItemDelegate>(db, nullptr, true);
    setItemDelegate(pairDelegate.get());
  } else {
    setItemDelegate(defaultDelegate);
    pairDelegate.reset();
  }

  setEnabled(db != nullptr);
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

