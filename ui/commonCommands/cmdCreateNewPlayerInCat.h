#ifndef CMDCREATENEWPLAYERINCAT_H
#define CMDCREATENEWPLAYERINCAT_H

#include <QObject>

#include "AbstractCommand.h"
#include "Category.h"

using namespace QTournament;

class cmdCreateNewPlayerInCat : public QObject, AbstractCommand
{
  Q_OBJECT

public:
  cmdCreateNewPlayerInCat(QWidget* p, const Category& _cat);
  virtual ERR exec() override;
  virtual ~cmdCreateNewPlayerInCat() {}

protected:
  Category cat;
};

#endif // CMDREGISTERPLAYER_H
