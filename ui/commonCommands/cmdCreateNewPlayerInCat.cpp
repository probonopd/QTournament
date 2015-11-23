#include <QObject>
#include <QMessageBox>

#include "Tournament.h"
#include "cmdCreateNewPlayerInCat.h"
#include "ui/dlgEditPlayer.h"
#include "ui/DlgPickPlayerSex.h"
#include "cmdCreatePlayerFromDialog.h"

cmdCreateNewPlayerInCat::cmdCreateNewPlayerInCat(QWidget* p, const Category& _cat)
  :AbstractCommand(p), cat(_cat)
{

}

//----------------------------------------------------------------------------

ERR cmdCreateNewPlayerInCat::exec()
{
  // check if can add more players anyway
  if (!(cat.canAddPlayers()))
  {
    QString msg = tr("Can't add any more player to the category. It's either\n");
    msg += tr("full or already started.");
    QMessageBox::warning(parentWidget, tr("Create new player in category"), msg);

    return CATEGORY_CLOSED_FOR_MORE_PLAYERS;
  }

  // show a dialog for selecting the new player's sex
  DlgPickPlayerSex dlgSex{parentWidget, QString()};
  if (dlgSex.exec() != QDialog::Accepted)
  {
    return OK;
  }
  SEX selectedSex = dlgSex.getSelectedSex();

  // check if we can add a player of the selected
  // sex to the category
  if (cat.getAddState(selectedSex) != CAN_JOIN)
  {
    QString msg = tr("Can't add a %1 to the category.");
    msg = msg.arg((selectedSex == M) ? "male player" : "female player");
    QMessageBox::warning(parentWidget, tr("Create new player in category"), msg);

    return INVALID_SEX;
  }

  // prepare a dialog for creating a new player
  DlgEditPlayer dlgCreate{parentWidget, selectedSex, cat};

  // let an external command do the rest of the work
  cmdCreatePlayerFromDialog cmd{parentWidget, &dlgCreate};
  return cmd.exec();
}

