#ifndef ABSTRACT_COMMAND__H
#define ABSTRACT_COMMAND__H

#include <QWidget>

#include "Tournament.h"
#include "TournamentErrorCodes.h"

using namespace QTournament;

class AbstractCommand
{

public:
  AbstractCommand(QWidget* _parent = nullptr);
  virtual ERR exec() = 0;
  virtual ~AbstractCommand() {}

protected:
  QWidget* parentWidget;
};

#endif
