#ifndef DLGIMPORTPLAYER_H
#define DLGIMPORTPLAYER_H

#include <QDialog>

#include "ExternalPlayerDB.h"

namespace Ui {
  class DlgImportPlayer;
}

using namespace QTournament;

class DlgImportPlayer : public QDialog
{
  Q_OBJECT

public:
  explicit DlgImportPlayer(QWidget *parent = 0, ExternalPlayerDB* _extDb = nullptr);
  ~DlgImportPlayer();

  int getSelectedExternalPlayerId();

public slots:
  virtual int exec() override;

private:
  Ui::DlgImportPlayer *ui;
  ExternalPlayerDB* extDb;

private slots:
  void onNameListSelectionChanged();
  void onSearchStringChanged();
};

#endif // DLGIMPORTPLAYER_H
