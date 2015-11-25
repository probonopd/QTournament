#ifndef CMDREMOVE_PLAYER_FROM_CAT__H
#define CMDREMOVE_PLAYER_FROM_CAT__H

#include <QObject>

#include "AbstractCommand.h"
#include "Player.h"
#include "Category.h"

using namespace QTournament;

class cmdRemovePlayerFromCategory : public QObject, AbstractCommand
{
  Q_OBJECT

public:
  cmdRemovePlayerFromCategory(QWidget* p, const Player& _pl, const Category& _cat);
  virtual ERR exec() override;
  virtual ~cmdRemovePlayerFromCategory() {}

protected:
  Player pl;
  Category cat;
};

#endif // CMDREGISTERPLAYER_H
