#ifndef SEEDINGLISTWIDGET_H
#define SEEDINGLISTWIDGET_H

#include <QObject>
#include <QWidget>
#include <QListWidget>
#include <QQueue>

#include "Tournament.h"
#include "PlayerPair.h"
#include "delegates/PairItemDelegate.h"

class SeedingListWidget : public QListWidget
{
  Q_OBJECT

public:
  SeedingListWidget(QWidget* parent);
  void initSeedingList(const PlayerPairList& _seed);
  ~SeedingListWidget();
  int getSelectedItemIndex() const;
  bool canMoveSelectedPlayerUp() const;
  bool canMoveSelectedPlayerDown() const;
  void shufflePlayers(int fromIndex = -1);
  void moveSelectedPlayerUp();
  void moveSelectedPlayerDown();
  void warpSelectedPlayerTo(int targetRow);
  PlayerPairList getSeedList() const;
  void clearListAndFillFromSeed(const PlayerPairList& seed);

private:
  PairItemDelegate* pairDelegate;
  QQueue<QListWidgetItem*> selectionQueue;
  void swapListItems(int row1, int row2);
};

#endif // SEEDINGLISTWIDGET_H
