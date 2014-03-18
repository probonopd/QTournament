/* 
 * File:   MainFrame.h
 * Author: volker
 *
 * Created on February 16, 2014, 5:16 PM
 */

#ifndef _MAINFRAME_H
#define	_MAINFRAME_H

#include "ui_MainFrame.h"
#include "Tournament.h"

using namespace QTournament;

class MainFrame : public QMainWindow
{
  Q_OBJECT
public:
  MainFrame ();
  virtual ~MainFrame ();
  
private:
  Ui::MainFrame ui;
  
  void enableControls(bool doEnable = true);
  void setupTestScenario(int scenarioID);
  
  Tournament* tnmt;
  
  QString testFileName;
  
  void closeCurrentTournament();
  

public slots:
  void newTournament ();
  void openTournament();
  void setupEmptyScenario();
  void setupScenario01();
  void onCreateTeamClicked();
  //void onTeamListDoubleClick();

signals:
  void tournamentOpened (Tournament* tnmt);
};

#endif	/* _MAINFRAME_H */
