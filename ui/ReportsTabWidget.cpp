/*
 *    This is QTournament, a badminton tournament management program.
 *    Copyright (C) 2014 - 2015  Volker Knollmann
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

#include <QStringList>
#include <QMessageBox>

#include "ReportsTabWidget.h"
#include "ui_ReportsTabWidget.h"

#include "Tournament.h"
#include "reports/ReportFactory.h"
#include "MainFrame.h"

ReportsTabWidget::ReportsTabWidget(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::ReportsTabWidget)
{
  ui->setupUi(this);

  // connect to open / close events for updating our reports tree
  connect(MainFrame::getMainFramePointer(), &MainFrame::tournamentOpened, this, &ReportsTabWidget::onTournamentOpened);

  // create a root item for the reports tree
  // and initialize the pool of available reports
  createRootItem();
  updateRepPool();
}

//----------------------------------------------------------------------------

ReportsTabWidget::~ReportsTabWidget()
{
  delete ui;
}

//----------------------------------------------------------------------------

void ReportsTabWidget::updateRepPool()
{
  auto repFab = Tournament::getReportFactory();
  if (repFab == nullptr) return;

  QStringList existingReports;
  for_each(repPool.cbegin(), repPool.cend(), [&existingReports](const upAbstractReport& rep)
  {
    existingReports.push_back(rep->getName());
  });

  auto newReps = repFab->getMissingReports(existingReports);

  // update the tree and add the new items
  for_each(newReps.cbegin(), newReps.cend(), [&](const upAbstractReport& rep)
  {
    QStringList locators = rep->getReportLocators();
    for (QString locator : locators)
    {
      QTreeWidgetItem* newRepItem = createOrRetrieveTreeItem(locator);
      newRepItem->setText(1, rep->getName());
    }
  });

  // add the new reports to the report tool
  while (!newReps.empty())
  {
    repPool.push_back(std::move(newReps.front()));
    newReps.erase(newReps.begin());
  }

}

//----------------------------------------------------------------------------

void ReportsTabWidget::onTournamentOpened(Tournament* _tnmt)
{
  tnmt = _tnmt;

  connect(tnmt, &Tournament::tournamentClosed, this, &ReportsTabWidget::onTournamentClosed);

  createRootItem();
  updateRepPool();
}

//----------------------------------------------------------------------------

void ReportsTabWidget::onTournamentClosed()
{
  disconnect(tnmt, &Tournament::tournamentClosed, this, &ReportsTabWidget::onTournamentClosed);
  repPool.clear();
}

//----------------------------------------------------------------------------

QTreeWidgetItem* ReportsTabWidget::createOrRetrieveTreeItem(const QString& locator)
{
  if (locator.isEmpty()) return nullptr;
  if (locator == "") return nullptr;

  QStringList locatorParts = locator.split("::");

  QTreeWidgetItem* curItem = treeRoot;
  for (QString locPart : locatorParts)
  {
    QTreeWidgetItem* nextLevel = findTreeItemChildByName(curItem, locPart);
    if (nextLevel == nullptr)
    {
      nextLevel = new QTreeWidgetItem();
      nextLevel->setText(0, locPart);
      curItem->addChild(nextLevel);
    }

    curItem = nextLevel;
  }

  return curItem;
}

//----------------------------------------------------------------------------

QTreeWidgetItem* ReportsTabWidget::findTreeItemChildByName(QTreeWidgetItem* _parent, const QString& childName) const
{
  QTreeWidgetItem* parentItem = (_parent != nullptr) ? _parent : treeRoot;

  int nChilds = parentItem->childCount();
  for (int i=0; i < nChilds; ++i)
  {
    QTreeWidgetItem* child = parentItem->child(i);
    if (child->text(0) == childName) return child;
  }

  return nullptr;
}

//----------------------------------------------------------------------------

void ReportsTabWidget::createRootItem()
{
  // delete old contents including all childs
  if (treeRoot != nullptr) ui->repTree->clear();

  // initialize a fresh tree
  treeRoot = new QTreeWidgetItem(ui->repTree);
  treeRoot->setText(0, tr("Reports"));

  // delete all old reports
  repPool.clear();
}

//----------------------------------------------------------------------------

void ReportsTabWidget::onTreeSelectionChanged()
{
  QTreeWidgetItem* curItem = ui->repTree->currentItem();
  if (curItem == nullptr) return;   // no selection

  // is a leaf selected? leaf = report!
  if (curItem->childCount() > 0) return;   // no leaf, nothing to do

  // okay, this is a leaf, so we have a new report selection
  QString repName = curItem->text(1);
  showReport(repName);
}

//----------------------------------------------------------------------------

void ReportsTabWidget::showReport(const QString& repName)
{
  for_each(repPool.cbegin(), repPool.cend(), [&](const upAbstractReport& rep)
  {
    if (rep->getName() == repName)
    {
      upSimpleReport newReport = rep->regenerateReport();
      if (newReport == nullptr) return;

      // store the newly created report in our own unique_ptr.
      // The old report will be automatically deleted
      //
      // if we don't store the new report in curReport,
      // it would be automatically deleted when we leave
      // this code block
      curReport = std::move(newReport);
      SimpleReportLib::SimpleReportGenerator* rawPointer = curReport.get();
      ui->repViewer->setReport(rawPointer);
      return;
    }
  });
}

//----------------------------------------------------------------------------

void ReportsTabWidget::onReloadRequested()
{
  // refresh the list of available reports
  updateRepPool();

  // if we currently have a report displayed,
  // regenerate this report because the underlying data
  // might have changed in the meantime
  if (curReport != nullptr)
  {
    onTreeSelectionChanged();
  }
}

//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------

