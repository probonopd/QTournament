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

#include "GroupConfigWidget.h"

#include <stdexcept>
#include <QMessageBox>

using namespace QTournament;

GroupConfigWidget::~GroupConfigWidget()
{
}

//----------------------------------------------------------------------------
    
GroupConfigWidget::GroupConfigWidget(QWidget* parent)
    : QWidget(parent)
{
  ui.setupUi(this);
  
  // copy spin box pointers to an array for easier access
  spGroupSize[0] = ui.spGroupSize1;
  spGroupSize[1] = ui.spGroupSize2;
  spGroupSize[2] = ui.spGroupSize3;
  spGroupCount[0] = ui.spGroupCount1;
  spGroupCount[1] = ui.spGroupCount2;
  spGroupCount[2] = ui.spGroupCount3;
  pointersInitialized = true;
  
  // apply min / max values to the group size spin boxes and
  // set min values for the group count spin boxes
  for (int i=0; i<3; i++)
  {
    spGroupSize[i]->setMinimum(3);
    spGroupSize[i]->setMaximum(MaxGroupSize);
    spGroupSize[i]->setValue(i+3);
    oldGroupSize[i] = i+3;
    
    spGroupCount[i]->setMinimum(0);
    spGroupCount[i]->setMaximum(MaxGroupCount);
  }
    
  rangeControlEnabled = true;
  
  applyDefaultConfig();
}

//----------------------------------------------------------------------------
    
void GroupConfigWidget::applyDefaultConfig()
{
  KO_Config cfg = KO_Config(KO_Start::Quarter, false);
  applyConfig(cfg);
}

//----------------------------------------------------------------------------
    
void GroupConfigWidget::applyConfig(const KO_Config& cfg)
{
  // Set the combobox with the start level
  KO_Start lvl = cfg.getStartLevel();
  if (lvl == KO_Start::Final)
  {
    ui.cbKOStart->setCurrentIndex(0);
  }
  else if (lvl == KO_Start::Semi)
  {
    ui.cbKOStart->setCurrentIndex(1);
  }
  else if (lvl == KO_Start::Quarter)
  {
    ui.cbKOStart->setCurrentIndex(2);
  }
  else if (lvl == KO_Start::L16)
  {
    ui.cbKOStart->setCurrentIndex(3);
  }
  else
  {
    throw std::invalid_argument("Invalid start level in KO_Config!");
  }
  
  // the check box for the "second player survives" choices
  ui.cbSecondSurvives->setChecked(cfg.getSecondSurvives());
  
  // if we proceed directly with the finals, we actually don't have a
  // choice if the group second's should survive or not
  ui.cbSecondSurvives->setEnabled(lvl != KO_Start::Final);
  
  // update the spin boxes for the group count / size values

  rangeControlEnabled = false;   // temporarily disable range control to prevent the range control logic from overwriting values
  for (int i=0; i<3; i++) spGroupCount[i]->setValue(0); // clear all old group counts

  int nGroupDefs = cfg.getNumGroupDefs();
  int iMax = std::min(3, nGroupDefs);

  // loop over all group definitions and set spin boxes accordingly
  for (int i=0; i<iMax; i++)
  {
    GroupDef g = cfg.getGroupDef(i);
    int spinBoxIndex = getSpinBoxIndexForGroupSize(g.getGroupSize());
    if (spinBoxIndex == -1) spinBoxIndex = getNextUnusedSpinBoxIndex();
    if (spinBoxIndex == -1)  // theoretically, this should never occur
    {
      throw std::runtime_error("Invalid group configuration encountered!");
    }
    spGroupCount[spinBoxIndex]->setValue(g.getNumGroups());
    spGroupSize[spinBoxIndex]->setValue(g.getGroupSize());
    oldGroupSize[spinBoxIndex] = g.getGroupSize();
  }
  
  // re-enable range control for the spin boxes
  rangeControlEnabled = true;
  
  updateLabels();
  
}

//----------------------------------------------------------------------------

void GroupConfigWidget::updateLabels()
{
  KO_Config cfg = getConfig();
  
  // update the text label with the actual and required counters
  GroupDefList gdl = cfg.getGroupDefList();
  ui.laReqGroupCount->setText(QString::number(cfg.getNumReqGroups()));
  ui.laReqPlayerCount->setText(QString::number(reqPlayers));
  ui.laTotalGroups->setText(QString::number(gdl.getTotalGroupCount()));
  ui.laTotalPlayers->setText(QString::number(gdl.getTotalPlayerCount()));
  
  // update the validity statement
  QString msg = tr("The configuration is ");
  if (cfg.isValid(reqPlayers))
  {
    msg += tr("VALID");
  } else {
    msg += tr("INVALID");
  }
  ui.laValidity->setText(msg);
}

//----------------------------------------------------------------------------
    
