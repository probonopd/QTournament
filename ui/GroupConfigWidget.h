/* 
 * File:   GroupConfigWidget.h
 * Author: volker
 *
 * Created on August 16, 2014, 5:42 PM
 */

#ifndef _GROUPCONFIGWIDGET_H
#define	_GROUPCONFIGWIDGET_H

#include "ui_GroupConfigWidget.h"

class GroupConfigWidget : public QWidget
{
  Q_OBJECT
public:
  GroupConfigWidget();
  GroupConfigWidget(QWidget* parent);
  virtual ~GroupConfigWidget();
private:
  Ui::GroupConfigWidget widget;
} ;

#endif	/* _GROUPCONFIGWIDGET_H */
