#include <QObject>
#include <QMessageBox>

#include "Tournament.h"
#include "cmdImportSinglePlayerFromExternalDatabase.h"
#include "ui/DlgImportPlayer.h"
#include "ui/DlgPickPlayerSex.h"
#include "ui/dlgEditPlayer.h"

cmdImportSinglePlayerFromExternalDatabase::cmdImportSinglePlayerFromExternalDatabase(QWidget* p)
  :AbstractCommand(p)
{

}

//----------------------------------------------------------------------------

ERR cmdImportSinglePlayerFromExternalDatabase::exec()
{
  // make sure we have an external database open
  PlayerMngr* pm = Tournament::getPlayerMngr();
  if (!(pm->hasExternalPlayerDatabaseOpen()))
  {
    QString msg = tr("No valid database for player import open.");
    QMessageBox::warning(parentWidget, tr("Import player"), msg);
    return EPD__NOT_OPENED;
  }

  ExternalPlayerDB* extDb = pm->getExternalPlayerDatabaseHandle();

  // show a search-and-select dialog
  DlgImportPlayer dlg{parentWidget, extDb};
  if (dlg.exec() != QDialog::Accepted)
  {
    return OK;
  }

  // get the selected ID
  int extId = dlg.getSelectedExternalPlayerId();
  if (extId < 0)
  {
    QString msg = tr("No valid player selection found");
    QMessageBox::warning(parentWidget, tr("Import player"), msg);
    return INVALID_ID;
  }

  // get the selected player from the database
  auto extPlayer = extDb->getPlayer(extId);
  if (extPlayer == nullptr)
  {
    QString msg = tr("No valid player selection found");
    QMessageBox::warning(parentWidget, tr("Import player"), msg);
    return INVALID_ID;
  }

  // if the player has no valid sex assigned,
  // show a selection dialog
  upExternalPlayerDatabaseEntry finalPlayerData;
  if (extPlayer->getSex() == DONT_CARE)
  {
    DlgPickPlayerSex dlgPickSex{parentWidget};
    if (dlgPickSex.exec() != QDialog::Accepted)
    {
      return OK;
    }

    finalPlayerData = make_unique<ExternalPlayerDatabaseEntry>(
          extPlayer->getFirstname(),
          extPlayer->getLastname(),
          dlgPickSex.getSelectedSex()
          );
  } else {
    finalPlayerData = std::move(extPlayer);
  }

  // now we have all player details, so we can show
  // the insert-player-dialog to allow the user the
  // selection of a team and possible category assignments
  DlgEditPlayer dlgCreate{parentWidget, *finalPlayerData};
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

