#ifndef CMDIMPORTSINGLEPLAYERFROMEXTERNALDATABASE_H
#define CMDIMPORTSINGLEPLAYERFROMEXTERNALDATABASE_H

#include <QObject>

#include "AbstractCommand.h"
#include "Category.h"

using namespace QTournament;

class cmdImportSinglePlayerFromExternalDatabase : public AbstractCommand, public QObject
{
public:
  cmdImportSinglePlayerFromExternalDatabase(QWidget* p, int _preselectedCatId=-1);
  virtual ERR exec() override;
  virtual ~cmdImportSinglePlayerFromExternalDatabase() {}

protected:
  int preselectedCatId;
};

#endif // CMDIMPORTSINGLEPLAYERFROMEXTERNALDATABASE_H
