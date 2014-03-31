/* 
 * File:   CatTabWidget.h
 * Author: volker
 *
 * Created on March 24, 2014, 7:13 PM
 */

#ifndef _CATTABWIDGET_H
#define	_CATTABWIDGET_H

#include "ui_CatTabWidget.h"

class CatTabWidget : public QDialog
{
  Q_OBJECT
public:
  CatTabWidget();
  virtual ~CatTabWidget();
private:
  Ui::CatTabWidget ui;
  void updateControls();
  void updatePairs();
  int unpairedPlayerId1;
  int unpairedPlayerId2;

public slots:
  void onCatModelChanged();
  void onTabSelectionChanged(const QItemSelection &, const QItemSelection &);
  void onCbDrawChanged(bool newState);
  void onDrawScoreChanged(int newVal);
  void onWinScoreChanged(int newVal);
  void onUnpairedPlayersSelectionChanged();
  void onBtnPairClicked();
  void onPairedPlayersSelectionChanged();
  void onBtnSplitClicked();
} ;

#endif	/* _CATTABWIDGET_H */
