#ifndef DLGSEEDINGEDITOR_H
#define DLGSEEDINGEDITOR_H

#include <QDialog>
#include <QTimer>

#include "Tournament.h"
#include "PlayerPair.h"

namespace Ui {
  class DlgSeedingEditor;
}

class DlgSeedingEditor : public QDialog
{
  Q_OBJECT

public:
  explicit DlgSeedingEditor(QWidget *parent = 0);
  ~DlgSeedingEditor();
  void initSeedingList(const PlayerPairList& _seed);

public slots:
  void onBtnUpClicked();
  void onBtnDownClicked();
  void onBtnShuffleClicked();
  void onShuffleModeChange();
  void onSelectionChanged();

private slots:
  void onKeypressTimerElapsed();

private:
  static constexpr int SUBSEQUENT_KEYPRESS_TIMEOUT__MS = 1000;
  Ui::DlgSeedingEditor *ui;
  void updateButtons();
  bool eventFilter(QObject *target, QEvent *event);
  int positionInput;
  QTimer* keypressTimer;
};

#endif // DLGSEEDINGEDITOR_H
