#ifndef CMDCREATEPLAYERFROMDIALOG_H
#define CMDCREATEPLAYERFROMDIALOG_H

#include <QObject>

#include "AbstractCommand.h"
#include "Player.h"
#include "../dlgEditPlayer.h"

using namespace QTournament;

class cmdCreatePlayerFromDialog : public QObject, AbstractCommand
{
  Q_OBJECT

public:
  cmdCreatePlayerFromDialog(QWidget* p, DlgEditPlayer* initializedDialog);
  virtual ERR exec() override;
  virtual ~cmdCreatePlayerFromDialog() {}

protected:
  DlgEditPlayer* dlg;
};

#endif // CMDCREATEPLAYERFROMDIALOG_H
