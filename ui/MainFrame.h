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

#ifndef MAINFRAME_H
#define	MAINFRAME_H

#include <memory>

#include <QShortcut>
#include <QCloseEvent>
#include <QTimer>

#include "ui_MainFrame.h"

#define PRG_VERSION_STRING "0.6.0"

class MainFrame : public QMainWindow
{
  Q_OBJECT
public:
  MainFrame ();
  virtual ~MainFrame () override;
  
protected:
  virtual void closeEvent(QCloseEvent *ev) override;

private:
  Ui::MainFrame ui;
  std::unique_ptr<QTournament::TournamentDB> currentDb{nullptr};
  QString testFileName;
  QString currentDatabaseFileName;

  // the test menu
  QShortcut* scToggleTestMenuVisibility;
  bool isTestMenuVisible;

  // timers for polling the database's dirty flag
  // and triggering the autosave function
  static constexpr int DirtyFlagPollIntervall_ms = 1000;
  static constexpr int AutosaveIntervall_ms = 120 * 1000;
  std::unique_ptr<QTimer> dirtyFlagPollTimer;
  std::unique_ptr<QTimer> autosaveTimer;

  // a label for the status bar that shows the last autosave
  QLabel* lastAutosaveTimeStatusLabel;

  // a timer and label for server syncs
  static constexpr int ServerSyncStatusInterval_ms = 1000;
  std::unique_ptr<QTimer> serverSyncTimer;
  QLabel* syncStatLabel;
  QPushButton* btnPingTest;

  void enableControls(bool doEnable = true);
  void setupTestScenario(int scenarioID);
  
  bool closeCurrentTournament();
  

  void distributeCurrentDatabasePointerToWidgets(bool forceNullptr = false);
  bool saveCurrentDatabaseToFile(const QString& dstFileName);
  bool execCmdSave();
  bool execCmdSaveAs();
  QString askForTournamentFileName(const QString& dlgTitle);

  void updateWindowTitle();

  void updateOnlineMenu();


public slots:
  void newTournament();
  void openTournament();
  void onSave();
  void onSaveAs();
  void onSaveCopy();
  void onCreateBaseline();
  void onClose();
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
  void onEditTournamentSettings();
  void onSetPassword();
  void onRegisterTournament();
  void onStartSession();
  void onTerminateSession();
  void onDeleteFromServer();
  void onEditConnectionSettings();

private slots:
  void onToggleTestMenuVisibility();
  void onDirtyFlagPollTimerElapsed();
  void onAutosaveTimerElapsed();
  void onServerSyncTimerElapsed();
  void onBtnPingTestClicked();

};

#endif	/* _MAINFRAME_H */
