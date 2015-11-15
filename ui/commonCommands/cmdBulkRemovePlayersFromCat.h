#ifndef CMDBULKREMOVEPLAYERSFROMCAT_H
#define CMDBULKREMOVEPLAYERSFROMCAT_H

#include <QObject>

#include "AbstractCommand.h"
#include "Category.h"

using namespace QTournament;

class cmdBulkRemovePlayersFromCategory : public AbstractCommand, public QObject
{
public:
  cmdBulkRemovePlayersFromCategory(QWidget* p, const Category& _cat);
  virtual ERR exec() override;
  virtual ~cmdBulkRemovePlayersFromCategory() {}

protected:
  Category cat;
};

#endif // CMDREGISTERPLAYER_H
