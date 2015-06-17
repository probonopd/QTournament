/*
 * File:   main.cpp
 * Author: volker
 *
 * Created on February 16, 2014, 5:14 PM
 */

#include <QApplication>
#include <QTranslator>
#include <QLibraryInfo>

#include <QDebug>
#include <QFile>

#include "ui/MainFrame.h"

int main(int argc, char *argv[])
{
  // initialize resources, if needed
  Q_INIT_RESOURCE(tournament);

  QApplication app(argc, argv);
  
  QTranslator qtTranslator;
  qDebug() << qtTranslator.load("qt_" + QLocale::system().name(),
                    QLibraryInfo::location(QLibraryInfo::TranslationsPath));
  app.installTranslator(&qtTranslator);
  
  QTranslator tournamentTranslator;
  qDebug() << QLocale::system().name();
  qDebug() << app.applicationDirPath();
  qDebug() << tournamentTranslator.load(app.applicationDirPath() + "/tournament_" + QLocale::system().name());
  app.installTranslator(&tournamentTranslator);
  
  MainFrame w;
  w.show();

  // create and show your widgets here

  return app.exec();
}
