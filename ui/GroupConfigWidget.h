/*
 *    This is QTournament, a badminton tournament management program.
 *    Copyright (C) 2014 - 2017  Volker Knollmann
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _GROUPCONFIGWIDGET_H
#define	_GROUPCONFIGWIDGET_H

#include "ui_GroupConfigWidget.h"

#include "KO_Config.h"
#include "TournamentDB.h"

using namespace QTournament;

class GroupConfigWidget : public QWidget
{
  Q_OBJECT
public:
  GroupConfigWidget(QWidget* parent = nullptr);
  virtual ~GroupConfigWidget();
  KO_Config getConfig();
  void applyConfig(const KO_Config& cfg);
  void setRequiredPlayersCount(int cnt);
  void setDatabase(TournamentDB* _db);

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
  TournamentDB* db;
  Ui::GroupConfigWidget ui;
  void onSpinBoxGroupCountChanged(int spinBoxIndex, int newVal);
  void onSpinBoxGroupSizeChanged(int spinBoxIndex, int newVal);
  int getSpinBoxIndexForGroupSize(int grpSize);
  int getNextUnusedSpinBoxIndex();
  
  void applyDefaultConfig();
  void updateLabels();
  
  int oldGroupSize[3];
  int reqPlayers;
  bool rangeControlEnabled;
  bool pointersInitialized;
  
  QSpinBox* spGroupSize[3];
  QSpinBox* spGroupCount[3];
} ;

#endif	/* _GROUPCONFIGWIDGET_H */
