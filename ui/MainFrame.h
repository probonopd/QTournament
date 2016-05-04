/*
 *    This is QTournament, a badminton tournament management program.
 *    Copyright (C) 2014 - 2015  Volker Knollmann
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

#ifndef _MAINFRAME_H
#define	_MAINFRAME_H

#include <memory>

#include <QShortcut>

#include "ui_MainFrame.h"

#define PRG_VERSION_STRING "<DevelopmentSnapshot>"

using namespace QTournament;

class MainFrame : public QMainWindow
{
  Q_OBJECT
public:
  MainFrame ();
  virtual ~MainFrame ();
  static MainFrame* getMainFramePointer();
  
private:
  Ui::MainFrame ui;
  
  void enableControls(bool doEnable = true);
  void setupTestScenario(int scenarioID);
  
  unique_ptr<TournamentDB> currentDb;
  
  QString testFileName;
  
  void closeCurrentTournament();
  
  static MainFrame* mainFramePointer;

  QShortcut* scToggleTestMenuVisibility;
  bool isTestMenuVisible;

  void distributeCurrentDatabasePointerToWidgets(bool forceNullptr = false);
  

public slots:
  void newTournament ();
  void openTournament();
  void setupEmptyScenario();
  void setupScenario01();
  void setupScenario02();
  void setupScenario03();
  void setupScenario04();
  void setupScenario05();
  void setupScenario06();
  void setupScenario07();
  void setupScenario08();
  void onCurrentTabChanged(int newCurrentTab);
  void onNewExternalPlayerDatabase();
  void onSelectExternalPlayerDatabase();
  void onInfoMenuTriggered();

private slots:
  void onToggleTestMenuVisibility();

};

#endif	/* _MAINFRAME_H */
