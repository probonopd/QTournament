/*
 *    This is QTournament, a badminton tournament management program.
 *    Copyright (C) 2014 - 2017  Volker Knollmann
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <iostream>

#include <QApplication>
#include <QTranslator>
#include <QLibraryInfo>

#include <QDebug>
#include <QFile>
#include <QLocale>
#include <QStyleFactory>

#include "ui/MainFrame.h"

int main(int argc, char *argv[])
{
  // initialize resources, if needed
  Q_INIT_RESOURCE(tournament);

  QApplication app(argc, argv);

  // use the "Fusion" style
  QStyle* fusionStyle = QStyleFactory::create("fusion");
  if (fusionStyle == nullptr)
  {
    fusionStyle = QStyleFactory::create("Fusion");
  }
  if (fusionStyle != nullptr)
  {
    app.setStyle(fusionStyle);
  }
  
  cout << "System locale: " << QLocale().name().toStdString() << endl;
  QTranslator qtTranslator;
  // Only temporary: hard-coded German translation while in debug mode
  //qtTranslator.load("qt_de", QLibraryInfo::location(QLibraryInfo::TranslationsPath));

  // try to load the basic Qt translations either from the Qt installation (system-wide)
  // or from a copy deployed along with the application (on non-Qt computers)
  QString appPath = app.applicationDirPath();
  QString transPath = QLibraryInfo::location(QLibraryInfo::TranslationsPath);
  bool isLoaded = qtTranslator.load(QLocale(), "qt", "_", appPath, ".qm");
  if (!isLoaded)
  {
    isLoaded = qtTranslator.load(QLocale(), "qt", "_", transPath, ".qm");
  }
  if (isLoaded) app.installTranslator(&qtTranslator);
  
  // load the application specific translations
  QTranslator tournamentTranslator;
  tournamentTranslator.load(QLocale(), "tournament", "_", appPath, ".qm");
  //
  // Only temporary: hard-coded German translation while in debug mode
  /*
#ifdef __IS_WINDOWS_BUILD
  tournamentTranslator.load(app.applicationDirPath() + "/../../QTournament/tournament_de");
#else
  tournamentTranslator.load(app.applicationDirPath() + "/../tournament_de");
#endif*/
  app.installTranslator(&tournamentTranslator);
  
  MainFrame w;
  w.show();

  // create and show your widgets here

  return app.exec();
}
