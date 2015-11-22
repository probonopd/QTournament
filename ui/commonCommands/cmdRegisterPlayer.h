#ifndef CMDREGISTERPLAYER_H
#define CMDREGISTERPLAYER_H

#include <QObject>

#include "AbstractCommand.h"
#include "Player.h"

using namespace QTournament;

class cmdRegisterPlayer : public AbstractCommand, public QObject
{
  //Q_OBJECT    // we need to inherit from QObject to have tr() available

public:
  cmdRegisterPlayer(QWidget* p, const Player& _pl);
  virtual ERR exec() override;
  virtual ~cmdRegisterPlayer() {}

protected:
  Player pl;
};

#endif // CMDREGISTERPLAYER_H
