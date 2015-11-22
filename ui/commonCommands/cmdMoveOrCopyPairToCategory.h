#ifndef CMDMOVEORCOPYPAIRTOCAT_H
#define CMDMOVEORCOPYPAIRTOCAT_H

#include <QObject>

#include "AbstractCommand.h"
#include "Category.h"
#include "PlayerPair.h"

using namespace QTournament;

class cmdMoveOrCopyPairToCategory : public AbstractCommand, public QObject
{
public:
  cmdMoveOrCopyPairToCategory(QWidget* p, const PlayerPair& _pp, const Category& _srcCat, const Category& _dstCat, bool _isMove=false);
  virtual ERR exec() override;
  virtual ~cmdMoveOrCopyPairToCategory() {}

protected:
  PlayerPair pp;
  Category srcCat;
  Category dstCat;
  bool isMove;
};

#endif // CMDREGISTERPLAYER_H
