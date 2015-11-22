#include <QObject>
#include <QMessageBox>

#include "Tournament.h"
#include "cmdBulkRemovePlayersFromCat.h"
#include "ui/DlgSelectPlayer.h"

cmdBulkRemovePlayersFromCategory::cmdBulkRemovePlayersFromCategory(QWidget* p, const Category& _cat)
  :AbstractCommand(p), cat(_cat)
{

}

//----------------------------------------------------------------------------

ERR cmdBulkRemovePlayersFromCategory::exec()
{
  // show a dialog for selecting the players
  DlgSelectPlayer dlg{parentWidget, DlgSelectPlayer::DLG_CONTEXT::REMOVE_FROM_CATEGORY, &cat};
  if (dlg.exec() != QDialog::Accepted)
  {
    return OK;
  }

  // remove all selected players from the category
  auto cm = Tournament::getCatMngr();
  for (const Player& pl : dlg.getSelectedPlayers())
  {
    ERR err = cm->removePlayerFromCategory(pl, cat);

    if (err != OK)
    {
      QString msg = tr("Could not remove player\n\n");
      msg += "     " + pl.getDisplayName_FirstNameFirst() + "\n\n";
      msg += "from the category.";
      QMessageBox::warning(parentWidget, tr("Remove players from category"), msg);
    }
  }

  return OK;
}

