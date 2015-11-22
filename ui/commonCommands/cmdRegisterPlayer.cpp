#include <QObject>
#include <QMessageBox>

#include "Tournament.h"
#include "cmdRegisterPlayer.h"

cmdRegisterPlayer::cmdRegisterPlayer(QWidget* p, const Player& _pl)
  :AbstractCommand(p), pl(_pl)
{

}

//----------------------------------------------------------------------------

ERR cmdRegisterPlayer::exec()
{
  ERR err;
  auto pm = Tournament::getPlayerMngr();
  err = pm->setWaitForRegistration(pl, false);

  if (err != OK)   // this shouldn't happen
  {
    QString msg = tr("Something went wrong during player registration. This shouldn't happen.\n\n");
    msg += tr("For the records: error code = ") + QString::number(static_cast<int> (err));
    QMessageBox::warning(parentWidget, tr("WTF??"), msg);
  }

  return err;
}

