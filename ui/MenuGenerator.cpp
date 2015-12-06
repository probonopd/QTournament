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

#include <memory>

#include <QMenu>
#include <QAction>

#include "Tournament.h"
#include "MenuGenerator.h"
#include "CatMngr.h"

using namespace QTournament;

void MenuGenerator::allCategories(QMenu* targetMenu)
{
  if (targetMenu == nullptr) return;

  // remove all previous menu contents
  targetMenu->clear();

  auto tnmt = Tournament::getActiveTournament();
  auto cm = tnmt->getCatMngr();

  // generate a sorted list of all categories
  auto allCats = cm->getAllCategories();
  std::sort(allCats.begin(), allCats.end(), CatMngr::getCategorySortFunction_byName());

  // create a new action for each category and add it to the menu
  for (const Category& cat : allCats)
  {
    auto newAction = targetMenu->addAction(cat.getName());
    newAction->setData(cat.getId());
  }
}

//----------------------------------------------------------------------------

bool MenuGenerator::isActionInMenu(const QMenu* m, const QAction* a)
{
  if ((m == nullptr) || (a == nullptr))
  {
    return false;
  }

  for (auto menuAction : m->actions())
  {
    if (menuAction == a) return true;
  }

  return false;
}
