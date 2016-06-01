/*
 *    This is QTournament, a badminton tournament management program.
 *    Copyright (C) 2014 - 2016  Volker Knollmann
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

  QApplication::setDesktopSettingsAware(false);

  QApplication app(argc, argv);
  
  QTranslator qtTranslator;
  // Only temporary: hard-coded German translation while in debug mode
  //qtTranslator.load("qt_de", QLibraryInfo::location(QLibraryInfo::TranslationsPath));
  qtTranslator.load("qt_" + QLocale::system().name(),
                    QLibraryInfo::location(QLibraryInfo::TranslationsPath));
  app.installTranslator(&qtTranslator);
  
  QTranslator tournamentTranslator;
  tournamentTranslator.load(app.applicationDirPath() + "/tournament_" + QLocale::system().name());
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
