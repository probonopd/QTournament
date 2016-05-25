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

#ifndef MENUGENERATOR_H
#define MENUGENERATOR_H

#include <memory>

#include <QMenu>

#include "TournamentDB.h"
#include "Category.h"

using namespace std;
using namespace QTournament;

class MenuGenerator
{
public:
  MenuGenerator() = delete;

  static void allCategories(TournamentDB* db, QMenu* targetMenu);
  static void fromCategoryList(TournamentDB* db, const CategoryList& catList, QMenu* targetMenu);

  static bool isActionInMenu(const QMenu* m, const QAction* a);
};

#endif // MENUGENERATOR_H
