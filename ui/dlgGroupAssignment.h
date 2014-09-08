/* 
 * File:   dlgGroupAssignment.h
 * Author: volker
 *
 * Created on September 7, 2014, 6:28 PM
 */

#ifndef _DLGGROUPASSIGNMENT_H
#define	_DLGGROUPASSIGNMENT_H

#include "ui_dlgGroupAssignment.h"

#include <QList>
#include "PlayerPair.h"

class dlgGroupAssignment : public QDialog
{
  Q_OBJECT
public:
  dlgGroupAssignment(Category* _cat);
  virtual ~dlgGroupAssignment();
private:
  Ui::dlgGroupAssignment ui;
  Category* cat;
  KO_Config cfg;
  
  QList<PlayerPairList> getRandomizedPlayerPairListList();
  
public slots:
  virtual void done (int result);

private slots:
  void onBtnRandomizeClicked();
  
} ;

#endif	/* _DLGGROUPASSIGNMENT_H */
