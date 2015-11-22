#ifndef CMDREMOVE_PLAYER_FROM_CAT__H
#define CMDREMOVE_PLAYER_FROM_CAT__H

#include <QObject>

#include "AbstractCommand.h"
#include "Player.h"
#include "Category.h"

using namespace QTournament;

class cmdRemovePlayerFromCategory : public AbstractCommand, public QObject
{
  //Q_OBJECT    // we need to inherit from QObject to have tr() available

public:
  cmdRemovePlayerFromCategory(QWidget* p, const Player& _pl, const Category& _cat);
  virtual ERR exec() override;
  virtual ~cmdRemovePlayerFromCategory() {}

protected:
  Player pl;
  Category cat;
};

#endif // CMDREGISTERPLAYER_H
