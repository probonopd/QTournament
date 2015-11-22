#ifndef MENUGENERATOR_H
#define MENUGENERATOR_H

#include <memory>

#include <QMenu>

using namespace std;

class MenuGenerator
{
public:
  MenuGenerator() = delete;

  static void allCategories(QMenu* targetMenu);

  static bool isActionInMenu(const QMenu* m, const QAction* a);
};

#endif // MENUGENERATOR_H
