/*
 * File:   GroupConfigWidget.cpp
 * Author: volker
 *
 * Created on August 16, 2014, 5:42 PM
 */

#include "GroupConfigWidget.h"

#include <stdexcept>
#include <QtWidgets/qmessagebox.h>

GroupConfigWidget::GroupConfigWidget()
{
  ui.setupUi(this);
}

//----------------------------------------------------------------------------
    
GroupConfigWidget::~GroupConfigWidget()
{
}

//----------------------------------------------------------------------------
    
GroupConfigWidget::GroupConfigWidget(QWidget* parent) : QWidget(parent)
{
  ui.setupUi(this);
  
  // apply min / max values to the group size spin boxes
  ui.spGroupSize1->setMinimum(3);
  ui.spGroupSize2->setMinimum(3);
  ui.spGroupSize3->setMinimum(3);
  ui.spGroupSize1->setMaximum(MAX_GROUP_SIZE);
  ui.spGroupSize2->setMaximum(MAX_GROUP_SIZE);
  ui.spGroupSize3->setMaximum(MAX_GROUP_SIZE);
  ui.spGroupSize1->setValue(3);
  ui.spGroupSize2->setValue(4);
  ui.spGroupSize3->setValue(5);
  oldGroupSize1 = 3;
  oldGroupSize2 = 4;
  oldGroupSize3 = 5;
  
  // set min values for the group count spin boxes
  ui.spGroupCount1->setMinimum(0);
  ui.spGroupCount2->setMinimum(0);
  ui.spGroupCount3->setMinimum(0);
  ui.spGroupCount1->setMaximum(MAX_GROUP_COUNT);
  ui.spGroupCount2->setMaximum(MAX_GROUP_COUNT);
  ui.spGroupCount3->setMaximum(MAX_GROUP_COUNT);
  
  applyDefaultConfig();
}

//----------------------------------------------------------------------------
    
void GroupConfigWidget::applyDefaultConfig()
{
  KO_Config cfg = KO_Config(QUARTER, false);
  applyConfig(cfg);
}

//----------------------------------------------------------------------------
    
void GroupConfigWidget::applyConfig(const KO_Config& cfg)
{
  // Set the combobox with the start level
  KO_START lvl = cfg.getStartLevel();
  if (lvl == SEMI)
  {
    ui.cbKOStart->setCurrentIndex(0);
  }
  else if (lvl == QUARTER)
  {
    ui.cbKOStart->setCurrentIndex(1);
  }
  else if (lvl == L16)
  {
    ui.cbKOStart->setCurrentIndex(2);
  }
  else
  {
    throw std::invalid_argument("Invalid start level in KO_Config!");
  }
  
  // the check box for the "second player survives" choices
  ui.cbSecondSurvives->setChecked(cfg.getSecondSurvives());
  
  // update the spin boxes for the group count / size values
  int nGroupDefs = cfg.getNumGroupDefs();
  if (nGroupDefs >= 1)
  {
    GroupDef g = cfg.getGroupDef(0);
    ui.spGroupCount1->setValue(g.getNumGroups());
    ui.spGroupSize1->setValue(g.getGroupSize());
    oldGroupSize1 = g.getGroupSize();
  } else {
    ui.spGroupCount1->setValue(0);
  }
  if (nGroupDefs >= 2)
  {
    GroupDef g = cfg.getGroupDef(1);
    ui.spGroupCount2->setValue(g.getNumGroups());
    ui.spGroupSize2->setValue(g.getGroupSize());
    oldGroupSize2 = g.getGroupSize();
  } else {
    ui.spGroupCount2->setValue(0);
  }
  if (nGroupDefs >= 3)
  {
    GroupDef g = cfg.getGroupDef(2);
    ui.spGroupCount3->setValue(g.getNumGroups());
    ui.spGroupSize3->setValue(g.getGroupSize());
    oldGroupSize3 = g.getGroupSize();
  } else {
    ui.spGroupCount3->setValue(0);
  }
  
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
  KO_START koLvl = SEMI;
  if (lvl == 1) koLvl = QUARTER;
  else if (lvl == 2) koLvl = L16;
  else if (lvl > 2)
  {
    throw std::invalid_argument("Illegal dropbox index!");
  }
  
  bool secondSurvives = ui.cbSecondSurvives->isChecked();
  
  GroupDefList gdl;
  if (ui.spGroupCount1->value() > 0)
  {
    gdl.append(GroupDef(ui.spGroupSize1->value(), ui.spGroupCount1->value()));
  }
  if (ui.spGroupCount2->value() > 0)
  {
    gdl.append(GroupDef(ui.spGroupSize2->value(), ui.spGroupCount2->value()));
  }
  if (ui.spGroupCount3->value() > 0)
  {
    gdl.append(GroupDef(ui.spGroupSize3->value(), ui.spGroupCount3->value()));
  }
  
  return KO_Config(koLvl, secondSurvives, gdl);
}

