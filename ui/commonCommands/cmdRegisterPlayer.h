#ifndef CMDREGISTERPLAYER_H
#define CMDREGISTERPLAYER_H

#include <QObject>

#include "AbstractCommand.h"
#include "Player.h"

using namespace QTournament;

class cmdRegisterPlayer : public QObject, AbstractCommand
{
  Q_OBJECT

public:
  cmdRegisterPlayer(QWidget* p, const Player& _pl);
  virtual ERR exec() override;
  virtual ~cmdRegisterPlayer() {}

protected:
  Player pl;
};

#endif // CMDREGISTERPLAYER_H
