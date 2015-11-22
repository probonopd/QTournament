#ifndef CMDIMPORTSINGLEPLAYERFROMEXTERNALDATABASE_H
#define CMDIMPORTSINGLEPLAYERFROMEXTERNALDATABASE_H

#include <QObject>

#include "AbstractCommand.h"
#include "Category.h"

using namespace QTournament;

class cmdImportSinglePlayerFromExternalDatabase : public AbstractCommand, public QObject
{
public:
  cmdImportSinglePlayerFromExternalDatabase(QWidget* p);
  virtual ERR exec() override;
  virtual ~cmdImportSinglePlayerFromExternalDatabase() {}

protected:
};

#endif // CMDIMPORTSINGLEPLAYERFROMEXTERNALDATABASE_H
