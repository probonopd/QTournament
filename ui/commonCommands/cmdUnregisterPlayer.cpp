#include <QObject>
#include <QMessageBox>

#include "Tournament.h"
#include "cmdUnregisterPlayer.h"

cmdUnregisterPlayer::cmdUnregisterPlayer(QWidget* p, const Player& _pl)
  :AbstractCommand(p), pl(_pl)
{

}

//----------------------------------------------------------------------------

ERR cmdUnregisterPlayer::exec()
{
  // set the "wait for registration"-flag
  ERR err;
  auto pm = Tournament::getPlayerMngr();
  err = pm->setWaitForRegistration(pl, true);

  if (err == OK) return OK; // no error

  QString msg = tr("The player is already assigned to matches\n");
  msg += tr("and/or currently running categories.\n\n");
  msg += tr("Can't undo the player registration.");
  QMessageBox::warning(parentWidget, tr("Player unregister failed"), msg);

  return err;
}

