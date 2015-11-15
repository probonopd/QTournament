#include <QObject>
#include <QMessageBox>

#include "Tournament.h"
#include "cmdRemovePlayerFromCategory.h"
#include "CatMngr.h"

cmdRemovePlayerFromCategory::cmdRemovePlayerFromCategory(QWidget* p, const Player& _pl, const Category& _cat)
  :AbstractCommand(p), pl(_pl), cat(_cat)
{

}

//----------------------------------------------------------------------------

ERR cmdRemovePlayerFromCategory::exec()
{
  ERR err;
  auto cm = Tournament::getCatMngr();
  err = cm->removePlayerFromCategory(pl, cat);

  if (err == OK) return OK;

  QString msg;
  if (err == PLAYER_NOT_REMOVABLE_FROM_CATEGORY)
  {
    msg = tr("The player cannot be removed from the category anymore,\n");
    msg += tr("e.g., because the category has already been started");
  }
  else   // unspecific error
  {
    msg = tr("Something went wrong when removing the player from the category. This shouldn't happen.\n\n");
    msg += tr("For the records: error code = ") + QString::number(static_cast<int> (err));
  }

  QMessageBox::warning(parentWidget, tr("Remove player from category"), msg);

  return err;
}
