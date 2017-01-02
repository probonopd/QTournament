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

#ifndef _TEAMTABWIDGET_H
#define	_TEAMTABWIDGET_H

#include "ui_TeamTabWidget.h"

#include <QObject>

#include "TournamentDB.h"

class TeamTabWidget : public QWidget
{
  Q_OBJECT
public:
  TeamTabWidget ();
  virtual ~TeamTabWidget ();
  void setDatabase(TournamentDB* _db);

private:
  TournamentDB* db;
  Ui::TeamTabWidget ui;
  
public slots:
  void onCreateTeamClicked();
};

#endif	/* _TEAMTABWIDGET_H */
