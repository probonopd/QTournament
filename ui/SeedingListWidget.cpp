/*
 *    This is QTournament, a badminton tournament management program.
 *    Copyright (C) 2014 - 2017  Volker Knollmann
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
#include <iostream>

#include "PlayerMngr.h"
#include "SeedingListWidget.h"

using namespace QTournament;

SeedingListWidget::SeedingListWidget(QWidget* parent)
  :GuiHelpers::AutoSizingTableWidget(
     {
       {tr("Player"), 3, -1, -1},
       {tr("Rank"), 1, -1, -1}
     },
     parent)
{
  setRubberBandCol(IdxColGroup);

  // create a new delegate for the pair item column only;
  // do not use the delegate management of the parent class
  // because that uses one common delegate for all columns
  //
  // we also don't need to switch the delegate when we open/close
  // a database, because the delegate works indepedently from the database
  pairColumnDelegate = std::make_unique<PairItemDelegate>(nullptr, true);
  setItemDelegateForColumn(0, pairColumnDelegate.get());
}

//----------------------------------------------------------------------------

SeedingListWidget::~SeedingListWidget()
{
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
  return ((currentRow() >= 0) && (currentRow() < (rowCount() - 1)));
}

//----------------------------------------------------------------------------

void SeedingListWidget::shufflePlayers(int fromIndex)
{
  // nothing to shuffle if the fromIndex doesn't point
  // at least to the second to last item
  if (fromIndex > (rowCount() - 2)) return;

  // set a lower boundary to fromIndex so that we can
  // use it as an iterator index
  if (fromIndex < 0) fromIndex = 0;

  // shuffle the seeding list
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
  setCurrentCell(selectedListIndex-1, 0);
}

//----------------------------------------------------------------------------

void SeedingListWidget::moveSelectedPlayerDown()
{
  if (!(canMoveSelectedPlayerDown())) return;

  int selectedListIndex = currentRow();
  swapListItems(selectedListIndex, selectedListIndex+1);

  // move the selection down as well
  setCurrentCell(selectedListIndex+1, 0);
}

//----------------------------------------------------------------------------

void SeedingListWidget::warpSelectedPlayerTo(int targetRow)
{
  int selectedRow = currentRow();
  if (selectedRow < 0) return;
  if ((targetRow < 0) || (targetRow >= rowCount())) return;
  if (targetRow == selectedRow) return;

  // take the item from its current row
  // and insert it at the target row
  auto it = seedList.begin();
  std::advance(it, selectedRow);
  auto tmp = *it;
  seedList.erase(it);
  it = seedList.begin();
  std::advance(it, targetRow);
  seedList.insert(it, tmp);

  clearListAndFillFromSeed(seedList);

  /*
  auto i = takeItem(selectedRow, 0);
  setItem(targetRow, 0, i);
  */

  // let the selection follow the item
  setCurrentCell(targetRow, 0);
}

//----------------------------------------------------------------------------

std::vector<int> SeedingListWidget::getSeedList() const
{
  std::vector<int> result;

  for (int row = 0; row < rowCount(); ++row)
  {
    auto i = item(row, 0);
    if (i == nullptr) continue;   // shouldn't happen
    result.push_back(i->data(Qt::UserRole).toInt());
  }

  return result;
}

//----------------------------------------------------------------------------

void SeedingListWidget::swapListItems(int row1, int row2)
{
  if ((row1 < 0) || (row1 > (rowCount() - 1))) return;
  if ((row2 < 0) || (row2 > (rowCount() - 1))) return;
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
  auto item2 = takeItem(row2, 0);
  auto item1 = takeItem(row1, 0);

  // insert the item at the lower index first because
  // that repairs / restores the index count for the
  // second item
  setItem(row1, 0, item2);
  setItem(row2, 0, item1);

  // update the original seed list as well
  auto tmp = seedList[row2];
  seedList[row2] = seedList[row1];
  seedList[row1] = tmp;

  for (const auto& ai : seedList)
  {
    std::cout << ai.pairName.toStdString() << std::endl;
  }
  std::cout << std::endl;
}

//----------------------------------------------------------------------------

void SeedingListWidget::clearListAndFillFromSeed(const std::vector<AnnotatedSeedEntry>& seed)
{
  int oldSelection = currentRow();

  clear();
  setRowCount(0);

  int row = 0;
  for (const auto& sle : seed)
  {
    insertRow(row);
    setRowHeight(row, PairItemDelegate::PairItemRowHeight);

    QTableWidgetItem* newItem = new QTableWidgetItem(sle.pairName);
    newItem->setData(Qt::UserRole, sle.playerPairId);
    newItem->setData(PairItemDelegate::PairNameRole, sle.pairName);
    newItem->setData(PairItemDelegate::TeamNameRole, sle.teamName);
    newItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    setItem(row, IdxColName, newItem);

    if (!sle.groupHint.isEmpty())
    {
      newItem = new QTableWidgetItem(sle.groupHint);
      newItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
      setItem(row, IdxColGroup, newItem);
    }

    ++row;
  }
  seedList = seed;

  // restore the old selection, if necessary and possible
  if ((!(seedList.empty())) && (oldSelection >= 0))
  {
    if (oldSelection < seedList.size())
    {
      setCurrentCell(oldSelection, 0);
    } else {
      setCurrentCell(seedList.size() - 1, 0);
    }
  }
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

