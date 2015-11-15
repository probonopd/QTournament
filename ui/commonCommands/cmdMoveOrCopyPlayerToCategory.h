#ifndef CMDMOVEORCOPYPLAYERTOCAT_H
#define CMDMOVEORCOPYPLAYERTOCAT_H

#include <QObject>

#include "AbstractCommand.h"
#include "Category.h"
#include "Player.h"

using namespace QTournament;

class cmdMoveOrCopyPlayerToCategory : public AbstractCommand, public QObject
{
public:
  cmdMoveOrCopyPlayerToCategory(QWidget* p, const Player& _pl, const Category& _srcCat, const Category& _dstCat, bool _isMove=false);
  virtual ERR exec() override;
  virtual ~cmdMoveOrCopyPlayerToCategory() {}

protected:
  Player pl;
  Category srcCat;
  Category dstCat;
  bool isMove;
};

#endif // CMDREGISTERPLAYER_H
