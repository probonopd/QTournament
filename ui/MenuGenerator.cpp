/*
 *    This is QTournament, a badminton tournament management program.
 *    Copyright (C) 2014 - 2016  Volker Knollmann
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

#include "MenuGenerator.h"
#include "CatMngr.h"

using namespace QTournament;

void MenuGenerator::allCategories(TournamentDB* db, QMenu* targetMenu)
{
  CatMngr cm{db};
  fromCategoryList(db, cm.getAllCategories(), targetMenu);
}

//----------------------------------------------------------------------------

void MenuGenerator::fromCategoryList(TournamentDB* db, const CategoryList& catList, QMenu* targetMenu)
{
  if (targetMenu == nullptr) return;

  // remove all previous menu contents
  targetMenu->clear();

  // stop here if the list of categories is empty
  if (catList.empty()) return;

  // generate a sorted list of the requested categories
  CategoryList cl{catList};
  std::sort(cl.begin(), cl.end(), CatMngr::getCategorySortFunction_byName());

  // create a new action for each category and add it to the menu
  for (const Category& cat : cl)
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
