#ifndef CMDUNREGISTERPLAYER_H
#define CMDUNREGISTERPLAYER_H

#include <QObject>

#include "AbstractCommand.h"
#include "Player.h"

using namespace QTournament;

class cmdUnregisterPlayer : public AbstractCommand, public QObject
{
  //Q_OBJECT    // we need to inherit from QObject to have tr() available

public:
  cmdUnregisterPlayer(QWidget* p, const Player& _pl);
  virtual ERR exec() override;
  virtual ~cmdUnregisterPlayer() {}

protected:
  Player pl;
};

#endif // CMDREGISTERPLAYER_H
