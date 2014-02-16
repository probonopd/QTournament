/* 
 * File:   MainFrame.h
 * Author: volker
 *
 * Created on February 16, 2014, 5:16 PM
 */

#ifndef _MAINFRAME_H
#define	_MAINFRAME_H

#include "ui_MainFrame.h"

class MainFrame : public QMainWindow
{
  Q_OBJECT
public:
  MainFrame ();
  virtual ~MainFrame ();
private:
  Ui::MainFrame widget;
};

#endif	/* _MAINFRAME_H */
