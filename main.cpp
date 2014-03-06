/*
 * File:   main.cpp
 * Author: volker
 *
 * Created on February 16, 2014, 5:14 PM
 */

#include <QApplication>
#include <QTranslator>
#include <QLibraryInfo>
//#include <qt4/QtCore/qtranslator.h>

#include "ui/MainFrame.h"

int main(int argc, char *argv[])
{
  // initialize resources, if needed
  // Q_INIT_RESOURCE(resfile);

  QApplication app(argc, argv);
  
  QTranslator qtTranslator;
  qtTranslator.load("qt_" + QLocale::system().name(),
                    QLibraryInfo::location(QLibraryInfo::TranslationsPath));
  app.installTranslator(&qtTranslator);
  
  QTranslator tournamentTranslator;
  tournamentTranslator.load("tournament_" + QLocale::system().name());
  app.installTranslator(&tournamentTranslator);
  
  MainFrame w;
  w.show();

  // create and show your widgets here

  return app.exec();
}
