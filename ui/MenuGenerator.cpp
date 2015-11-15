
#include <memory>

#include <QMenu>
#include <QAction>

#include "Tournament.h"
#include "MenuGenerator.h"
#include "CatMngr.h"

void MenuGenerator::allCategories(QMenu* targetMenu)
{
  if (targetMenu == nullptr) return;

  // remove all previous menu contents
  targetMenu->clear();

  auto cm = Tournament::getCatMngr();

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
