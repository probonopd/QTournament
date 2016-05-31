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

#ifndef TEAMLISTVIEW_H
#define	TEAMLISTVIEW_H

#include <QTableView>
#include <QStringListModel>

#include "TournamentDB.h"
#include "models/TeamTableModel.h"
#include "ui/delegates/TeamItemDelegate.h"


using namespace QTournament;

class TeamTableView : public QTableView
{
  Q_OBJECT
  
public:
  TeamTableView (QWidget* parent);
  virtual ~TeamTableView();
  void setDatabase(TournamentDB* _db);
  
private:
  TournamentDB* db;
  QStringListModel* emptyModel;
  TeamTableModel* curDataModel;
  unique_ptr<TeamItemDelegate> teamItemDelegate;
  QAbstractItemDelegate* defaultDelegate;

};

#endif	/* TEAMLISTVIEW_H */

