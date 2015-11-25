#ifndef CMDUNREGISTERPLAYER_H
#define CMDUNREGISTERPLAYER_H

#include <QObject>

#include "AbstractCommand.h"
#include "Player.h"

using namespace QTournament;

class cmdUnregisterPlayer : public QObject, AbstractCommand
{
  Q_OBJECT

public:
  cmdUnregisterPlayer(QWidget* p, const Player& _pl);
  virtual ERR exec() override;
  virtual ~cmdUnregisterPlayer() {}

protected:
  Player pl;
};

#endif // CMDREGISTERPLAYER_H
