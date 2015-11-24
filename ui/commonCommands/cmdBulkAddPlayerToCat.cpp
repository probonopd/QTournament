#include <QObject>
#include <QMessageBox>

#include "Tournament.h"
#include "cmdBulkAddPlayerToCat.h"
#include "ui/DlgSelectPlayer.h"

cmdBulkAddPlayerToCategory::cmdBulkAddPlayerToCategory(QWidget* p, const Category& _cat)
  :AbstractCommand(p), cat(_cat)
{

}

//----------------------------------------------------------------------------

ERR cmdBulkAddPlayerToCategory::exec()
{
  // check if can add more players anyway
  if (!(cat.canAddPlayers()))
  {
    QString msg = tr("Can't add any more player to the category. It's either\n");
    msg += tr("full or already started.");
    QMessageBox::warning(parentWidget, tr("Add players to category"), msg);

    return CATEGORY_CLOSED_FOR_MORE_PLAYERS;
  }

  // show a dialog for selecting the players
  DlgSelectPlayer dlg{parentWidget, DlgSelectPlayer::DLG_CONTEXT::ADD_TO_CATEGORY, &cat};
  if (dlg.exec() != QDialog::Accepted)
  {
    return OK;
  }

  // add all selected players to the category
  auto cm = Tournament::getCatMngr();
  for (const Player& pl : dlg.getSelectedPlayers())
  {
    ERR err = cm->addPlayerToCategory(pl, cat);

    if (err != OK)
    {
      QString msg = tr("Could not add player\n\n");
      msg += "     " + pl.getDisplayName_FirstNameFirst() + "\n\n";
      msg += tr("to the category.");
      QMessageBox::warning(parentWidget, tr("Add players to category"), msg);
    }
  }

  return OK;
}

