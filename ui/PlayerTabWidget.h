/* 
 * File:   PlayerTabWidget.h
 * Author: volker
 *
 * Created on March 19, 2014, 7:38 PM
 */

#ifndef _PLAYERTABWIDGET_H
#define	_PLAYERTABWIDGET_H

#include "ui_PlayerTabWidget.h"

#include <QObject>
#include <QMenu>
#include <QAction>

class PlayerTabWidget : public QWidget
{
  Q_OBJECT
public:
  PlayerTabWidget();
  virtual ~PlayerTabWidget ();
  
private:
  Ui::PlayerTabWidget ui;

  unique_ptr<QMenu> registrationMenu;
  QAction* actRegisterAll;
  QAction* actUnregisterAll;

  void initRegistrationMenu();

public slots:
  void onCreatePlayerClicked();
  void onPlayerDoubleClicked(const QModelIndex& index);
  void onPlayerCountChanged();
  void onTournamentOpened();
  void onTournamentClosed();
  void onRegisterAllTriggered();
  void onUnregisterAllTriggered();
};

#endif	/* _PLAYERTABWIDGET_H */
