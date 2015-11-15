#ifndef DLGSELECTPLAYER_H
#define DLGSELECTPLAYER_H

#include <QDialog>

#include "Tournament.h"
#include "Player.h"
#include "Category.h"

namespace Ui {
  class DlgSelectPlayer;
}

using namespace QTournament;

class DlgSelectPlayer : public QDialog
{
  Q_OBJECT

public:
  enum class DLG_CONTEXT
  {
    ADD_TO_CATEGORY,
    REMOVE_FROM_CATEGORY,
    NONE
  };

  explicit DlgSelectPlayer(QWidget *parent = 0, DLG_CONTEXT _ctxt = DLG_CONTEXT::NONE, Category* _cat = nullptr);
  ~DlgSelectPlayer();
  PlayerList getSelectedPlayers() const;

private:
  Ui::DlgSelectPlayer *ui;
  DLG_CONTEXT ctxt;
  Category* cat;

private slots:
  void onItemSelectionChanged();
};

#endif // DLGSELECTPLAYER_H
