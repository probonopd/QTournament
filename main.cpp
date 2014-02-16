/*
 * File:   main.cpp
 * Author: volker
 *
 * Created on February 16, 2014, 5:14 PM
 */

#include <QApplication>

#include "ui/MainFrame.h"

int main(int argc, char *argv[])
{
  // initialize resources, if needed
  // Q_INIT_RESOURCE(resfile);

  QApplication app(argc, argv);
  
  MainFrame w;
  w.show();

  // create and show your widgets here

  return app.exec();
}
