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

#ifndef DLGPICKPLAYERSEX_H
#define DLGPICKPLAYERSEX_H

#include <QDialog>

#include "TournamentDataDefs.h"

namespace Ui {
  class DlgPickPlayerSex;
}

using namespace QTournament;

class DlgPickPlayerSex : public QDialog
{
  Q_OBJECT

public:
  explicit DlgPickPlayerSex(QWidget *parent, const QString& playerName);
  ~DlgPickPlayerSex();
  SEX getSelectedSex();

private slots:
  void onBtnMaleClicked();
  void onBtnFemaleClicked();

private:
  Ui::DlgPickPlayerSex *ui;
  SEX selectedSex = M;

};

#endif // DLGPICKPLAYERSEX_H
