/* 
 * File:   MainFrame.h
 * Author: volker
 *
 * Created on February 16, 2014, 5:16 PM
 */

#ifndef _MAINFRAME_H
#define	_MAINFRAME_H

#include <QShortcut>

#include "ui_MainFrame.h"
#include "Tournament.h"

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
  
  Tournament* tnmt;
  
  QString testFileName;
  
  void closeCurrentTournament();
  
  static MainFrame* mainFramePointer;

  QShortcut* scToggleTestMenuVisibility;
  bool isTestMenuVisible;
  

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

private slots:
  void onToggleTestMenuVisibility();

signals:
  void tournamentOpened (Tournament* tnmt);
  void tournamentClosed ();
};

#endif	/* _MAINFRAME_H */