KO_Config GroupConfigWidget::getConfig()
{
  int lvl = ui.cbKOStart->currentIndex();
  KO_Start koLvl = KO_Start::Final;
  if (lvl == 1) koLvl = KO_Start::Semi;
  else if (lvl == 2) koLvl = KO_Start::Quarter;
  else if (lvl == 3) koLvl = KO_Start::L16;
  else if (lvl > 3)
  {
    throw std::invalid_argument("Illegal dropbox index!");
  }
  
  bool secondSurvives = ui.cbSecondSurvives->isChecked();
  
  GroupDefList gdl;
  for (int i=0; i<3; i++)
  {
    if (spGroupCount[i]->value() > 0)
    {
      gdl.append(GroupDef(spGroupSize[i]->value(), spGroupCount[i]->value()));
    }
  }
  
  return KO_Config(koLvl, secondSurvives, gdl);
}

//----------------------------------------------------------------------------

void GroupConfigWidget::onStartLevelChanged(int newIndex)
{
  if (!pointersInitialized) return;
  
  // fake a complete update of the widget, because if we switch to/from
  // KO_Start::FinalS as the start level, we have to activate / deactivate some
  // widgets
  KO_Config cfg = getConfig();
  applyConfig(cfg);
  emit groupConfigChanged(cfg);
}

//----------------------------------------------------------------------------
    
void GroupConfigWidget::onSecondSurvivesChanged()
{
  updateLabels();
  emit groupConfigChanged(getConfig());
}

//----------------------------------------------------------------------------

void GroupConfigWidget::onSpinBoxGroupCount1Changed(int newVal)
{
  onSpinBoxGroupCountChanged(0, newVal);
}

//----------------------------------------------------------------------------

void GroupConfigWidget::onSpinBoxGroupCount2Changed(int newVal)
{
  onSpinBoxGroupCountChanged(1, newVal);
}

//----------------------------------------------------------------------------

void GroupConfigWidget::onSpinBoxGroupCount3Changed(int newVal)
{
  onSpinBoxGroupCountChanged(2, newVal);
}

//----------------------------------------------------------------------------
    
void GroupConfigWidget::onSpinBoxGroupSize1Changed(int newVal)
{
  onSpinBoxGroupSizeChanged(0, newVal);
}

//----------------------------------------------------------------------------
    
void GroupConfigWidget::onSpinBoxGroupSize2Changed(int newVal)
{
  onSpinBoxGroupSizeChanged(1, newVal);
}

//----------------------------------------------------------------------------
    
void GroupConfigWidget::onSpinBoxGroupSize3Changed(int newVal)
{
  onSpinBoxGroupSizeChanged(2, newVal);
}

//----------------------------------------------------------------------------

void GroupConfigWidget::setRequiredPlayersCount(int cnt)
{
  if (cnt < 0) return;
  
  reqPlayers = cnt;
  updateLabels();
}

//----------------------------------------------------------------------------

void GroupConfigWidget::setDatabase(const TournamentDB* _db)
{
  db = _db;

  setEnabled(db != nullptr);
}

//----------------------------------------------------------------------------

void GroupConfigWidget::onSpinBoxGroupCountChanged(int spinBoxIndex, int newVal)
{
  if (!rangeControlEnabled) return;
  
  QSpinBox* sb = spGroupCount[spinBoxIndex];
  
  if (newVal < 0) sb->setValue(0);
  if (newVal > MaxGroupCount) sb->setValue(MaxGroupCount);
  updateLabels();
  emit groupConfigChanged(getConfig());
}

//----------------------------------------------------------------------------

void GroupConfigWidget::onSpinBoxGroupSizeChanged(int spinBoxIndex, int newVal)
{
  if (!rangeControlEnabled) return;
  
  QSpinBox* sb = spGroupSize[spinBoxIndex];
  
  // avoid the same group size in two or more group defs
  bool hasIncreased = (newVal > oldGroupSize[spinBoxIndex]);
  bool conflictFound = true;
  while (conflictFound)
  {
    conflictFound = false;
    for (int i=0; i<3; i++)
    {
      if ((i != spinBoxIndex) && (spGroupSize[i]->value() == newVal)) conflictFound = true;
    }
    
    if (conflictFound)
    {
      if (hasIncreased) newVal++;
      else newVal--;
    }
  }
  
  // avoid too small or too large values
  if ((newVal < 3) || (newVal > MaxGroupSize))
  {
    sb->setValue(oldGroupSize[spinBoxIndex]);
    return;
  }
  
  oldGroupSize[spinBoxIndex] = newVal;
  sb->setValue(newVal);
  
  updateLabels();
  emit groupConfigChanged(getConfig());
}

//----------------------------------------------------------------------------

int GroupConfigWidget::getSpinBoxIndexForGroupSize(int grpSize)
{
  for (int i=0; i<3; i++)
  {
    if (spGroupSize[i]->value() == grpSize) return i;
  }
  
  return -1;
}

//----------------------------------------------------------------------------

int GroupConfigWidget::getNextUnusedSpinBoxIndex()
{
  for (int i=0; i<3; i++)
  {
    if (spGroupCount[i]->value() == 0) return i;
  }
  
  return -1;
}

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    
