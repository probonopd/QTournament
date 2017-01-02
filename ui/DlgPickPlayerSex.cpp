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

#include "DlgPickPlayerSex.h"
#include "ui_DlgPickPlayerSex.h"

DlgPickPlayerSex::DlgPickPlayerSex(QWidget *parent, const QString& playerName) :
  QDialog(parent),
  ui(new Ui::DlgPickPlayerSex)
{
  ui->setupUi(this);

  if (playerName.isEmpty())
  {
    ui->lblHeadline->setVisible(false);
    ui->lblName->setVisible(false);
  } else {
    ui->lblName->setText(playerName);
  }
}

DlgPickPlayerSex::~DlgPickPlayerSex()
{
  delete ui;
}

SEX DlgPickPlayerSex::getSelectedSex()
{
  return selectedSex;
}

void DlgPickPlayerSex::onBtnMaleClicked()
{
  selectedSex = M;
  accept();
}

void DlgPickPlayerSex::onBtnFemaleClicked()
{
  selectedSex = F;
  accept();
}

