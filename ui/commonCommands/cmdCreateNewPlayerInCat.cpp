#include <QObject>
#include <QMessageBox>

#include "Tournament.h"
#include "cmdCreateNewPlayerInCat.h"
#include "ui/dlgEditPlayer.h"
#include "ui/DlgPickPlayerSex.h"

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
  DlgPickPlayerSex dlgSex{parentWidget};
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

  // show a dialog for creating a new player
  DlgEditPlayer dlgCreate{parentWidget, selectedSex, cat};
  if (dlgCreate.exec() != QDialog::Accepted)
  {
    return OK;
  }

  // we can be sure that all selected data in the dialog
  // is valid. That has been checked before the dialog
  // returns with "Accept". So we can directly step
  // into the creation of the new player
  ERR e = Tournament::getPlayerMngr()->createNewPlayer(
                                                       dlgCreate.getFirstName(),
                                                       dlgCreate.getLastName(),
                                                       dlgCreate.getSex(),
                                                       dlgCreate.getTeam().getName()
                                                       );

  if (e != OK)
  {
    QString msg = tr("Something went wrong when inserting the player. This shouldn't happen.");
    msg += tr("For the records: error code = ") + QString::number(static_cast<int>(e));
    QMessageBox::warning(parentWidget, tr("WTF??"), msg);
    return e;
  }
  Player p = Tournament::getPlayerMngr()->getPlayer(dlgCreate.getFirstName(), dlgCreate.getLastName());

  // assign the player to the selected categories
  //
  // we can be sure that all selected categories in the dialog
  // are valid. That has been checked upon creation of the "selectable"
  // category entries. So we can directly step
  // into the assignment of the categories
  CatMngr* cmngr = Tournament::getCatMngr();

  QHash<Category, bool> catSelection = dlgCreate.getCategoryCheckState();
  QHash<Category, bool>::const_iterator it = catSelection.constBegin();

  while (it != catSelection.constEnd()) {
    if (it.value()) {
      Category cat = it.key();
      ERR e = cmngr->addPlayerToCategory(p, cat);

      if (e != OK) {
        QString msg = tr("Something went wrong when adding the player to a category. This shouldn't happen.");
        msg += tr("For the records: error code = ") + QString::number(static_cast<int> (e));
        QMessageBox::warning(parentWidget, tr("WTF??"), msg);
      }
    }
    ++it;
  }

  return OK;
}

