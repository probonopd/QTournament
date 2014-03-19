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

class PlayerTabWidget : public QWidget
{
  Q_OBJECT
public:
  PlayerTabWidget(QWidget* parent);
  virtual ~PlayerTabWidget ();
  
private:
  Ui::PlayerTabWidget ui;
  
public slots:
  void onCreatePlayerClicked();
  void onPlayerDoubleClicked(const QModelIndex& index);
};

#endif	/* _PLAYERTABWIDGET_H */