//----------------------------------------------------------------------------

void GroupConfigWidget::onStartLevelChanged(int newIndex)
{
  updateLabels();
  emit groupConfigChanged(getConfig());
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
  QSpinBox* sb = ui.spGroupCount1;
  
  if (newVal < 0) sb->setValue(0);
  updateLabels();
  emit groupConfigChanged(getConfig());
}

//----------------------------------------------------------------------------

void GroupConfigWidget::onSpinBoxGroupCount2Changed(int newVal)
{
  QSpinBox* sb = ui.spGroupCount2;
  
  if (newVal < 0) sb->setValue(0);
  updateLabels();
  emit groupConfigChanged(getConfig());
}

//----------------------------------------------------------------------------

void GroupConfigWidget::onSpinBoxGroupCount3Changed(int newVal)
{
  QSpinBox* sb = ui.spGroupCount3;
  
  if (newVal < 0) sb->setValue(0);
  updateLabels();
  emit groupConfigChanged(getConfig());
}

//----------------------------------------------------------------------------
    
void GroupConfigWidget::onSpinBoxGroupSize1Changed(int newVal)
{
  QSpinBox* sb = ui.spGroupSize1;
  
  // avoid the same group size in two or more group defs
  bool hasIncreased = (newVal > oldGroupSize1);  
  while ((newVal == ui.spGroupSize2->value()) || (newVal == ui.spGroupSize3->value()))
  {
    if (hasIncreased) newVal++;
    else newVal--;
  }
  
  // avoid too small or too large values
  if ((newVal < 3) || (newVal > MAX_GROUP_SIZE))
  {
    sb->setValue(oldGroupSize1);
    return;
  }
  
  oldGroupSize1 = newVal;
  sb->setValue(newVal);
  
  updateLabels();
  emit groupConfigChanged(getConfig());
}

//----------------------------------------------------------------------------
    
void GroupConfigWidget::onSpinBoxGroupSize2Changed(int newVal)
{
  QSpinBox* sb = ui.spGroupSize2;
  
  // avoid the same group size in two or more group defs
  bool hasIncreased = (newVal > oldGroupSize2);  
  while ((newVal == ui.spGroupSize1->value()) || (newVal == ui.spGroupSize3->value()))
  {
    if (hasIncreased) newVal++;
    else newVal--;
  }
  
  // avoid too small or too large values
  if ((newVal < 3) || (newVal > MAX_GROUP_SIZE))
  {
    sb->setValue(oldGroupSize2);
    return;
  }
  
  oldGroupSize2 = newVal;
  sb->setValue(newVal);
  
  updateLabels();
  emit groupConfigChanged(getConfig());
}

//----------------------------------------------------------------------------
    
void GroupConfigWidget::onSpinBoxGroupSize3Changed(int newVal)
{
  QSpinBox* sb = ui.spGroupSize3;
  
  // avoid the same group size in two or more group defs
  bool hasIncreased = (newVal > oldGroupSize3);  
  while ((newVal == ui.spGroupSize2->value()) || (newVal == ui.spGroupSize1->value()))
  {
    if (hasIncreased) newVal++;
    else newVal--;
  }
  
  // avoid too small or too large values
  if ((newVal < 3) || (newVal > MAX_GROUP_SIZE))
  {
    sb->setValue(oldGroupSize3);
    return;
  }
  
  oldGroupSize3 = newVal;
  sb->setValue(newVal);
  
  updateLabels();
  emit groupConfigChanged(getConfig());
}

//----------------------------------------------------------------------------

void GroupConfigWidget::setRequiredPlayersCount(int cnt)
{
  if (cnt < 0) return;
  
  reqPlayers = cnt;
  updateLabels();
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
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    

//----------------------------------------------------------------------------
    
