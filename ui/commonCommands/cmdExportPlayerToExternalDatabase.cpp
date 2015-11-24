#include <QObject>
#include <QMessageBox>

#include "Tournament.h"
#include "cmdExportPlayerToExternalDatabase.h"

cmdExportPlayerToExternalDatabase::cmdExportPlayerToExternalDatabase(QWidget* p, const Player& _pl)
  :AbstractCommand(p), pl(_pl)
{

}

//----------------------------------------------------------------------------

ERR cmdExportPlayerToExternalDatabase::exec()
{
  // make sure we have an external database open
  PlayerMngr* pm = Tournament::getPlayerMngr();
  if (!(pm->hasExternalPlayerDatabaseOpen()))
  {
    QString msg = tr("No valid database for player export open.");
    QMessageBox::warning(parentWidget, tr("Export player"), msg);
    return EPD__NOT_OPENED;
  }

  ExternalPlayerDB* extDb = pm->getExternalPlayerDatabaseHandle();

  ERR err = pm->exportPlayerToExternalDatabase(pl);
  if (err == OK) return OK;

  QString msg;
  switch (err)
  {
  case EPD__CREATION_FAILED:
    msg = tr("Could not export the player data to the database.");
    break;

  default:
    msg = tr("An undefined error occurred. The player has not\n");
    msg += tr("been exported to the external database");
  }

  QMessageBox::warning(parentWidget, tr("Export player"), msg);
  return err;
}

