/* 
 * File:   PlayerTabWidget.h
 * Author: volker
 *
 * Created on March 19, 2014, 6:20 PM
 */

#ifndef _TEAMTABWIDGET_H
#define	_TEAMTABWIDGET_H

#include "ui_TeamTabWidget.h"

#include <QObject>


class TeamTabWidget : public QWidget
{
  Q_OBJECT
public:
  TeamTabWidget ();
  virtual ~TeamTabWidget ();
private:
  Ui::TeamTabWidget ui;
  
public slots:
  void onCreateTeamClicked();
};

#endif	/* _TEAMTABWIDGET_H */
