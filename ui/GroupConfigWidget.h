/* 
 * File:   GroupConfigWidget.h
 * Author: volker
 *
 * Created on August 16, 2014, 5:42 PM
 */

#ifndef _GROUPCONFIGWIDGET_H
#define	_GROUPCONFIGWIDGET_H

#include "ui_GroupConfigWidget.h"

#include "KO_Config.h"

using namespace QTournament;

class GroupConfigWidget : public QWidget
{
  Q_OBJECT
public:
  GroupConfigWidget();
  GroupConfigWidget(QWidget* parent);
  virtual ~GroupConfigWidget();
  KO_Config getConfig();
  void applyConfig(const KO_Config& cfg);
  void setRequiredPlayersCount(int cnt);

signals:
  void groupConfigChanged(const KO_Config& newCfg);
  
public slots:
  void onStartLevelChanged(int newIndex);
  void onSecondSurvivesChanged();
  void onSpinBoxGroupCount1Changed(int newVal);
  void onSpinBoxGroupCount2Changed(int newVal);
  void onSpinBoxGroupCount3Changed(int newVal);
  void onSpinBoxGroupSize1Changed(int newVal);
  void onSpinBoxGroupSize2Changed(int newVal);
  void onSpinBoxGroupSize3Changed(int newVal);
  
private:
  Ui::GroupConfigWidget ui;
  
  void applyDefaultConfig();
  void updateLabels();
  
  int oldGroupSize1;
  int oldGroupSize2;
  int oldGroupSize3;
  int reqPlayers;
} ;

#endif	/* _GROUPCONFIGWIDGET_H */
