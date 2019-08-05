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

#ifndef _DLGGROUPASSIGNMENT_H
#define	_DLGGROUPASSIGNMENT_H

#include "ui_dlgGroupAssignment.h"

#include <QList>

#include "PlayerPair.h"
#include "TournamentDB.h"
#include "KO_Config.h"

class dlgGroupAssignment : public QDialog
{
  Q_OBJECT
public:
  dlgGroupAssignment(const QTournament::TournamentDB& _db, QWidget* p, QTournament::Category& _cat);
  virtual ~dlgGroupAssignment();
  std::vector<QTournament::PlayerPairList> getGroupAssignments();

private:
  Ui::dlgGroupAssignment ui;
  std::reference_wrapper<const QTournament::TournamentDB> db;
  QTournament::Category& cat;
  QTournament::KO_Config cfg;
  
  QList<QTournament::PlayerPairList> getRandomizedPlayerPairListList();
  
public slots:
  virtual void done (int result);

private slots:
  void onBtnRandomizeClicked();
  void onBtnSwapClicked();
  
} ;

#endif	/* _DLGGROUPASSIGNMENT_H */
